#include "forward_like.hpp"

struct probe {};

template <typename M> struct S {
  M m;
  using value_type = M;
};

template <typename T>
using fwd_type = decltype((std::forward<T>(std::declval<T>()).m));

template <typename T>
using like_type_lang =
    decltype(flang::forward<T, decltype(std::declval<T>().m)>(
        std::declval<T>().m));

template <typename T, typename Expected> void test_like_lang() {
  using fwd = fwd_type<T>;
  using like = like_type_lang<T>;
  using diff = decltype(FWD(std::declval<T>().m));
  is_same<Expected, fwd>(); // test expectations on std::forward
  is_same<fwd, like>();
  is_same<diff, fwd>();
}

// comment
// to
// align
// functions
void test_like_lang_lambdas() {
  using flang::forward;

  probe x;
  {
    // by-value
    auto l = [x](auto &&self) mutable {
      is_same<_copy_ref_t<decltype(self), probe>,
              decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
  {
    // by-ref capture
    auto l = [&x, &y = x](auto &&self) mutable {
      // correct
      is_same<probe &, decltype(forward<decltype(self), decltype(y)>(y))>();
      // WRONG! we don't own X, and yet we move
      is_same<_copy_ref_t<decltype(self), probe>,
              decltype(forward<decltype(self), decltype(x)>(x))>();
    };
    l(l);            // lvalue-call emulation
    l(std::move(l)); // sortish like a this-auto-self with a && call operator
  }
}

void test_forward_like_lang() {
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

  test_like_lang_lambdas();
}

int main() { test_forward_like_lang(); }