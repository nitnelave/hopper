#pragma once

#include <cassert>
#include <cstddef>  // size_t
#include <functional>
#include <limits>
#include <new>        // operator new
#include <stdexcept>  // logic_error
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

class BadVariantAccess : public std::logic_error {
 public:
  explicit BadVariantAccess(const std::string& what_arg)
      : logic_error(what_arg) {}

  explicit BadVariantAccess(const char* what_arg) : logic_error(what_arg) {}
  // LCOV_EXCL_START: never called
  ~BadVariantAccess() override = default;
  // LCOV_EXCL_STOP
};

namespace internals {
using TypeIndex = unsigned int;

static constexpr TypeIndex invalid_value = TypeIndex(-1);

/// Used to find a type.
template <typename T, typename... Types>
struct DirectType;

template <typename T, typename First, typename... Types>
struct DirectType<T, First, Types...> {
  static constexpr TypeIndex index = std::is_same<T, First>::value
                                         ? sizeof...(Types)
                                         : DirectType<T, Types...>::index;
};

template <typename T>
struct DirectType<T> {
  static constexpr TypeIndex index = invalid_value;
};

/// Compile time && and ||
template <typename...>
struct Conjunction : std::true_type {};

template <typename B1>
struct Conjunction<B1> : B1 {};

template <typename B1, typename B2>
struct Conjunction<B1, B2> : std::conditional<B1::value, B2, B1>::type {};

template <typename B1, typename... Bs>
struct Conjunction<B1, Bs...>
    : std::conditional<B1::value, Conjunction<Bs...>, B1>::type {};

template <typename...>
struct Disjunction : std::false_type {};

template <typename B1>
struct Disjunction<B1> : B1 {};

template <typename B1, typename B2>
struct Disjunction<B1, B2> : std::conditional<B1::value, B1, B2>::type {};

template <typename B1, typename... Bs>
struct Disjunction<B1, Bs...>
    : std::conditional<B1::value, B1, Disjunction<Bs...>>::type {};

template <typename T, typename... Types>
struct ConvertibleType;

template <typename T, typename First, typename... Types>
struct ConvertibleType<T, First, Types...> {
  static constexpr TypeIndex index =
      std::is_convertible<T, First>::value
          ? Disjunction<std::is_convertible<T, Types>...>::value
                ? invalid_value
                : sizeof...(Types)
          : ConvertibleType<T, Types...>::index;
};

template <typename T>
struct ConvertibleType<T> {
  static constexpr TypeIndex index = invalid_value;
};

template <typename T, typename... Types>
struct ValueTraits {
  using ValueType =
      typename std::remove_const<typename std::remove_reference<T>::type>::type;
  static constexpr TypeIndex direct_index =
      DirectType<ValueType, Types...>::index;
  static constexpr bool is_direct = direct_index != invalid_value;
  static constexpr TypeIndex index =
      is_direct ? direct_index : ConvertibleType<ValueType, Types...>::index;
  static constexpr bool is_valid = index != invalid_value;
  static constexpr TypeIndex tindex = is_valid ? sizeof...(Types)-index : 0;
  using TargetType =
      typename std::tuple_element<tindex, std::tuple<void, Types...>>::type;
};

template <TypeIndex Arg1, TypeIndex... Others>
struct StaticMax;

template <TypeIndex Arg>
struct StaticMax<Arg> {
  static const TypeIndex value = Arg;
};

template <TypeIndex Arg1, TypeIndex Arg2, TypeIndex... Others>
struct StaticMax<Arg1, Arg2, Others...> {
  static const TypeIndex value = Arg1 >= Arg2
                                     ? StaticMax<Arg1, Others...>::value
                                     : StaticMax<Arg2, Others...>::value;
};

template <typename... Types>
struct VariantHelper;

template <typename T, typename... Types>
struct VariantHelper<T, Types...> {
  static void destroy(const TypeIndex type_index, void* data) {
    if (type_index == sizeof...(Types)) {
      reinterpret_cast<T*>(data)->~T();  // NOLINT
    } else {
      VariantHelper<Types...>::destroy(type_index, data);
    }
  }

  static void move(const TypeIndex old_type_index, void* old_value,
                   void* new_value) {
    if (old_type_index == sizeof...(Types)) {
      new (new_value) T(std::move(*reinterpret_cast<T*>(old_value)));  // NOLINT
    } else {
      VariantHelper<Types...>::move(old_type_index, old_value, new_value);
    }
  }

