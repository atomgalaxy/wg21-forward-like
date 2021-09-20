#include "forward_like.hpp"

struct probe {};

template <typename M> struct S {
  M m;
  using value_type = M;
};

template <typename T, typename Expected> void test_like_merge() {
  using like = decltype(fmrg::forward_like<T>(std::declval<T>().m));
  is_same<Expected, like>();
}

void test_like_merge_lambdas() {
  using fmrg::forward_like;

  probe x;
  probe z;
  auto l = [x, &y = x, z](auto &&self) mutable {
    // correct, this is what we *meant*, consistently
    // If we didn't mean to forward the capture, we wouldn't have used
    // forward_like.
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(forward_like<decltype(self)>(y))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(forward_like<decltype(self)>(x))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(forward_like<decltype(self)>(z))>();
  };
  l(l);            // lvalue-call emulation
  l(std::move(l)); // sortish like a this-auto-self with a && call operator
}

void test_forward_like_merge() {
  test_like_merge<S<probe>, probe &&>();
  test_like_merge<S<probe> &, probe &>();
  test_like_merge<S<probe> &&, probe &&>();
  test_like_merge<S<probe> const, probe const &&>();
  test_like_merge<S<probe> const &, probe const &>();
  test_like_merge<S<probe> const &&, probe const &&>();
  test_like_merge<S<probe &>, probe &&>();
  test_like_merge<S<probe &> &, probe &>();
  test_like_merge<S<probe &> &&, probe &&>();
  test_like_merge<S<probe &> const, probe const &&>();
  test_like_merge<S<probe &> const &, probe const &>();
  test_like_merge<S<probe &> const &&, probe const &&>();
  test_like_merge<S<probe &&>, probe &&>();
  test_like_merge<S<probe &&> &, probe &>();
  test_like_merge<S<probe &&> &&, probe &&>();
  test_like_merge<S<probe &&> const, probe const &&>();
  test_like_merge<S<probe &&> const &, probe const &>();
  test_like_merge<S<probe &&> const &&, probe const &&>();
  test_like_merge<S<probe const>, probe const &&>();
  test_like_merge<S<probe const> &, probe const &>();
  test_like_merge<S<probe const> &&, probe const &&>();
  test_like_merge<S<probe const> const, probe const &&>();
  test_like_merge<S<probe const> const &, probe const &>();
  test_like_merge<S<probe const> const &&, probe const &&>();
  test_like_merge<S<probe const &>, probe const &&>();
  test_like_merge<S<probe const &> &, probe const &>();
  test_like_merge<S<probe const &> &&, probe const &&>();
  test_like_merge<S<probe const &> const, probe const &&>();
  test_like_merge<S<probe const &> const &, probe const &>();
  test_like_merge<S<probe const &> const &&, probe const &&>();
  test_like_merge<S<probe const &&>, probe const &&>();
  test_like_merge<S<probe const &&> &, probe const &>();
  test_like_merge<S<probe const &&> &&, probe const &&>();
  test_like_merge<S<probe const &&> const, probe const &&>();
  test_like_merge<S<probe const &&> const &, probe const &>();
  test_like_merge<S<probe const &&> const &&, probe const &&>();

  test_like_merge_lambdas();
}

int main() { test_forward_like_merge(); }