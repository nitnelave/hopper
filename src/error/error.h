#pragma once

#include <memory>
#include <stdexcept>
#include <string>

// Error utilities.

/// Base error interface.
/// All error classes must derive from this one.
class Error {
 public:
  /// Conversion to a string for display.
  virtual std::string to_string() const = 0;
  virtual ~Error() = default;
};

/// Basic error, with an error message.
class GenericError : public Error {
 public:
  /// Build a new generic error, with an error message.
  explicit GenericError(std::string message)
      : Error(), message_(std::move(message)) {}

  /// Return the error message it was constructed with.
  std::string to_string() const override { return message(); }

  ~GenericError() override = default;

 protected:
  /// Access to the field containing the error message passed to the
  /// constructor.
  std::string message() const { return message_; }

 private:
  const std::string message_;
};

/// Represents either an error or a T.
/// This class is immutable (but the T can be mutable).
template <typename T, typename Err = GenericError>
class ErrorOr {
  static_assert(std::is_base_of<Error, Err>::value,
                "Error type must be a subclass of Error");
  template <typename E>
  using enable_if_error = typename std::enable_if<std::is_base_of<
      typename std::decay<Err>::type, typename std::decay<E>::type>::value>;

 public:
  // Constructors from value or errors.

  // Not explicit on purpose.
  template <typename E>
  ErrorOr(E error,  // NOLINT: explicit
          typename enable_if_error<E>::type* /*unused*/ = nullptr)
      : union_(std::move(error)), is_ok_(false) {}

  // Not explicit on purpose.
  ErrorOr(T value)  // NOLINT: explicit
      : union_(std::move(value)),
        is_ok_(true) {}

  // Move constructor.
  template <typename E>
  ErrorOr(ErrorOr<T, E>&& other)  // NOLINT: explicit
      : is_ok_(other.is_ok_) {
    if (is_ok_)
      new (&union_.value)                    // NOLINT: union access
          T(std::move(other.union_.value));  // NOLINT: union access
    else
      new (&union_.error)  // NOLINT: union access
          std::unique_ptr<Err>(
              std::move(other.union_.error));  // NOLINT: union access
  }

  // Move assignment.
  template <typename E>
  ErrorOr& operator=(ErrorOr<T, E>&& other) {
    this->~ErrorOr();
    is_ok_ = other.is_ok_;
    if (is_ok_)
      new (&union_.value)                    // NOLINT: union access
          T(std::move(other.union_.value));  // NOLINT: union access
    else
      new (&union_.error)  // NOLINT: union access
          std::unique_ptr<Err>(
              std::move(other.union_.error));  // NOLINT: union access
    return *this;
  }

  // Copy constructor.
  ErrorOr(const ErrorOr& other) = delete;
  // Copy assignment.
  ErrorOr& operator=(const ErrorOr& other) = delete;

  /// Check whether it is an error or a value.
  bool is_ok() const { return is_ok_; }

  /// Return the value if it is one, fail otherwise.
  T& value_or_die() {
    if (!is_ok_) throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;  // NOLINT: union access
  }

  /// Return the value if it is one, fail otherwise.
  const T& value_or_die() const {
    if (!is_ok_) throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;  // NOLINT: union access
  }

  /// Return the error if it is one, fail otherwise.
  const Err& error_or_die() const {
    if (is_ok_) throw std::domain_error("ErrorOr was value, asked for error");
    return *union_.error;  // NOLINT: union access
  }

  /// Return the error if it is one, otherwise return "Ok".
  std::string to_string() const {
    if (is_ok_) return "Ok";
    return error_or_die().to_string();
  }

  ~ErrorOr() {
    // Make sure to delete the right value.
    if (is_ok_)
      union_.value.~T();  // NOLINT: union access
    else
      union_.error.~unique_ptr<Err>();  // NOLINT: union access
  }

 private:
  // Contain either an error or a value, with the appropriate constructors.
  union Union {
    std::unique_ptr<Err> error;
    T value;

    template <typename E>
    explicit Union(E e) : error(new E(std::move(e))) {}
    explicit Union(T v) : value(std::move(v)) {}
    // Default constructor leaves the memory uninitialized, make to to
    // initialize it after.
    Union() {}  // NOLINT: modernize suggests =default
    // Destructor does not destroy (not enough information).
    ~Union() {}  // NOLINT: modernize suggests =default
  } union_;
  // true: value, false: error.
  bool is_ok_;

  // Friend other implementations of that class, for the move
  // constructor/assignment.
  template <typename Value, typename E>
  friend class ErrorOr;
};

namespace internals {
// Empty struct to act as placeholder of a value.
struct Dummy {};
}  // namespace internals

/// Either an error or a positive result.
/// This class is immutable.
template <typename Err = GenericError>
class MaybeError : private ErrorOr<internals::Dummy, Err> {
 public:
  using Base = ErrorOr<internals::Dummy, Err>;
  // Constructor for no error.
  MaybeError() : Base(internals::Dummy()) {}  // NOLINT: explicit

  // Inherit the constructors.
  template <typename T>
  MaybeError(T&& value) : Base(std::forward<T>(value)) {}  // NOLINT: explicit
  // Inherit methods.
  using Base::error_or_die;
  using Base::is_ok;
};

// Macro to propagate the error from the method called, if it failed.
#define RETURN_IF_ERROR(CALL)                      \
  do {                                             \
    const auto& res = (CALL);                      \
    if (!res.is_ok()) return {res.error_or_die()}; \
  } while (0)

// These macros are needed because we can't just use res##__LINE__, because
// then __LINE__ is not a token that get preprocessed.
#define __ERROR_MACRO_COMBINE(X, Y) X##Y
#define __ERROR_MACRO_VAR(X) __ERROR_MACRO_COMBINE(__ERROR_RESULT, X)

// Macro to either propagate the error from the method called, or assign it to
// a local variable if it succeeded.
#define RETURN_OR_ASSIGN(DECL, CALL)                     \
  const auto& __ERROR_MACRO_VAR(__LINE__) = CALL;        \
  if (!__ERROR_MACRO_VAR(__LINE__).is_ok())              \
    return {__ERROR_MACRO_VAR(__LINE__).error_or_die()}; \
  DECL = __ERROR_MACRO_VAR(__LINE__).value_or_die();  // NOLINT (parenthesis)
