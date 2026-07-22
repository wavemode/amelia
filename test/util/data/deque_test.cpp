#include <doctest.h>

#include "util/data/deque.hpp"
#include "util/data/list.hpp"
#include "util/data/pair.hpp"
#include "util/data/slice.hpp"

TEST_SUITE_BEGIN("Deque");

using namespace amelia;

TEST_CASE("indexing") {
  Deque<int> deque({5, 2, 9, 1, 5, 6});
  REQUIRE(deque[0] == 5);
  REQUIRE(deque[1] == 2);
  REQUIRE(deque[2] == 9);
  REQUIRE(deque[3] == 1);
  REQUIRE(deque[4] == 5);
  REQUIRE(deque[5] == 6);
  CHECK_THROWS_AS(deque[6], RuntimeError);
  CHECK_THROWS_AS(deque[100], RuntimeError);
}

TEST_CASE("push_back and size") {
  Deque<int> deque;
  REQUIRE(deque.size() == 0);
  deque.push_back(5);
  REQUIRE(deque.size() == 1);
  deque.push_back(2);
  REQUIRE(deque.size() == 2);
}

TEST_CASE("pop_back") {
  Deque<int> deque({5, 2});
  REQUIRE(deque.size() == 2);
  deque.pop_back();
  REQUIRE(deque.size() == 1);
  REQUIRE(deque[0] == 5);
  deque.pop_back();
  REQUIRE(deque.size() == 0);
  CHECK_THROWS_AS(deque.pop_back(), RuntimeError);
}

TEST_CASE("push_front") {
  Deque<int> deque;
  REQUIRE(deque.size() == 0);
  deque.push_front(5);
  REQUIRE(deque.size() == 1);
  REQUIRE(deque[0] == 5);
  deque.push_front(2);
  REQUIRE(deque.size() == 2);
  REQUIRE(deque[0] == 2);
  REQUIRE(deque[1] == 5);
}

TEST_CASE("pop_front") {
  Deque<int> deque({5, 2});
  REQUIRE(deque.size() == 2);
  deque.pop_front();
  REQUIRE(deque.size() == 1);
  REQUIRE(deque[0] == 2);
  deque.pop_front();
  REQUIRE(deque.size() == 0);
  CHECK_THROWS_AS(deque.pop_front(), RuntimeError);
}

TEST_CASE("emplace_back") {
  Deque<Pair<int, int>> deque;
  deque.emplace_back(1, 2);
  REQUIRE(deque[0].first == 1);
  REQUIRE(deque[0].second == 2);

  Pair<int, int> &elem2 = deque.emplace_back(3, 4);
  REQUIRE(deque[1].first == 3);
  REQUIRE(deque[1].second == 4);
  elem2.first = 5;
  elem2.second = 6;
  REQUIRE(deque[1].first == 5);
  REQUIRE(deque[1].second == 6);
}

TEST_CASE("emplace_front") {
  Deque<Pair<int, int>> deque;
  deque.emplace_front(1, 2);
  REQUIRE(deque[0].first == 1);
  REQUIRE(deque[0].second == 2);

  Pair<int, int> &elem2 = deque.emplace_front(3, 4);
  REQUIRE(deque[0].first == 3);
  REQUIRE(deque[0].second == 4);
  REQUIRE(deque[1].first == 1);
  REQUIRE(deque[1].second == 2);
  elem2.first = 5;
  elem2.second = 6;
  REQUIRE(deque[0].first == 5);
  REQUIRE(deque[0].second == 6);
}

TEST_CASE("copy constructor") {
  Deque<int> deque1({5, 2, 9});
  Deque<int> deque2 = deque1;
  REQUIRE(deque2.size() == 3);
  REQUIRE(deque2[0] == 5);
  REQUIRE(deque2[1] == 2);
  REQUIRE(deque2[2] == 9);
}

TEST_CASE("move constructor") {
  Deque<int> deque1({5, 2, 9});
  Deque<int> deque2 = move(deque1);
  REQUIRE(deque2.size() == 3);
  REQUIRE(deque2[0] == 5);
  REQUIRE(deque2[1] == 2);
  REQUIRE(deque2[2] == 9);
  REQUIRE(deque1.size() == 0);
}

