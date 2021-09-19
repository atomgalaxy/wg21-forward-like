#include <tuple>
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
using _copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
                       _copy_ref_t<U, std::remove_reference_t<U> const>, U>;

template <typename T>
constexpr bool _is_reference_v =
    std::is_lvalue_reference_v<T> || std::is_rvalue_reference_v<T>;

template <typename T, typename U>
using _copy_cvref_t = _copy_ref_t<T &&, _copy_const_t<T, U>>;

template <typename T, typename U>
using _fwd_like_tuple_t =
    std::conditional_t<_is_reference_v<U>, _copy_ref_t<T, U>,
                       _copy_cvref_t<T, U>>;

template <typename T, typename U>
using _fwd_like_lang_t =
    std::conditional_t<_is_reference_v<U>, U &,
                       _copy_ref_t<T, _copy_const_t<T, U>> &&>;

// implementation
template <typename T, typename M, _similar<M> U>
auto forward_like_tuple(U &&x) noexcept -> decltype(auto) {
  return static_cast<_fwd_like_tuple_t<T, M>>(x);
}

template <typename T, typename M, _similar<M> U>
auto forward_like_lang(U &&x) noexcept -> decltype(auto) {
  return static_cast<_fwd_like_lang_t<T, _copy_const_t<U, M>>>(x);
}

template <typename T, typename M, _similar<M> U>
auto forward(U &&x) noexcept -> decltype(auto) {
  return forward_like_tuple<T, M>(static_cast<U &&>(x));
}

// test utilities
#define FWD(...) std::forward<decltype((__VA_ARGS__))>(__VA_ARGS__)
struct probe {};
template <typename M> struct S {
  M m;
  using value_type = M;
};

template <typename T>
using fwd_type = decltype((std::forward<T>(std::declval<T>()).m));

template <typename T>
using like_type_lang =
    decltype(forward_like_lang<T, decltype(std::declval<T>().m)>(
        std::declval<T>().m));

template <typename Expected, typename Actual> constexpr void is_same_exp() {
  static_assert(std::is_same_v<Expected, Actual>);
}

template <typename Fwd, typename FwdLike> constexpr void is_same_fwd() {
  static_assert(std::is_same_v<Fwd, FwdLike>);
}

template <typename T, typename Expected> void test_like_tuple() {
  using value_type = typename std::remove_cvref_t<T>::value_type;
  using like_1 = decltype(forward_like_tuple<T, decltype(std::declval<T>().m)>(
      std::declval<T>().m));
  using like_2 =
      decltype(forward_like_tuple<T, value_type>(std::declval<T>().m));

  using tuple_like_T = _copy_cvref_t<T, std::tuple<value_type>>;
  using tuple_does = decltype(std::get<0>(std::declval<tuple_like_T>()));
  is_same_exp<Expected, tuple_does>(); // test expectations on tuple
  is_same_exp<Expected, like_1>();
  is_same_exp<Expected, like_2>();
}

template <typename T, typename Expected> void test_forward() {
  using value_type = typename std::remove_cvref_t<T>::value_type;
  using like_1 =
      decltype(forward<T, decltype(std::declval<T>().m)>(std::declval<T>().m));
  using like_2 = decltype(forward<T, value_type>(std::declval<T>().m));

  is_same_exp<Expected, like_1>();
  is_same_exp<Expected, like_2>();
}

