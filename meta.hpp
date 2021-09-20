#pragma once

#include <type_traits>
#include <utility>

template <typename T, typename U>
concept _similar =
    std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

template <typename T, typename U>
using _copy_ref_t = std::conditional_t<
    std::is_rvalue_reference_v<T>, U &&,
    std::conditional_t<std::is_lvalue_reference_v<T>, U &, U>>;

template <typename T, typename U>
using _override_ref_t = std::conditional_t<std::is_rvalue_reference_v<T>,
                                           std::remove_reference_t<U> &&, U &>;

template <typename T, typename U>
using _copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
                       _copy_ref_t<U, std::remove_reference_t<U> const>, U>;

template <typename T>
constexpr bool _is_reference_v =
    std::is_lvalue_reference_v<T> || std::is_rvalue_reference_v<T>;

template <typename T, typename U>
using _copy_cvref_t = _copy_ref_t<T &&, _copy_const_t<T, U>>;

// test utilities

#define FWD(...) std::forward<decltype((__VA_ARGS__))>(__VA_ARGS__)

template <typename Expected, typename Actual> constexpr void is_same() {
  static_assert(std::is_same_v<Expected, Actual>);
}