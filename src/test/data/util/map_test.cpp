#include <doctest.h>

#include "prelude.hpp"

#include "data/util/map.hpp"

TEST_SUITE_BEGIN("Map");

using namespace amelia;

TEST_CASE("get, set") {
  Map<String, int> map;
  REQUIRE(map.size() == 0);
  map.set("a", 1);
  REQUIRE(map.size() == 1);
  map.set("b", 2);
  REQUIRE(map.size() == 2);
  REQUIRE(map.get("a") == 1);
  REQUIRE(map.get("b") == 2);

  const Map<String, int> &const_map = map;
  REQUIRE(const_map.get("a") == 1);
  REQUIRE(const_map.get("b") == 2);
}

TEST_CASE("has and clear") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  REQUIRE(map.has("a"));
  REQUIRE(map.has("b"));
  REQUIRE(map.has("c"));

  map.clear();
  REQUIRE(!map.has("a"));
  REQUIRE(!map.has("b"));
  REQUIRE(!map.has("c"));
}

TEST_CASE("indexing operator") {
  Map<String, int> map;
  map.set("a", 1);
  map.set("b", 2);
  REQUIRE(map["a"] == 1);
  REQUIRE(map["b"] == 2);

  const Map<String, int> &const_map = map;
  REQUIRE(const_map["a"] == 1);
  REQUIRE(const_map["b"] == 2);

  CHECK_THROWS_AS(map["c"], RuntimeError);
  CHECK_THROWS_AS(const_map["c"], RuntimeError);
}

TEST_CASE("find") {
  Map<String, int> map;
  map.set("a", 1);
  auto a_value = map.find("a");
  REQUIRE(a_value.has_value());
  REQUIRE(**a_value == 1);
  **a_value = 10;
  REQUIRE(map.get("a") == 10);
  REQUIRE(map.find("b") == None());

  const Map<String, int> &const_map = map;
  auto const_a_value = const_map.find("a");
  REQUIRE(const_a_value.has_value());
  REQUIRE(**const_a_value == 10);
  REQUIRE(const_map.find("b") == None());
}

TEST_CASE("remove and remove_and_get") {
  Map<String, int> map;
  map.set("a", 1);
  map.set("b", 2);
  REQUIRE(map.size() == 2);
  map.remove("a");
  REQUIRE(map.size() == 1);
  REQUIRE(!map.has("a"));
  REQUIRE(map.has("b"));

  map.remove("c"); // Removing a non-existent key should do nothing
  REQUIRE(!map.has("a"));
  REQUIRE(map.has("b"));

  int b_value = map.remove_and_get("b");
  REQUIRE(b_value == 2);
  REQUIRE(!map.has("b"));

  CHECK_THROWS_AS(map.remove_and_get("c"), RuntimeError);
}

TEST_CASE("equals") {
  auto map1 = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  auto map2 = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  auto map3 = Map<String, int>({{"a", 1}, {"b", 2}});
  auto map4 = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 4}});

  REQUIRE(map1 == map2);
  REQUIRE(map1 != map3);
  REQUIRE(map1 != map4);
  REQUIRE(map2 != map3);
  REQUIRE(map2 != map4);
  REQUIRE(map3 != map4);

  map4.remove("c");
  REQUIRE(map3 == map4);

  map3.set("b", 20);
  REQUIRE(map3 != map4);
  map3.set("b", 2);
  REQUIRE(map3 == map4);

  map3.set("c", 3);
  REQUIRE(map1 == map3);
  REQUIRE(map3 != map4);
}

TEST_CASE("pair iterator") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  Map<String, int> seen;
  for (auto [key, value] : map) {
    seen.set(key, value);
  }
  REQUIRE(seen.get("a") == 1);
  REQUIRE(seen.get("b") == 2);
  REQUIRE(seen.get("c") == 3);

  const Map<String, int> &const_map = map;
  Map<String, int> const_seen;
  for (const auto &[key, value] : const_map) {
    const_seen.set(key, value);
  }
  REQUIRE(const_seen.get("a") == 1);
  REQUIRE(const_seen.get("b") == 2);
  REQUIRE(const_seen.get("c") == 3);
}

