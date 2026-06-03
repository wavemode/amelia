#include <doctest.h>

#include "prelude.hpp"

#include "data/util/option.hpp"

TEST_SUITE_BEGIN("Option");

using namespace amelia;

TEST_CASE("general usage") {
  Option<String> opt;
  CHECK(!opt.has_value());

  opt = "Hello";
  CHECK(opt.has_value());
  CHECK(opt.value() == "Hello");
  CHECK(opt == "Hello");
  CHECK(opt != "World");

  opt.value() = "World";
  CHECK(opt.value() == "World");
  CHECK((*opt).begin().peek() == 'W');
  CHECK(opt->size() == 5);
  opt->append("!");
  CHECK(opt->text() == "World!");

  opt = None();
  CHECK(!opt.has_value());
  CHECK_THROWS_AS(opt.value(), RuntimeError);
  CHECK_THROWS_AS(*opt, RuntimeError);

  Option<int> opt2 = Some(10);
  CHECK(opt2.has_value());
  CHECK(opt2.value() == 10);
  CHECK(opt2 == 10);
  CHECK(opt2 != 20);
  CHECK(opt2 == Some(10));
  CHECK(opt2 != Some(20));
  opt2 = 20;
  CHECK(opt2.value() == 20);
  opt2 = Some(30);
  CHECK(*opt2 == 30);

  Option<int *> opt3 = &*opt2;
  **opt3 = 40;
  CHECK(opt2.value() == 40);
}
TEST_SUITE_END();
