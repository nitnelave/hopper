#pragma once

#include "util/variant.h"

template <typename Value>
class Option {
  static_assert(!std::is_reference<Value>::value,
                "Option doesn't support references");

  struct NoneType {};

  Variant<NoneType, Value> variant_;

 public:
  Option() = default;

  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option(Option<T>&& rhs) : variant_(std::move(rhs.variant_)) {}  // NOLINT

  Option(Value v) : variant_(std::move(v)) {}  // NOLINT

  bool is_ok() const noexcept { return variant_.template is<Value>(); }

  const Value& value_or_die() const { return variant_.template get<Value>(); }
  Value& value_or_die() { return variant_.template get<Value>(); }

  Option& operator=(const Value& v) {
    variant_ = v;
    return *this;
  }

  template <typename T, typename = typename std::is_convertible<T, Value>>
  Option& operator=(Option<T>&& rhs) {
    if (this != &rhs) {
      variant_ = rhs.variant_;
    }
    return *this;
  }

  template <typename T>
  friend class Option;
};
