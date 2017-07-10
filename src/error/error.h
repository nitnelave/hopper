#pragma once

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

#include "util/option.h"
#include "util/variant.h"

// Error utilities.

/// Base error interface.
/// All error classes must derive from this one.
class Error {
 public:
  /// Conversion to a string for display.
  virtual std::string to_string() const = 0;
  // LCOV_EXCL_START: never called
  virtual ~Error() = default;
  // LCOV_EXCL_STOP
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

inline bool operator==(const GenericError& lhs, const GenericError& rhs) {
  return lhs.to_string() == rhs.to_string();
}

inline bool operator!=(const GenericError& lhs, const GenericError& rhs) {
  return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const GenericError& error) {
  return os << error.to_string();
}

namespace internals {
template <typename Base, typename Derived>
using enable_if_base_of = typename std::enable_if<
    std::is_base_of<Base, typename std::decay<Derived>::type>::value>;
}  // namespace internals

/// Represents either an error or a Value.
/// This class is immutable (but the Value can be mutable).
template <typename Value, typename Err = GenericError>
class ErrorOr {
  static_assert(std::is_base_of<Error, Err>::value,
                "Error type must be a subclass of Error");
  template <typename E>
  using enable_if_error = internals::enable_if_base_of<Err, E>;

  using ErrPtr = std::unique_ptr<Err>;

 public:
  using ErrorType = Err;
  using ValueType = Value;

  // Constructors from value or errors.

  /// Construct an error from a subtype of Err.
  template <typename E>
  ErrorOr(E error,  // NOLINT: explicit
          typename enable_if_error<E>::type* /*unused*/ = nullptr)
      : variant_(std::make_unique<Err>(std::move(error))) {}

  /// Construct a value directly.
  template <typename T, typename = typename std::enable_if<
                            std::is_constructible<Value, T>::value>::type>
  ErrorOr(T value)  // NOLINT: explicit
      : variant_(Value(std::move(value))) {}

  // Move constructor.
  template <typename T, typename E,
            typename = typename std::enable_if<
                std::is_constructible<Value, T>::value>::type,
            typename = typename enable_if_error<E>::type>
  ErrorOr(ErrorOr<T, E>&& other)  // NOLINT: explicit
      : variant_(std::move(other.variant_)) {}

  // Move assignment.
  template <
      typename T, typename E, typename = typename std::enable_if<
                                  std::is_constructible<Value, T>::value>::type,
      typename =
          typename std::enable_if<std::is_constructible<Err, E>::value>::type>
  ErrorOr& operator=(ErrorOr<T, E>&& other) {
    variant_ = std::move(other.variant_);
    return *this;
  }

  // Copy constructor.
  ErrorOr(const ErrorOr& other) = delete;
  // Copy assignment.
  ErrorOr& operator=(const ErrorOr& other) = delete;

  /// Check whether it is an error or a value.
  bool is_ok() const { return variant_.template is<Value>(); }

  /// Return the value if it is one, fail otherwise.
  Value& value_or_die() { return variant_.template get<Value>(); }

  /// Return the value if it is one, fail otherwise.
  const Value& value_or_die() const { return variant_.template get<Value>(); }

  /// Gives ownership of the value if it is one, fail otherwise.
  Value consume_value_or_die() { return variant_.template consume<Value>(); }

  /// Return the error if it is one, fail otherwise.
  const Err& error_or_die() const { return *variant_.template get<ErrPtr>(); }

  /// Return the error if it is one, otherwise return "Ok".
  std::string to_string() const {
    if (is_ok()) return "Ok";
    return error_or_die().to_string();
  }

 private:
  Variant<Value, ErrPtr> variant_;

  // Friend other implementations of that class, for the move
  // constructor/assignment.
  template <typename T, typename E>
  friend class ErrorOr;
};

/// Either an error or a positive result.
/// This class is immutable.
template <typename Err = GenericError>
class MaybeError {
 public:
  static_assert(std::is_base_of<Error, Err>::value,
                "Error type must be a subclass of Error");

  using ErrorType = Err;

  template <typename E>
  using enable_if_error = internals::enable_if_base_of<Err, E>;

  // Constructor for no error.
  MaybeError() : error_or_() {}

  // Inherit the constructors.
  template <typename E>
  MaybeError(  // NOLINT: explicit
      E&& value,
      typename std::enable_if<
          std::is_constructible<Err, E>::value>::type* /*unused*/ = nullptr)
      : error_or_(std::make_unique<Err>(std::forward<E>(value))) {}

  template <typename E>
  MaybeError(  // NOLINT: explicit
      MaybeError<E>&& other,
      typename std::enable_if<
          std::is_constructible<Err, E>::value>::type* /*unused*/ = nullptr)
      : error_or_(std::move(other.error_or_)) {}

  template <typename E, typename = typename std::enable_if<
                            std::is_constructible<Err, E>::value>::type>
  MaybeError& operator=(MaybeError<E>&& other) {
    error_or_ = std::move(other.error_or_);
    return *this;
  }

  MaybeError(const MaybeError&) = delete;
  MaybeError& operator=(const MaybeError&) = delete;

  // Inherit methods.
  const Err& error_or_die() const { return *error_or_.value_or_die(); }
  bool is_ok() const { return !error_or_.is_ok(); }

  std::string to_string() const {
    if (is_ok()) return "Ok";
    return error_or_die().to_string();
  }

 private:
  Option<std::unique_ptr<Err>> error_or_;

  template <typename E>
  friend class MaybeError;
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

// Macro to either propagate the error from the method called, or move it to
// a local variable if it succeeded.
#define RETURN_OR_MOVE(DECL, CALL)                       \
  auto __ERROR_MACRO_VAR(__LINE__) = CALL;               \
  if (!__ERROR_MACRO_VAR(__LINE__).is_ok())              \
    return {__ERROR_MACRO_VAR(__LINE__).error_or_die()}; \
  DECL = __ERROR_MACRO_VAR(__LINE__).consume_value_or_die();  // NOLINT
