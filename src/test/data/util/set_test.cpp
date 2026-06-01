#include <doctest.h>

#include "prelude.hpp"

TEST_SUITE_BEGIN("Set");

using namespace amelia;

TEST_CASE("add, has") {
  Set<String> set;
  CHECK(set.size() == 0);
  set.add("a");
  CHECK(set.size() == 1);
  set.add("b");
  CHECK(set.size() == 2);
  CHECK(set.has("a"));
  CHECK(set.has("b"));

  const Set<String> &const_set = set;
  CHECK(const_set.has("a"));
  CHECK(const_set.has("b"));
}

TEST_CASE("clear") {
  Set<String> set = {"a", "b", "c"};
  CHECK(set.has("a"));
  CHECK(set.has("b"));
  CHECK(set.has("c"));
  set.clear();
  CHECK(!set.has("a"));
  CHECK(!set.has("b"));
  CHECK(!set.has("c"));
}

TEST_CASE("remove") {
  Set<String> set;
  set.add("a");
  set.add("b");
  CHECK(set.size() == 2);
  set.remove("a");
  CHECK(set.size() == 1);
  CHECK(!set.has("a"));
  CHECK(set.has("b"));

  set.remove("c"); // Removing a non-existent key should do nothing
  CHECK(!set.has("a"));
  CHECK(set.has("b"));
}

TEST_CASE("equals") {
  Set<String> set1 = {"a", "b", "c"};
  Set<String> set2 = {"a", "b", "c"};
  Set<String> set3 = {"a", "b"};
  Set<String> set4 = {"a", "b", "c"};

  CHECK(set1 == set2);
  CHECK(set1 != set3);
  CHECK(set1 == set4);
  CHECK(set2 != set3);
  CHECK(set2 == set4);
  CHECK(set3 != set4);

  set4.remove("c");
  CHECK(set3 == set4);

  set3.add("c");
  CHECK(set3 != set4);

  set3.remove("c");
  CHECK(set3 == set4);
}

TEST_CASE("iterator") {
  Set<String> set = {"a", "b", "c"};
  Set<String> seen;
  for (auto &value : set) {
    seen.add(value);
  }
  CHECK(seen.has("a"));
  CHECK(seen.has("b"));
  CHECK(seen.has("c"));
}

TEST_CASE("iterator - manual") {
  Set<String> set = {"a", "b", "c"};
  auto it = set.begin();
  auto end = set.end();
  Set<String> seen;
  for (; it != end; ++it) {
    const auto &value = *it;
    seen.add(value);
  }
  CHECK(seen.has("a"));
  CHECK(seen.has("b"));
  CHECK(seen.has("c"));

  CHECK_THROWS_AS(*it, std::runtime_error);
  CHECK_THROWS_AS(*end, std::runtime_error);
}