  static void copy(const TypeIndex old_type_index, const void* old_value,
                   void* new_value) {
    if (old_type_index == sizeof...(Types)) {
      new (new_value) T(*reinterpret_cast<const T*>(old_value));  // NOLINT
    } else {
      VariantHelper<Types...>::copy(old_type_index, old_value, new_value);
    }
  }
};

template <>
struct VariantHelper<> {
  // LCOV_EXCL_START: never called
  static void destroy(const TypeIndex /*unused*/, void* /*unused*/) {}
  static void move(const TypeIndex /*unused*/, void* /*unused*/,
                   void* /*unused*/) {}
  static void copy(const TypeIndex /*unused*/, const void* /*unused*/,
                   void* /*unused*/) {}
  // LCOV_EXCL_STOP
};

}  // namespace internals

struct NoInit {};

template <typename... Types>
class Variant {
  static_assert(sizeof...(Types) > 0,
                "Template parameter type list of Variant can not be empty.");
  static_assert(!internals::Disjunction<std::is_reference<Types>...>::value,
                "Variant can not hold reference types. Maybe use "
                "std::reference_wrapper?");
  static_assert(!internals::Disjunction<std::is_array<Types>...>::value,
                "Variant can not hold array types.");
  static_assert(
      sizeof...(Types) < std::numeric_limits<internals::TypeIndex>::max(),
      "Internal index type must be able to accommodate all alternatives.");

 private:
  static const std::size_t data_size =
      internals::StaticMax<sizeof(Types)...>::value;
  static const std::size_t data_align =
      internals::StaticMax<alignof(Types)...>::value;

 public:
  using types = std::tuple<Types...>;

 private:
  using first_type = typename std::tuple_element<0, types>::type;
  using data_type = typename std::aligned_storage<data_size, data_align>::type;
  using helper_type = internals::VariantHelper<Types...>;

  internals::TypeIndex type_index;
  data_type data;

 public:
  Variant() noexcept(std::is_nothrow_default_constructible<first_type>::value)
      : type_index(sizeof...(Types)-1) {
    static_assert(std::is_default_constructible<first_type>::value,
                  "First type in Variant must be default constructible to "
                  "allow default construction of Variant.");
    new (&data) first_type();
  }

  Variant(NoInit) noexcept : type_index(internals::invalid_value) {}  // NOLINT

  template <typename T, typename Traits = internals::ValueTraits<T, Types...>,
            typename Enable = typename std::enable_if<
                Traits::is_valid &&
                !std::is_same<Variant<Types...>,
                              typename Traits::ValueType>::value>::type>
  Variant(T&& val)  // NOLINT
      noexcept(std::is_nothrow_constructible<typename Traits::TargetType,
                                             T&&>::value)
      : type_index(Traits::index) {
    new (&data) typename Traits::TargetType(std::forward<T>(val));
  }

  template <typename... Ts,
            typename = typename std::enable_if<internals::Disjunction<
                std::is_copy_constructible<Ts>...>::value>::type>
  Variant(const Variant<Types...>& old)  // NOLINT
      : type_index(old.type_index) {
    helper_type::copy(old.type_index, &old.data, &data);
  }

  template <typename... Ts,
            typename = typename std::enable_if<internals::Disjunction<
                std::is_convertible<Ts, Types>...>::value>::type>
  Variant(Variant<Ts...>&& old)  // NOLINT
      noexcept(internals::Conjunction<
               std::is_nothrow_move_constructible<Types>...>::value)
      : type_index(old.type_index) {
    helper_type::move(old.type_index, &old.data, &data);
  }

 private:
  template <typename... Ts,
            typename = typename std::enable_if<internals::Disjunction<
                std::is_copy_constructible<Ts>...>::value>::type>
  void copy_assign(const Variant<Ts...>& rhs) {
    helper_type::destroy(type_index, &data);
    type_index = internals::invalid_value;
    helper_type::copy(rhs.type_index, &rhs.data, &data);
    type_index = rhs.type_index;
  }

  void move_assign(Variant<Types...>&& rhs) {
    helper_type::destroy(type_index, &data);
    type_index = internals::invalid_value;
    helper_type::move(rhs.type_index, &rhs.data, &data);
    type_index = rhs.type_index;
  }

 public:
  Variant<Types...>& operator=(Variant<Types...>&& other) noexcept {
    if (this != &other) move_assign(std::move(other));
    return *this;
  }

  Variant<Types...>& operator=(const Variant<Types...>& other) {
    if (this != &other) copy_assign(other);
    return *this;
  }

  // conversions
  // move-assign
  template <typename T>
  Variant<Types...>& operator=(T&& rhs) noexcept {
    Variant<Types...> temp(std::forward<T>(rhs));
    move_assign(std::move(temp));
    return *this;
  }

