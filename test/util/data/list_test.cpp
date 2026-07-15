#include <doctest.h>

#include "util/data/list.hpp"
#include "util/data/pair.hpp"
#include "util/data/slice.hpp"

TEST_SUITE_BEGIN("List");

using namespace amelia;

TEST_CASE("indexing") {
  List<int> list({5, 2, 9, 1, 5, 6});
  REQUIRE(list[0] == 5);
  REQUIRE(list[1] == 2);
  REQUIRE(list[2] == 9);
  REQUIRE(list[3] == 1);
  REQUIRE(list[4] == 5);
  REQUIRE(list[5] == 6);
  CHECK_THROWS_AS(list[6], RuntimeError);
  CHECK_THROWS_AS(list[100], RuntimeError);
}

TEST_CASE("push_back and size") {
  List<int> list;
  REQUIRE(list.size() == 0);
  list.push_back(5);
  REQUIRE(list.size() == 1);
  list.push_back(2);
  REQUIRE(list.size() == 2);
}

TEST_CASE("emplace_back") {
  List<Pair<int, int>> list;
  list.emplace_back(1, 2);
  REQUIRE(list[0].first == 1);
  REQUIRE(list[0].second == 2);

  Pair<int, int> &elem2 = list.emplace_back(3, 4);
  REQUIRE(list[1].first == 3);
  REQUIRE(list[1].second == 4);
  elem2.first = 5;
  elem2.second = 6;
  REQUIRE(list[1].first == 5);
  REQUIRE(list[1].second == 6);
}

TEST_SUITE_END();
