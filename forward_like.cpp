#include "forward_like.hpp"

#include <memory>
#include <string>

struct probe {};

template <typename M> struct S {
  M m;
  using value_type = M;
};

template <typename T, typename Merge, typename Tuple, typename Lang>
void test() {
  using value_type = typename std::remove_cvref_t<T>::value_type;

  using mrg = decltype(fmrg::forward_like<T>(std::declval<value_type>()));
  using tpl_model = decltype(std::get<0>(
      std::declval<_copy_cvref_t<T, std::tuple<value_type>>>()));
  using tpl =
      decltype(ftpl::forward<T, value_type>(std::declval<value_type>()));
  using lng_model = decltype((std::forward<T>(std::declval<T>()).m));
  using lng =
      decltype(flang::forward<T, value_type>(std::declval<value_type>()));

  is_same<Merge, mrg>();
  is_same<Tuple, tpl>();
  is_same<Lang, lng>();
  // sanity checks
  is_same<Tuple, tpl_model>();
  is_same<Lang, lng_model>();
}

void test() {
  using p = probe;
  // clang-format off
  //   TEST TYPE             ,'merge'    ,'tuple'    ,'language'
  test<S<p         >         , p &&      , p &&      , p &&      >();
  test<S<p         > &       , p &       , p &       , p &       >();
  test<S<p         > &&      , p &&      , p &&      , p &&      >();
  test<S<p         > const   , p const &&, p const &&, p const &&>();
  test<S<p         > const & , p const & , p const & , p const & >();
  test<S<p         > const &&, p const &&, p const &&, p const &&>();
  test<S<p const   >         , p const &&, p const &&, p const &&>();
  test<S<p const   > &       , p const & , p const & , p const & >();
  test<S<p const   > &&      , p const &&, p const &&, p const &&>();
  test<S<p const   > const   , p const &&, p const &&, p const &&>();
  test<S<p const   > const & , p const & , p const & , p const & >();
  test<S<p const   > const &&, p const &&, p const &&, p const &&>();
  test<S<p &       > &       , p &       , p &       , p &       >();
  test<S<p &&      > &       , p &       , p &       , p &       >();
  test<S<p const & > &       , p const & , p const & , p const & >();
  test<S<p const &&> &       , p const & , p const & , p const & >();
  test<S<p const & > const & , p const & , p const & , p const & >();
  test<S<p const &&> const & , p const & , p const & , p const & >();

  test<S<p &       >         , p &&      , p &       , p &       >();
  test<S<p &       > &&      , p &&      , p &       , p &       >();
  test<S<p &       > const   , p const &&, p &       , p &       >();
  test<S<p &       > const & , p const & , p &       , p &       >();
  test<S<p &       > const &&, p const &&, p &       , p &       >();
  test<S<p &&      >         , p &&      , p &&      , p &       >();
  test<S<p &&      > &&      , p &&      , p &&      , p &       >();
  test<S<p &&      > const   , p const &&, p &&      , p &       >();
  test<S<p &&      > const & , p const & , p &       , p &       >();
  test<S<p &&      > const &&, p const &&, p &&      , p &       >();
  test<S<p const & >         , p const &&, p const & , p const & >();
  test<S<p const & > &&      , p const &&, p const & , p const & >();
  test<S<p const & > const   , p const &&, p const & , p const & >();
  test<S<p const & > const &&, p const &&, p const & , p const & >();
  test<S<p const &&>         , p const &&, p const &&, p const & >();
  test<S<p const &&> &&      , p const &&, p const &&, p const & >();
  test<S<p const &&> const   , p const &&, p const &&, p const & >();
  test<S<p const &&> const &&, p const &&, p const &&, p const & >();
  // clang-format on
}

void test_lambdas() {
  probe x;
  probe z;
  auto l = [x, &y = x, z](auto &&self) mutable {
    // correct, this is what we *meant*, consistently
    // If we didn't mean to forward the capture, we wouldn't have used
    // forward_like.
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(fmrg::forward_like<decltype(self)>(y))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(fmrg::forward_like<decltype(self)>(x))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(fmrg::forward_like<decltype(self)>(z))>();

    // x and y behave differently with the tuple model (problem)
    is_same<probe &, decltype(ftpl::forward<decltype(self), decltype(y)>(y))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(ftpl::forward<decltype(self), decltype(x)>(x))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(ftpl::forward<decltype(self), decltype(z)>(z))>();

    // x and y behave differently with the language model (problem)
    is_same<probe &,
            decltype(flang::forward<decltype(self), decltype(y)>(y))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(flang::forward<decltype(self), decltype(x)>(x))>();
    is_same<_override_ref_t<decltype(self), probe>,
            decltype(flang::forward<decltype(self), decltype(z)>(z))>();
  };
  l(l);            // lvalue-call emulation
  l(std::move(l)); // sortish like a this-auto-self with a && call operator
}

struct owns_far_string {
  std::unique_ptr<std::string> s;
};

void test_far_objects() {
  // problem is that *unique_ptr returns a reference
  owns_far_string fs;
  auto l = [](auto &&fs) {
    using mrg = decltype(fmrg::forward_like<decltype(fs)>(*fs.s));
    using tpl = decltype(ftpl::forward<decltype(fs), decltype(*fs.s)>(*fs.s));
    using lng = decltype(flang::forward<decltype(fs), decltype(*fs.s)>(*fs.s));

    // fit for purpose
    is_same<_override_ref_t<decltype(fs), std::string>, mrg>();
    // these are not fit for purpose
    is_same<std::string &, tpl>();
    is_same<std::string &, lng>();
  };
  l(fs);            // lvalue call
  l(std::move(fs)); // rvalue call - we want to move the string out
}

int main() {
  test();
  test_lambdas();
  test_far_objects();
}