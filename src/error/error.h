#pragma once

#include <string>
#include <stdexcept>

// Error utilities.

/// Base error interface.
/// All error classes must derive from this one.
class Error {
 public:
  /// Conversion to a string for display.
  virtual std::string toString() const = 0;
  virtual ~Error() {}
};

/// Basic error, with an error message.
class GenericError : public Error {
 public:
  /// Build a new generic error, with an error message.
  explicit GenericError(const std::string& message) : Error(), message_(message) {}

  /// Return the error message it was constructed with.
  std::string toString() const override { return message(); }

  ~GenericError() override {}
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
 public:
  // Constructors from value or errors.

  // Not explicit on purpose.
  ErrorOr(const Err& error) : union_(error), is_ok_(false) {}

  // Not explicit on purpose.
  ErrorOr(Err&& error) : union_(std::move(error)), is_ok_(false) {}

  // Not explicit on purpose.
  ErrorOr(const T& value) : union_(value), is_ok_(true) {}

  // Not explicit on purpose.
  ErrorOr(T&& value) : union_(std::move(value)), is_ok_(true) {}

  // Copy constructor.
  ErrorOr(const ErrorOr& other)
      : is_ok_(other.is_ok_)
  {
    if (is_ok_)
      new (&union_.value) T(other.union_.value);
    else
      new (&union_.error) Err(other.union_.error);
  }

  // Move constructor.
  ErrorOr(ErrorOr&& other)
      : is_ok_(other.is_ok_)
  {
    if (is_ok_)
      new (&union_.value) T(std::move(other.union_.value));
    else
      new (&union_.error) Err(std::move(other.union_.error));
  }

  // Copy assignment.
  ErrorOr& operator=(const ErrorOr& other) {
    this->~ErrorOr();
    is_ok_ = other.is_ok_;
    if (is_ok_)
      new (&union_.value) T(other.union_.value);
    else
      new (&union_.error) Err(other.union_.error);
    return *this;
  }

  // Move assignment.
  ErrorOr& operator=(ErrorOr&& other) {
    this->~ErrorOr();
    is_ok_ = other.is_ok_;
    if (is_ok_)
      new (&union_.value) T(std::move(other.union_.value));
    else
      new (&union_.error) Err(std::move(other.union_.error));
    return *this;
  }

  /// Check whether it is an error or a value.
  bool is_ok() const { return is_ok_; }

  /// Return the value if it is one, fail otherwise.
  T& value_or_die() {
    if (!is_ok_)
      throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;
  }

  /// Return the value if it is one, fail otherwise.
  const T& value_or_die() const {
    if (!is_ok_)
      throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;
  }

  /// Return the error if it is one, fail otherwise.
  const Err& error_or_die() const {
    if (is_ok_)
      throw std::domain_error("ErrorOr was value, asked for error");
    return union_.error;
  }

  /// Return the error if it is one, otherwise return "Ok".
  std::string toString() const {
    if (is_ok_)
      return "Ok";
    else
      return error_or_die().toString();
  }

  ~ErrorOr() {
    // Make sure to delete the right value.
    if (is_ok_)
      union_.value.~T();
    else
      union_.error.~Err();
  }

 private:
  // Contain either an error or a value, with the appropriate constructors.
  union Union {
    Err error;
    T value;

    Union(Err&& e) : error(std::move(e)) {}
    Union(const Err& e) : error(e) {}
    Union(T&& v) : value(std::move(v)) {}
    Union(const T& v) : value(v) {}
    // Default constructor leaves the memory uninitialized, make to to
    // initialize it after.
    Union() {}
    // Destructor does not destroy (not enough information).
    ~Union() {}
  } union_;
  // true: value, false: error.
  bool is_ok_;
};

namespace internals {
  // Empty struct to act as placeholder of a value.
  struct Dummy{};
}

/// Either an error or a positive result.
/// This class is immutable.
template <typename Err = GenericError>
class MaybeError : private ErrorOr<internals::Dummy, Err> {
 public:
  using Base = ErrorOr<internals::Dummy, Err>;
  // Constructor for no error.
  MaybeError() : Base(internals::Dummy()) {}
  // Inherit constructors.
  using Base::Base;
  // Inherit methods.
  using Base::error_or_die;
  using Base::is_ok;
};


// Macro to propagate the error from the method called, if it failed.
#define RETURN_IF_ERROR(CALL)      \
  do {                             \
    const auto& res = (CALL);      \
    if (!res.is_ok())              \
      return {res.error_or_die()}; \
  } while (0)

// These macros are needed because we can't just use res##__LINE__, because
// then __LINE__ is not a token that get preprocessed.
#define __ERROR_MACRO_COMBINE(X, Y) X##Y
#define __ERROR_MACRO_VAR(X) __ERROR_MACRO_COMBINE(__ERROR_RESULT, X)

// Macro to either propagate the error from the method called, or assign it to
// a local variable if it succeeded.
#define RETURN_OR_ASSIGN(DECL, CALL)                         \
  const auto& __ERROR_MACRO_VAR(__LINE__) = CALL;            \
  if (!__ERROR_MACRO_VAR(__LINE__).is_ok())                  \
    return {__ERROR_MACRO_VAR(__LINE__).error_or_die()};     \
  DECL = __ERROR_MACRO_VAR(__LINE__).value_or_die();
