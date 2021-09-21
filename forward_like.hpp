#pragma once

#include "meta.hpp"
#include <tuple>

namespace ftpl {
using std::forward;

template <typename T, typename U>
using _fwd_like_tuple_t =
    std::conditional_t<_is_reference_v<U>, _copy_ref_t<T, U>,
                       _copy_cvref_t<T, U>>;

// implementation
template <typename T, typename M, _similar<M> U>
auto forward_like_tuple(U &&x) noexcept -> decltype(auto) {
  return static_cast<_fwd_like_tuple_t<T, M>>(x);
}

template <typename T, typename M, _similar<M> U>
auto forward(U &&x) noexcept -> decltype(auto) {
  return forward_like_tuple<T, M>(static_cast<U &&>(x));
}
} // namespace ftpl

namespace flang {
using std::forward;
template <typename T, typename U>
using _fwd_like_lang_t =
    std::conditional_t<_is_reference_v<U>, U &,
                       _copy_ref_t<T, _copy_const_t<T, U>> &&>;
template <typename T, typename M, _similar<M> U>
auto forward(U &&x) noexcept -> decltype(auto) {
  return static_cast<_fwd_like_lang_t<T, _copy_const_t<U, M>>>(x);
}
} // namespace flang

namespace fmrg {
template <typename T, typename U>
using _copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, U const, U>;

template <typename T, typename U>
using _fwd_like_merge_t =
    _override_ref_t<T &&, _copy_const_t<T, std::remove_reference_t<U>>>;

template <typename T, typename U>
auto forward_like(U &&x) noexcept -> decltype(auto) {
  return static_cast<_fwd_like_merge_t<T, U>>(x);
}
} // namespace fmrg