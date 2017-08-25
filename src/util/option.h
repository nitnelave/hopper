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

  Option& operator=(const NoneType& /*unused*/) {
    variant_ = none;
    return *this;
  }

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

template <typename... Values>
class Option<Variant<Values...>> {
 public:
  using RawVariant = Variant<Values...>;
  using FullVariant = Variant<NoneType, Values...>;

 private:
  FullVariant variant_;

 public:
  Option() = default;

  Option(const NoneType& /*unused*/) : Option() {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a copy
  // constructor.
  Option(Option&& rhs) : variant_(std::move(rhs.variant_)){};  // NOLINT
  template <typename T,
            typename = typename std::is_constructible<RawVariant, T>>
  Option(Option<T>&& rhs) : variant_(std::move(rhs.variant_)) {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a move
  // constructor.
  Option(const Option& rhs) : variant_(rhs.variant_){};  // NOLINT
  template <typename T, typename = typename std::is_convertible<RawVariant, T>>
  Option(const Option<T>& rhs) : variant_(rhs.variant_) {}  // NOLINT

  template <typename T, typename = typename std::is_convertible<RawVariant, T>>
  Option(T v) : variant_(std::move(v)) {}  // NOLINT

  bool is_ok() const noexcept { return !variant_.template is<NoneType>(); }

  const FullVariant& value_or_die() const { return variant_; }
  FullVariant& value_or_die() { return variant_; }
  FullVariant consume_value_or_die() { return std::move(variant_); }

  const FullVariant& value_or(const FullVariant& default_value) {
    if (is_ok()) return variant_;
    return default_value;
  }

  template <typename T,
            typename = typename std::is_constructible<RawVariant, T>>
  Option& operator=(T v) {
    variant_ = RawVariant(std::move(v));
    return *this;
  }

  Option& operator=(Option&& rhs) {
    if (this != &rhs) {
      variant_ = std::move(rhs.variant_);
    }
    return *this;
  }

  template <typename T,
            typename = typename std::is_constructible<RawVariant, T>>
  Option& operator=(Option<T>&& rhs) {
    variant_ = std::move(rhs.variant_);
    return *this;
  }

  Option& operator=(const Option& rhs) {
    if (this != &rhs) {
      variant_ = rhs.variant_;
    }
    return *this;
  }

  Option& operator=(const NoneType& /*unused*/) {
    variant_ = none;
    return *this;
  }

  template <typename T,
            typename = typename std::is_constructible<RawVariant, T>>
  Option& operator=(const Option<T>& rhs) {
    variant_ = rhs.variant_;
    return *this;
  }

  template <typename T>
  friend class Option;
};

template <typename RawValue>
class Option<RawValue*> {
 public:
  using Value = RawValue*;

 private:
  Value variant_ = nullptr;

 public:
  Option() = default;

  Option(const NoneType& /*unused*/) : Option() {}        // NOLINT
  Option(const std::nullptr_t& /*unused*/) : Option() {}  // NOLINT

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

  bool is_ok() const noexcept { return variant_ != nullptr; }

  const Value& value_or_die() const { return variant_; }
  Value& value_or_die() { return variant_; }
  Value consume_value_or_die() { return variant_; }

  const Value& value_or(const Value& default_value) {
    if (is_ok()) return variant_;
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

  Option& operator=(const NoneType& /*unused*/) {
    variant_ = nullptr;
    return *this;
  }

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
