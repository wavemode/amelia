#include <doctest.h>

#include "prelude.hpp"

#include "data/util/set.hpp"

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
  auto set = Set<String>({"a", "b", "c"});
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
  auto set1 = Set<String>({"a", "b", "c"});
  auto set2 = Set<String>({"a", "b", "c"});
  auto set3 = Set<String>({"a", "b"});
  auto set4 = Set<String>({"a", "b", "c"});

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
  auto set = Set<String>({"a", "b", "c"});
  Set<String> seen;
  for (const String &value : set) {
    seen.add(value);
  }
  CHECK(seen.has("a"));
  CHECK(seen.has("b"));
  CHECK(seen.has("c"));
}

TEST_CASE("iterator - manual") {
  auto set = Set<String>({"a", "b", "c"});
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

  CHECK_THROWS_AS(*it, RuntimeError);
  CHECK_THROWS_AS(*end, RuntimeError);
}

TEST_CASE("stress") {
  Set<int> set;
  const int num_elements = 10000;
  for (int i = 0; i < num_elements; i++) {
    set.add(i);
    REQUIRE(set.has(i));
    REQUIRE(set.size() == static_cast<size_t>(i + 1));
  }
  for (int i = 0; i < num_elements; i++) {
    REQUIRE(set.has(i));
  }
  for (int i = num_elements - 1; i >= 0; i--) {
    set.remove(i);
    REQUIRE(!set.has(i));
    REQUIRE(set.size() == static_cast<size_t>(i));
  }
  REQUIRE(set.size() == 0);
  for (int i = 0; i < num_elements; i++) {
    REQUIRE(!set.has(i));
  }
  for (int i = num_elements + 1; i <= num_elements * 2; i++) {
    REQUIRE(!set.has(i));
    set.add(i);
    REQUIRE(set.has(i));
    REQUIRE(set.size() == static_cast<size_t>(i - num_elements));
  }
  REQUIRE(set.size() == static_cast<size_t>(num_elements));
}