  // copy-assign
  template <typename T>
  Variant<Types...>& operator=(const T& rhs) {
    Variant<Types...> temp(rhs);
    copy_assign(temp);
    return *this;
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  bool is() const {
    return type_index == internals::DirectType<T, Types...>::index;
  }

  bool valid() const { return type_index != internals::invalid_value; }

  template <typename T, typename... Args>
  void set(Args&&... args) {
    helper_type::destroy(type_index, &data);
    type_index = internals::invalid_value;
    new (&data) T(std::forward<Args>(args)...);
    type_index = internals::DirectType<T, Types...>::index;
  }

  // get<T>()
  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  T& get() {
    if (type_index == internals::DirectType<T, Types...>::index)
      return get_unchecked<T>();  // NOLINT
    throw BadVariantAccess("in get<T>()");
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  const T& get() const {
    if (type_index == internals::DirectType<T, Types...>::index)
      return get_unchecked<T>();  // NOLINT
    throw BadVariantAccess("in get<T>()");
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  T&& consume() {
    if (type_index == internals::DirectType<T, Types...>::index) {
      return consume_unchecked<T>();
    }
    throw BadVariantAccess("in get<T>()");
  }

  // get_unchecked<T>()
  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  T& get_unchecked() {
    return *reinterpret_cast<T*>(&data);  // NOLINT
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  const T& get_unchecked() const {
    return *reinterpret_cast<const T*>(&data);  // NOLINT
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  T&& consume_unchecked() {
    T&& res = std::move(*reinterpret_cast<T*>(&data));  // NOLINT
    helper_type::destroy(type_index, &data);
    type_index = internals::invalid_value;
    return std::move(res);
  }

  // get<T>() - T stored as std::reference_wrapper<T>
  template <
      typename T,
      typename std::enable_if<
          (internals::DirectType<std::reference_wrapper<T>, Types...>::index !=
           internals::invalid_value)>::type* = nullptr>
  T& get() {
    if (type_index == internals::DirectType<T, Types...>::index)
      return get_unchecked<T>();
    throw BadVariantAccess("in get<T>()");
  }

  template <typename T,
            typename std::enable_if<
                (internals::DirectType<std::reference_wrapper<const T>,
                                       Types...>::index !=
                 internals::invalid_value)>::type* = nullptr>
  const T& get() const {
    if (type_index == internals::DirectType<T, Types...>::index)
      return get_unchecked<T>();
    throw BadVariantAccess("in get<T>()");
  }

  template <
      typename T,
      typename std::enable_if<
          (internals::DirectType<std::reference_wrapper<T>, Types...>::index !=
           internals::invalid_value)>::type* = nullptr>
  T&& consume() {
    if (type_index == internals::DirectType<T, Types...>::index)
      return consume_unchecked<T>();
    throw BadVariantAccess("in get<T>()");
  }

  // get_unchecked<T>() - T stored as std::reference_wrapper<T>
  template <
      typename T,
      typename std::enable_if<
          (internals::DirectType<std::reference_wrapper<T>, Types...>::index !=
           internals::invalid_value)>::type* = nullptr>
  T& get_unchecked() {
    return (*reinterpret_cast<std::reference_wrapper<T>*>(&data))  // NOLINT
        .get();
  }

  template <typename T,
            typename std::enable_if<
                (internals::DirectType<std::reference_wrapper<const T>,
                                       Types...>::index !=
                 internals::invalid_value)>::type* = nullptr>
  const T& get_unchecked() const {
    return (*reinterpret_cast<const std::reference_wrapper<const T>  // NOLINT
                                  *>(&data))
        .get();
  }

  template <
      typename T,
      typename std::enable_if<
          (internals::DirectType<std::reference_wrapper<T>, Types...>::index !=
           internals::invalid_value)>::type* = nullptr>
  T&& consume_unchecked() {
    auto&& res = std::move(
        (*reinterpret_cast<std::reference_wrapper<T>*>(&data))  // NOLINT
            .get());
    helper_type::destroy(type_index, &data);
    type_index = internals::invalid_value;
    return std::move(res);
  }

  int which() const noexcept {
    return static_cast<int>(sizeof...(Types)-type_index - 1);
  }

  template <typename T, typename std::enable_if<
                            (internals::DirectType<T, Types...>::index !=
                             internals::invalid_value)>::type* = nullptr>
  static constexpr int which() noexcept {
    return static_cast<int>(
        sizeof...(Types)-internals::DirectType<T, Types...>::index - 1);
  }

  ~Variant() noexcept  // no-throw destructor
  {
    helper_type::destroy(type_index, &data);
  }

  template <typename... Ts>
  friend class Variant;
};