TEST_CASE("pair iterator - manual") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  auto it = map.begin();
  auto end = map.end();
  Map<String, int> seen;
  for (; it != end; ++it) {
    const auto &[key, value] = *it;
    seen.set(key, value);
  }
  REQUIRE(seen.get("a") == 1);
  REQUIRE(seen.get("b") == 2);
  REQUIRE(seen.get("c") == 3);

  CHECK_THROWS_AS(*it, RuntimeError);
  CHECK_THROWS_AS(*end, RuntimeError);
}

TEST_CASE("key iterator") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  List<String> seen;
  for (const String &key : map.keys()) {
    seen.push_back(key);
  }
  REQUIRE(seen.has("a"));
  REQUIRE(seen.has("b"));
  REQUIRE(seen.has("c"));

  const Map<String, int> &const_map = map;
  List<String> const_seen;
  for (const auto &key : const_map.keys()) {
    const_seen.push_back(key);
  }
  REQUIRE(const_seen.has("a"));
  REQUIRE(const_seen.has("b"));
  REQUIRE(const_seen.has("c"));
}

TEST_CASE("key iterator - manual") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  auto it = map.keys().begin();
  auto end = map.keys().end();
  List<String> seen;
  for (; it != end; ++it) {
    const String &key = *it;
    seen.push_back(key);
  }
  REQUIRE(seen.has("a"));
  REQUIRE(seen.has("b"));
  REQUIRE(seen.has("c"));

  CHECK_THROWS_AS(*it, RuntimeError);
  CHECK_THROWS_AS(*end, RuntimeError);
}

TEST_CASE("value iterator") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  List<int> seen;
  for (int &value : map.values()) {
    seen.push_back(value);
  }
  REQUIRE(seen.has(1));
  REQUIRE(seen.has(2));
  REQUIRE(seen.has(3));

  const Map<String, int> &const_map = map;
  List<int> const_seen;
  for (const int &value : const_map.values()) {
    const_seen.push_back(value);
  }
  REQUIRE(const_seen.has(1));
  REQUIRE(const_seen.has(2));
  REQUIRE(const_seen.has(3));
}

TEST_CASE("value iterator - manual") {
  auto map = Map<String, int>({{"a", 1}, {"b", 2}, {"c", 3}});
  auto it = map.values().begin();
  auto end = map.values().end();
  List<int> seen;
  for (; it != end; ++it) {
    const auto &value = *it;
    seen.push_back(value);
  }
  REQUIRE(seen.has(1));
  REQUIRE(seen.has(2));
  REQUIRE(seen.has(3));

  CHECK_THROWS_AS(*it, RuntimeError);
  CHECK_THROWS_AS(*end, RuntimeError);
}

TEST_CASE("stress") {
  Map<int, int> map;
  const int num_elements = 10000;
  for (int i = 0; i < num_elements; i++) {
    map.set(i, i * 10);
    REQUIRE(map.get(i) == i * 10);
    REQUIRE(map.has(i));
    REQUIRE(map.size() == static_cast<size_t>(i + 1));
  }
  for (int i = 0; i < num_elements; i++) {
    REQUIRE(map.has(i));
    REQUIRE(map.get(i) == i * 10);
  }
  for (int i = num_elements - 1; i >= 0; i--) {
    map.remove(i);
    REQUIRE(!map.has(i));
    REQUIRE(map.size() == static_cast<size_t>(i));
  }
  REQUIRE(map.size() == 0);
  for (int i = 0; i < num_elements; i++) {
    REQUIRE(!map.has(i));
  }
  for (int i = num_elements + 1; i <= num_elements * 2; i++) {
    REQUIRE(!map.has(i));
    map.set(i, i * 10);
    REQUIRE(map.get(i) == i * 10);
    REQUIRE(map.has(i));
    REQUIRE(map.size() == static_cast<size_t>(i - num_elements));
  }
  REQUIRE(map.size() == static_cast<size_t>(num_elements));
}

TEST_SUITE_END();
