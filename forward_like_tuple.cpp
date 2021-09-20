#include "forward_like.hpp"

#include <tuple>

struct probe {};

template <typename M> struct S {
  M m;
  using value_type = M;
};

template <typename T>
using fwd_type = decltype((std::forward<T>(std::declval<T>()).m));

template <typename T, typename Expected> void test_like_tuple() {
  using ftpl::forward;

  using value_type = typename std::remove_cvref_t<T>::value_type;

  using like_1 =
      decltype(forward<T, decltype(std::declval<T>().m)>(std::declval<T>().m));
  using like_2 = decltype(forward<T, value_type>(std::declval<T>().m));

  using tuple_like_T = _copy_cvref_t<T, std::tuple<value_type>>;
  using tuple_does = decltype(std::get<0>(std::declval<tuple_like_T>()));
  is_same<Expected, tuple_does>(); // test expectations on tuple
  is_same<Expected, like_1>();
  is_same<Expected, like_2>();
}

void test_like_tuple_lambdas() {
  using ftpl::forward;

  probe x;
  {
    // by-value
    auto l = [x](auto &&self) mutable -> decltype(auto) {
      is_same<_copy_ref_t<decltype(self), probe>,
              decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
  {
    // by-ref capture
    auto l = [&x, &y = x](auto &&self) mutable {
      is_same<probe &, decltype(forward<decltype(self), decltype(y)>(y))>();
      // WRONG! we don't own X, and yet we move
      is_same<_copy_ref_t<decltype(self), probe>,
              decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
}

void test_forward_like_tuple() {
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

  test_like_tuple_lambdas();
}

int main() { test_forward_like_tuple(); }