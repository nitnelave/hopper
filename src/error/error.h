#pragma once

#include <string>

// Error utilities.

// Base error class, with a message.
// All error classes should derive from this one.
class Error {
 public:
  explicit Error(const std::string& message) : message_(message) {}

  virtual std::string toString() const { return message(); }

 protected:
  std::string message() const { return message_; }
 private:
  const std::string message_;
};


// Either an error or a T.
// This class is immutable (but the T can be mutable).
template <typename T, typename Err = Error>
class ErrorOr {
 public:
  // Not explicit on purpose.
  ErrorOr(const Err& error) : is_ok_(false) {
    // Placement new.
    new (&union_.error) Err(error);
  }

  // Not explicit on purpose.
  ErrorOr(Err&& error) : is_ok_(false) {
    // Placement new.
    new (&union_.error) Err(std::move(error));
  }

  // Not explicit on purpose.
  ErrorOr(const T& value) : is_ok_(true) {
    // Placement new.
    new (&union_.value) T(value);
  }

  // Not explicit on purpose.
  ErrorOr(T&& value) : is_ok_(true) {
    // Placement new.
    new (&union_.value) T(std::move(value));
  }

  ErrorOr(const ErrorOr& other) : is_ok_(other.is_ok_) {
    if (is_ok_)
      union_.value = other.union_.value;
    else
      union_.error = other.union_.error;
  }

  ErrorOr(ErrorOr&& other) : is_ok_(other.is_ok_) {
    if (is_ok_)
      new (&union_.value) T(std::move(other.union_.value));
    else
      new (&union_.error) Err(std::move(other.union_.error));
  }

  ErrorOr& operator=(const ErrorOr& other) {
    ~ErrorOr();
    is_ok_ = other.is_ok_;
    if (is_ok_)
      union_.value = other.union_.value;
    else
      union_.error = other.union_.error;
  }

  ErrorOr& operator=(ErrorOr&& other) {
    std::swap(is_ok_, other.is_ok_);
    if (is_ok_)
      std::swap(union_.value, other.union_.value);
    else
      std::swap(union_.error, other.union_.error);
  }

  // Check if it's an error or a value.
  bool ok() const { return is_ok_; }

  // Return the value if it is one, fail otherwise.
  T& value_or_die() {
    if (!ok())
      throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;
  }

  // Return the value if it is one, fail otherwise.
  const T& value_or_die() const {
    if (!ok())
      throw std::domain_error("ErrorOr was error, asked for value");
    return union_.value;
  }

  // Return the error if it is one, fail otherwise.
  const Err& error_or_die() const {
    if (ok())
      throw std::domain_error("ErrorOr was value, asked for error");
    return union_.error;
  }

  // Return the error if it is one, otherwise return "Ok".
  std::string toString() const {
    if (ok())
      return "Ok";
    else
      return error_or_die().toString();
  }

  ~ErrorOr() {
    if (!ok())
      union_.error.~Err();
    else
      union_.value.~T();
  }

 private:
  bool is_ok_;
  union Union {
    Err error;
    T value;

    Union() {}
    ~Union() {}
  } union_;
};

namespace internals {
  struct Dummy{};
}

// Either an error or a positive result.
// This class is immutable.
template <typename Err>
class MaybeError : public ErrorOr<internals::Dummy, Err> {
 public:
  using Base = ErrorOr<internals::Dummy, Err>;
  // Constructor for no error.
  MaybeError() : Base(internals::Dummy()) {}
  using Base::Base;
};

#define RETURN_IF_ERROR(CALL)      \
  do {                             \
    const auto& res = CALL;        \
    if (!res.ok())                 \
      return {res.error_or_die()}; \
  } while (0)

#define RETURN_OR_ASSIGN(DECL, CALL) \
  const auto& res = CALL;            \
  if (!res.ok())                     \
    return {res.error_or_die()};     \
  DECL = res.value_or_die();
