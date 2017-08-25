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

  // End constructors.

  bool is_ok() const noexcept { return variant_.template is<Value>(); }

  const Value& value_or_die() const { return variant_.template get<Value>(); }
  Value& value_or_die() { return variant_.template get<Value>(); }
  Value consume_value_or_die() { return variant_.template consume<Value>(); }

  const Value& value_or(const Value& default_value) {
    if (is_ok()) return variant_.template get_unchecked<Value>();
    return default_value;
  }

  // Assignment operators.

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

/// Overload Option in case of Option<Variant<...>>.
template <typename... Values>
class Option<Variant<Values...>> {
 public:
  using Value = Variant<Values...>;
  // Variant's type_index counts forward, so 0 is the first element. To have
  // compatibility between Value and FullVariant, we need to put the NoneType
  // at the end.
  using FullVariant = Variant<Values..., NoneType>;

 private:
  FullVariant variant_;

 public:
  Option() : variant_(none) {}

  Option(const NoneType& /*unused*/) : Option() {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a copy
  // constructor.
  Option(Option&& rhs) : variant_(std::move(rhs.variant_)){};  // NOLINT
  template <typename T, typename = typename std::is_constructible<Value, T>>
  Option(Option<T>&& rhs) : variant_(std::move(rhs.variant_)) {}  // NOLINT

  // We need the explicit version otherwise the compiler generates a move
  // constructor.
  Option(const Option& rhs) : variant_(rhs.variant_){};  // NOLINT
  template <typename T, typename = typename std::is_constructible<Value, T>>
  Option(const Option<T>& rhs) : variant_(rhs.variant_) {}  // NOLINT

  template <typename T, typename = typename std::is_constructible<Value, T>>
  Option(T v) : variant_(std::move(v)) {}  // NOLINT

  Option(Value v) : variant_(std::move(v)) {}  // NOLINT

  // End constructors.

  bool is_ok() const noexcept { return !variant_.template is<NoneType>(); }

  const Value& value_or_die() const {
    return *reinterpret_cast<const Value*>(&variant_);  // NOLINT
  }
  Value& value_or_die() {
    return *reinterpret_cast<Value*>(&variant_);  // NOLINT
  }
  Value consume_value_or_die() { return std::move(value_or_die()); }

  const Value& value_or(const Value& default_value) {
    if (is_ok()) return value_or_die();
    return default_value;
  }

  // Copy Variant's interface and forward the calls.

  template <typename T>
  const T& get() const {
    return variant_.template get<T>();
  }

  template <typename T>
  T& get() {
    return variant_.template get<T>();
  }

  template <typename T>
  const T& get_unchecked() const {
    return variant_.template get_unchecked<T>();
  }

  template <typename T>
  T& get_unchecked() {
    return variant_.template get_unchecked<T>();
  }

  template <typename T>
  T consume() {
    return variant_.template consume<T>();
  }

  template <typename T>
  T consume_unchecked() {
    return variant_.template consume_unchecked<T>();
  }

  template <typename T>
  bool is() const {
    return variant_.template is<T>();
  }

  int which() const noexcept { return variant_.which(); }

  template <typename T, typename... Args>
  void set(Args&&... args) {
    variant_.template set(std::forward<Args>(args)...);
  }

  // Assignement operators.

  template <typename T, typename = typename std::is_constructible<Value, T>>
  Option& operator=(T v) {
    variant_ = FullVariant(std::move(v));
    return *this;
  }

  Option& operator=(Variant<Values...> v) {
    variant_ =
        FullVariant(std::move(*reinterpret_cast<FullVariant*>(&v)));  // NOLINT
    return *this;
  }

  Option& operator=(Option&& rhs) {
    if (this != &rhs) {
      variant_ = std::move(rhs.variant_);
    }
    return *this;
  }

  template <typename T, typename = typename std::is_constructible<Value, T>>
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

  template <typename T, typename = typename std::is_constructible<Value, T>>
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

  // End constructors.

  bool is_ok() const noexcept { return variant_ != nullptr; }

  const Value& value_or_die() const { return variant_; }
  Value& value_or_die() { return variant_; }
  Value consume_value_or_die() { return variant_; }

  const Value& value_or(const Value& default_value) {
    if (is_ok()) return variant_;
    return default_value;
  }

  // Assignment operators.

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
