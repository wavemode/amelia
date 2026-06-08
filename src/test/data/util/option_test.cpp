#include <doctest.h>

#include "prelude.hpp"

#include "data/util/option.hpp"

TEST_SUITE_BEGIN("Option");

using namespace amelia;

TEST_CASE("general usage") {
  Option<String> opt;
  REQUIRE(!opt.has_value());

  opt = "Hello";
  REQUIRE(opt.has_value());
  REQUIRE(opt.value() == "Hello");
  REQUIRE(opt == "Hello");
  REQUIRE(opt != "World");

  opt.value() = "World";
  REQUIRE(opt.value() == "World");
  REQUIRE((*opt).begin().peek() == 'W');
  REQUIRE(opt->size() == 5);
  opt->append("!");
  REQUIRE(opt->text() == "World!");

  opt = None();
  REQUIRE(!opt.has_value());
  CHECK_THROWS_AS(opt.value(), RuntimeError);
  CHECK_THROWS_AS(*opt, RuntimeError);

  Option<int> opt2 = Some(10);
  REQUIRE(opt2.has_value());
  REQUIRE(opt2.value() == 10);
  REQUIRE(opt2 == 10);
  REQUIRE(opt2 != 20);
  REQUIRE(opt2 == Some(10));
  REQUIRE(opt2 != Some(20));
  opt2 = 20;
  REQUIRE(opt2.value() == 20);
  opt2 = Some(30);
  REQUIRE(*opt2 == 30);

  Option<int *> opt3 = &*opt2;
  **opt3 = 40;
  REQUIRE(opt2.value() == 40);
}
TEST_SUITE_END();