TEST_CASE("copy assignment operator") {
  Deque<int> deque1({5, 2, 9});
  Deque<int> deque2;
  deque2 = deque1;
  REQUIRE(deque2.size() == 3);
  REQUIRE(deque2[0] == 5);
  REQUIRE(deque2[1] == 2);
  REQUIRE(deque2[2] == 9);
}

TEST_CASE("move assignment operator") {
  Deque<int> deque1({5, 2, 9});
  Deque<int> deque2;
  deque2 = move(deque1);
  REQUIRE(deque2.size() == 3);
  REQUIRE(deque2[0] == 5);
  REQUIRE(deque2[1] == 2);
  REQUIRE(deque2[2] == 9);
  REQUIRE(deque1.size() == 0);
}

TEST_CASE("equality operator") {
  Deque<int> deque1({5, 2, 9});
  Deque<int> deque2({5, 2, 9});
  Deque<int> deque3({5, 2});
  REQUIRE(deque1 == deque2);
  REQUIRE(deque1 != deque3);
}

TEST_CASE("iterator") {
  Deque<int> deque({5, 2, 9});
  auto it = deque.begin();
  REQUIRE(*it == 5);
  ++it;
  REQUIRE(*it == 2);
  ++it;
  REQUIRE(*it == 9);
  ++it;
  REQUIRE(it.at_end());
  REQUIRE(it == deque.end());
  CHECK_THROWS_AS(*it, RuntimeError);
}

TEST_CASE("const iterator") {
  const Deque<int> deque({5, 2, 9});
  auto it = deque.begin();
  REQUIRE(*it == 5);
  ++it;
  REQUIRE(*it == 2);
  ++it;
  REQUIRE(*it == 9);
  ++it;
  REQUIRE(it.at_end());
  REQUIRE(it == deque.end());
  CHECK_THROWS_AS(*it, RuntimeError);
}

TEST_CASE("pointers not invalidated by push_back or push_front") {
  Deque<int> deque({5, 2, 9});
  int *ptr1 = &deque[0];
  int *ptr2 = &deque[1];
  int *ptr3 = &deque[2];
  for (int i = 0; i < 10000; ++i) {
    deque.push_back(i);
  }
  REQUIRE(&deque[0] == ptr1);
  REQUIRE(&deque[1] == ptr2);
  REQUIRE(&deque[2] == ptr3);
  for (int i = 0; i < 10000; ++i) {
    deque.push_front(i);
  }
  REQUIRE(&deque[10000] == ptr1);
  REQUIRE(&deque[10001] == ptr2);
  REQUIRE(&deque[10002] == ptr3);
}

List<int> list_with(int i) {
  return List<int>({i});
}

TEST_CASE("stress") {
  Deque<List<int>> deque;
  for (int i = 0; i < 10000; ++i) {
    deque.push_back(list_with(i));
    deque.push_front(list_with(-i));
  }
  REQUIRE(deque.size() == 20000);
  for (int i = 0; i < 10000; ++i) {
    REQUIRE(deque[i] == list_with(i - 9999));
  }
  for (int i = 0; i < 10000; ++i) {
    REQUIRE(deque[10000 + i] == list_with(i));
  }
  for (int i = 0; i < 10000; ++i) {
    deque.pop_back();
    deque.pop_front();
  }
  REQUIRE(deque.size() == 0);
  for (int i = 0; i < 10000; ++i) {
    deque.push_back(list_with(i));
    deque.push_front(list_with(-i));
  }
  REQUIRE(deque.size() == 20000);
  for (int i = 0; i < 10000; ++i) {
    REQUIRE(deque[i] == list_with(i - 9999));
  }
  for (int i = 0; i < 10000; ++i) {
    REQUIRE(deque[10000 + i] == list_with(i));
  }
  for (int i = 0; i < 10000; ++i) {
    deque.pop_back();
    deque.pop_front();
  }
  REQUIRE(deque.size() == 0);
}

TEST_SUITE_END();
