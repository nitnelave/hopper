#pragma once

#include "util/variant.h"

struct NoneType {};

static constexpr NoneType none = NoneType();

template <typename Value>
class Option {
  static_assert(!std::is_reference<Value>::value,
                "Option doesn't support references");

  Variant<NoneType, Value> variant_;

 public:
  Option() = default;

  Option(const NoneType& /*unused*/) : Option() {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a copy
  // constructor.
  Option(Option&& rhs) : variant_(std::move(rhs.variant_)){};  // NOLINT
  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option(Option<T>&& rhs) : variant_(std::move(rhs.variant_)) {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a move
  // constructor.
  Option(const Option& rhs) : variant_(rhs.variant_){};  // NOLINT
  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option(const Option<T>& rhs) : variant_(rhs.variant_) {}  // NOLINT

  Option(Value v) : variant_(std::move(v)) {}  // NOLINT

  bool is_ok() const noexcept { return variant_.template is<Value>(); }

  const Value& value_or_die() const { return variant_.template get<Value>(); }
  Value& value_or_die() { return variant_.template get<Value>(); }
  Value consume_value_or_die() { return variant_.template consume<Value>(); }

  const Value& value_or(const Value& default_value) {
    if (is_ok()) return variant_.template get_unchecked<Value>();
    return default_value;
  }

  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option& operator=(T v) {
    variant_ = Value(std::move(v));
    return *this;
  }

  // Need the explicit version.
  Option& operator=(Option&& rhs) { return operator=<Value>(std::move(rhs)); }

  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option& operator=(Option<T>&& rhs) {
    if (this != &rhs) {
      variant_ = std::move(rhs.variant_);
    }
    return *this;
  }

  // Need the explicit version.
  Option& operator=(const Option& rhs) { return operator=<Value>(rhs); }

  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option& operator=(const Option<T>& rhs) {
    if (this != &rhs) {
      variant_ = rhs.variant_;
    }
    return *this;
  }

  template <typename T>
  friend class Option;
};
