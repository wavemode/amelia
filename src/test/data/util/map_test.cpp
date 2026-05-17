#include <doctest.h>

#include "prelude.h"

TEST_SUITE_BEGIN("Map");

using namespace amelia;

TEST_CASE("get, set") {
  Map<String, int> map;
  CHECK(map.size() == 0);
  map.set("a", 1);
  CHECK(map.size() == 1);
  map.set("b", 2);
  CHECK(map.size() == 2);
  CHECK(map.get("a") == 1);
  CHECK(map.get("b") == 2);

  const Map<String, int> &const_map = map;
  CHECK(const_map.get("a") == 1);
  CHECK(const_map.get("b") == 2);
}

TEST_CASE("has and clear") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  CHECK(map.has("a"));
  CHECK(map.has("b"));
  CHECK(map.has("c"));

  map.clear();
  CHECK(!map.has("a"));
  CHECK(!map.has("b"));
  CHECK(!map.has("c"));
}

TEST_CASE("indexing operator") {
  Map<String, int> map;
  map.set("a", 1);
  map.set("b", 2);
  CHECK(map["a"] == 1);
  CHECK(map["b"] == 2);

  const Map<String, int> &const_map = map;
  CHECK(const_map["a"] == 1);
  CHECK(const_map["b"] == 2);

  CHECK_THROWS_AS(map["c"], std::runtime_error);
  CHECK_THROWS_AS(const_map["c"], std::runtime_error);
}

TEST_CASE("find") {
  Map<String, int> map;
  map.set("a", 1);
  auto a_value = map.find("a");
  CHECK(a_value.has_value());
  CHECK(**a_value == 1);
  **a_value = 10;
  CHECK(map.get("a") == 10);
  CHECK(map.find("b") == None());

  const Map<String, int> &const_map = map;
  auto const_a_value = const_map.find("a");
  CHECK(const_a_value.has_value());
  CHECK(**const_a_value == 10);
  CHECK(const_map.find("b") == None());
}

TEST_CASE("remove and remove_and_get") {
  Map<String, int> map;
  map.set("a", 1);
  map.set("b", 2);
  CHECK(map.size() == 2);
  map.remove("a");
  CHECK(map.size() == 1);
  CHECK(!map.has("a"));
  CHECK(map.has("b"));

  map.remove("c"); // Removing a non-existent key should do nothing
  CHECK(!map.has("a"));
  CHECK(map.has("b"));

  int b_value = map.remove_and_get("b");
  CHECK(b_value == 2);
  CHECK(!map.has("b"));

  CHECK_THROWS_AS(map.remove_and_get("c"), std::runtime_error);
}

TEST_CASE("equals") {
  Map<String, int> map1 = {{"a", 1}, {"b", 2}, {"c", 3}};
  Map<String, int> map2 = {{"a", 1}, {"b", 2}, {"c", 3}};
  Map<String, int> map3 = {{"a", 1}, {"b", 2}};
  Map<String, int> map4 = {{"a", 1}, {"b", 2}, {"c", 4}};

  CHECK(map1 == map2);
  CHECK(map1 != map3);
  CHECK(map1 != map4);
  CHECK(map2 != map3);
  CHECK(map2 != map4);
  CHECK(map3 != map4);

  map4.remove("c");
  CHECK(map3 == map4);

  map3.set("b", 20);
  CHECK(map3 != map4);
  map3.set("b", 2);
  CHECK(map3 == map4);

  map3.set("c", 3);
  CHECK(map1 == map3);
  CHECK(map3 != map4);
}

TEST_CASE("pair iterator") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  Map<String, int> seen;
  for (auto &[key, value] : map) {
    seen.set(key, value);
  }
  CHECK(seen.get("a") == 1);
  CHECK(seen.get("b") == 2);
  CHECK(seen.get("c") == 3);

  const Map<String, int> &const_map = map;
  Map<String, int> const_seen;
  for (const auto &[key, value] : const_map) {
    const_seen.set(key, value);
  }
  CHECK(const_seen.get("a") == 1);
  CHECK(const_seen.get("b") == 2);
  CHECK(const_seen.get("c") == 3);
}

TEST_CASE("pair iterator - manual") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  auto it = map.begin();
  auto end = map.end();
  Map<String, int> seen;
  for (; it != end; ++it) {
    auto &[key, value] = *it;
    seen.set(key, value);
  }
  CHECK(seen.get("a") == 1);
  CHECK(seen.get("b") == 2);
  CHECK(seen.get("c") == 3);

  CHECK_THROWS_AS(*it, std::runtime_error);
  CHECK_THROWS_AS(*end, std::runtime_error);
}

TEST_CASE("key iterator") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  List<String> seen;
  for (const String &key : map.keys()) {
    seen.push_back(key);
  }
  CHECK(seen.has("a"));
  CHECK(seen.has("b"));
  CHECK(seen.has("c"));

  const Map<String, int> &const_map = map;
  List<String> const_seen;
  for (const auto &key : const_map.keys()) {
    const_seen.push_back(key);
  }
  CHECK(const_seen.has("a"));
  CHECK(const_seen.has("b"));
  CHECK(const_seen.has("c"));
}

TEST_CASE("key iterator - manual") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  auto it = map.keys().begin();
  auto end = map.keys().end();
  List<String> seen;
  for (; it != end; ++it) {
    const String &key = *it;
    seen.push_back(key);
  }
  CHECK(seen.has("a"));
  CHECK(seen.has("b"));
  CHECK(seen.has("c"));

  CHECK_THROWS_AS(*it, std::runtime_error);
  CHECK_THROWS_AS(*end, std::runtime_error);
}

TEST_CASE("value iterator") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  List<int> seen;
  for (int &value : map.values()) {
    seen.push_back(value);
  }
  CHECK(seen.has(1));
  CHECK(seen.has(2));
  CHECK(seen.has(3));

  const Map<String, int> &const_map = map;
  List<int> const_seen;
  for (const int &value : const_map.values()) {
    const_seen.push_back(value);
  }
  CHECK(const_seen.has(1));
  CHECK(const_seen.has(2));
  CHECK(const_seen.has(3));
}

TEST_CASE("value iterator - manual") {
  Map<String, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
  auto it = map.values().begin();
  auto end = map.values().end();
  List<int> seen;
  for (; it != end; ++it) {
    const auto &value = *it;
    seen.push_back(value);
  }
  CHECK(seen.has(1));
  CHECK(seen.has(2));
  CHECK(seen.has(3));

  CHECK_THROWS_AS(*it, std::runtime_error);
  CHECK_THROWS_AS(*end, std::runtime_error);
}

TEST_SUITE_END();
