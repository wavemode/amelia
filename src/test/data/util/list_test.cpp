#include <doctest.h>
#include <initializer_list>

#include "prelude.hpp"

TEST_SUITE_BEGIN("List");

using namespace amelia;

TEST_CASE("indexing") {
  List<int> list({5, 2, 9, 1, 5, 6});
  CHECK(list[0] == 5);
  CHECK(list[1] == 2);
  CHECK(list[2] == 9);
  CHECK(list[3] == 1);
  CHECK(list[4] == 5);
  CHECK(list[5] == 6);
  CHECK_THROWS_AS(list[6], RuntimeError);
  CHECK_THROWS_AS(list[100], RuntimeError);
}

TEST_CASE("push_back and size") {
  List<int> list;
  CHECK(list.size() == 0);
  list.push_back(5);
  CHECK(list.size() == 1);
  list.push_back(2);
  CHECK(list.size() == 2);
}

TEST_CASE("emplace_back") {
  List<std::pair<int, int>> list;
  list.emplace_back(1, 2);
  CHECK(list[0].first == 1);
  CHECK(list[0].second == 2);

  std::pair<int, int> &elem2 = list.emplace_back(3, 4);
  CHECK(list[1].first == 3);
  CHECK(list[1].second == 4);
  elem2.first = 5;
  elem2.second = 6;
  CHECK(list[1].first == 5);
  CHECK(list[1].second == 6);
}

TEST_SUITE_END();
