#pragma once

namespace std
{
  /// integral_constant
  template <class T, T v>
  struct integral_constant
  {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant; // using injected-class-name
    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; } // since c++14
  };

  typedef integral_constant<bool, true> true_type;
  typedef integral_constant<bool, false> false_type;

  /// remove_reference
  template <class T>
  struct remove_reference
  {
    typedef T type;
  };

  template <class T>
  struct remove_reference<T &>
  {
    typedef T type;
  };

  template <class T>
  struct remove_reference<T &&>
  {
    typedef T type;
  };

  /// is_same
  template <class T, class U>
  struct is_same : std::false_type
  {
  };

  template <class T>
  struct is_same<T, T> : std::true_type
  {
  };
}