int test_forward_like_tuple() {
  test_like_tuple<S<probe>, probe &&>();
  test_like_tuple<S<probe> &, probe &>();
  test_like_tuple<S<probe> &&, probe &&>();
  test_like_tuple<S<probe> const, probe const &&>();
  test_like_tuple<S<probe> const &, probe const &>();
  test_like_tuple<S<probe> const &&, probe const &&>();
  test_like_tuple<S<probe &>, probe &>();
  test_like_tuple<S<probe &> &, probe &>();
  test_like_tuple<S<probe &> &&, probe &>();
  test_like_tuple<S<probe &> const, probe &>();
  test_like_tuple<S<probe &> const &, probe &>();
  test_like_tuple<S<probe &> const &&, probe &>();
  test_like_tuple<S<probe &&>, probe &&>();
  test_like_tuple<S<probe &&> &, probe &>();
  test_like_tuple<S<probe &&> &&, probe &&>();
  test_like_tuple<S<probe &&> const, probe &&>();
  test_like_tuple<S<probe &&> const &, probe &>();
  test_like_tuple<S<probe &&> const &&, probe &&>();
  test_like_tuple<S<probe const>, probe const &&>();
  test_like_tuple<S<probe const> &, probe const &>();
  test_like_tuple<S<probe const> &&, probe const &&>();
  test_like_tuple<S<probe const> const, probe const &&>();
  test_like_tuple<S<probe const> const &, probe const &>();
  test_like_tuple<S<probe const> const &&, probe const &&>();
  test_like_tuple<S<probe const &>, probe const &>();
  test_like_tuple<S<probe const &> &, probe const &>();
  test_like_tuple<S<probe const &> &&, probe const &>();
  test_like_tuple<S<probe const &> const, probe const &>();
  test_like_tuple<S<probe const &> const &, probe const &>();
  test_like_tuple<S<probe const &> const &&, probe const &>();
  test_like_tuple<S<probe const &&>, probe const &&>();
  test_like_tuple<S<probe const &&> &, probe const &>();
  test_like_tuple<S<probe const &&> &&, probe const &&>();
  test_like_tuple<S<probe const &&> const, probe const &&>();
  test_like_tuple<S<probe const &&> const &, probe const &>();
  test_like_tuple<S<probe const &&> const &&, probe const &&>();

  probe x;

  {
    // by-value
    auto l = [x](auto &&self) mutable -> decltype(auto) {
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward_like_tuple<decltype(self), decltype(x)>(
                      x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
  {
    // by-ref capture
    auto l = [&x, &y = x](auto &&self) mutable {
      is_same_exp<probe &, decltype(forward_like_tuple<decltype(self),
                                                       decltype(y)>(y))>();
      // WRONG! we don't own X, and yet we move
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward_like_lang<decltype(self), decltype(x)>(
                      x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }

  return 0;
}

int test_forward() {
  test_forward<S<probe>, probe &&>();
  test_forward<S<probe> &, probe &>();
  test_forward<S<probe> &&, probe &&>();
  test_forward<S<probe> const, probe const &&>();
  test_forward<S<probe> const &, probe const &>();
  test_forward<S<probe> const &&, probe const &&>();
  test_forward<S<probe &>, probe &>();
  test_forward<S<probe &> &, probe &>();
  test_forward<S<probe &> &&, probe &>();
  test_forward<S<probe &> const, probe &>();
  test_forward<S<probe &> const &, probe &>();
  test_forward<S<probe &> const &&, probe &>();
  test_forward<S<probe &&>, probe &&>();
  test_forward<S<probe &&> &, probe &>();
  test_forward<S<probe &&> &&, probe &&>();
  test_forward<S<probe &&> const, probe &&>();
  test_forward<S<probe &&> const &, probe &>();
  test_forward<S<probe &&> const &&, probe &&>();
  test_forward<S<probe const>, probe const &&>();
  test_forward<S<probe const> &, probe const &>();
  test_forward<S<probe const> &&, probe const &&>();
  test_forward<S<probe const> const, probe const &&>();
  test_forward<S<probe const> const &, probe const &>();
  test_forward<S<probe const> const &&, probe const &&>();
  test_forward<S<probe const &>, probe const &>();
  test_forward<S<probe const &> &, probe const &>();
  test_forward<S<probe const &> &&, probe const &>();
  test_forward<S<probe const &> const, probe const &>();
  test_forward<S<probe const &> const &, probe const &>();
  test_forward<S<probe const &> const &&, probe const &>();
  test_forward<S<probe const &&>, probe const &&>();
  test_forward<S<probe const &&> &, probe const &>();
  test_forward<S<probe const &&> &&, probe const &&>();
  test_forward<S<probe const &&> const, probe const &&>();
  test_forward<S<probe const &&> const &, probe const &>();
  test_forward<S<probe const &&> const &&, probe const &&>();

  probe x;

  {
    // by-value
    auto l = [x](auto &&self) mutable {
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
  {
    // by-ref capture
    auto l = [&x, &y = x](auto &&self) mutable {
      is_same_exp<probe &, decltype(forward<decltype(self), decltype(y)>(y))>();
      // WRONG! we don't own X, and yet we move
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }

  return 0;
}

template <typename T, typename Expected> void test_like_lang() {
  using fwd = fwd_type<T>;
  using like = like_type_lang<T>;
  using diff = decltype(FWD(std::declval<T>().m));
  is_same_exp<Expected, fwd>(); // test expectations on std::forward
  is_same_fwd<fwd, like>();
  is_same_fwd<diff, fwd>();
}

int test_forward_like_lang() {
  test_like_lang<S<probe>, probe &&>();
  test_like_lang<S<probe> &, probe &>();
  test_like_lang<S<probe> &&, probe &&>();
  test_like_lang<S<probe> const, probe const &&>();
  test_like_lang<S<probe> const &, probe const &>();
  test_like_lang<S<probe> const &&, probe const &&>();
  test_like_lang<S<probe &>, probe &>();
  test_like_lang<S<probe &> &, probe &>();
  test_like_lang<S<probe &> &&, probe &>();
  test_like_lang<S<probe &> const, probe &>();
  test_like_lang<S<probe &> const &, probe &>();
  test_like_lang<S<probe &> const &&, probe &>();
  test_like_lang<S<probe &&>, probe &>();
  test_like_lang<S<probe &&> &, probe &>();
  test_like_lang<S<probe &&> &&, probe &>();
  test_like_lang<S<probe &&> const, probe &>();
  test_like_lang<S<probe &&> const &, probe &>();
  test_like_lang<S<probe &&> const &&, probe &>();
  test_like_lang<S<probe const>, probe const &&>();
  test_like_lang<S<probe const> &, probe const &>();
  test_like_lang<S<probe const> &&, probe const &&>();
  test_like_lang<S<probe const> const, probe const &&>();
  test_like_lang<S<probe const> const &, probe const &>();
  test_like_lang<S<probe const> const &&, probe const &&>();
  test_like_lang<S<probe const &>, probe const &>();
  test_like_lang<S<probe const &> &, probe const &>();
  test_like_lang<S<probe const &> &&, probe const &>();
  test_like_lang<S<probe const &> const, probe const &>();
  test_like_lang<S<probe const &> const &, probe const &>();
  test_like_lang<S<probe const &> const &&, probe const &>();
  test_like_lang<S<probe const &&>, probe const &>();
  test_like_lang<S<probe const &&> &, probe const &>();
  test_like_lang<S<probe const &&> &&, probe const &>();
  test_like_lang<S<probe const &&> const, probe const &>();
  test_like_lang<S<probe const &&> const &, probe const &>();
  test_like_lang<S<probe const &&> const &&, probe const &>();

  probe x;

  {
    // by-value
    auto l = [x](auto &&self) mutable {
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward_like_lang<decltype(self), decltype(x)>(
                      x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
  {
    // by-ref capture
    auto l = [&x, &y = x](auto &&self) mutable {
      // correct
      is_same_exp<probe &, decltype(forward_like_lang<decltype(self),
                                                      decltype(y)>(y))>();
      // WRONG! we don't own X, and yet we move
      is_same_exp<_copy_ref_t<decltype(self), probe>,
                  decltype(forward_like_lang<decltype(self), decltype(x)>(
                      x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }

  return 0;
}

int main() {
  test_forward_like_tuple();
  test_forward_like_lang();
}