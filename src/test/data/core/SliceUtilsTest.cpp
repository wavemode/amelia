#include <doctest.h>

#include "data/core/SliceUtils.h"

TEST_SUITE_BEGIN("SliceUtils");

using namespace amelia;

TEST_CASE("sort - default comparison") {
  List<int> list = {5, 2, 9, 1, 5, 6};
  SliceUtils::sort(list);
  CHECK(list == List({1, 2, 5, 5, 6, 9}));
}

TEST_CASE("sort - custom comparison") {
  List<int> list = {5, 2, 9, 1, 5, 6};
  SliceUtils::sort(list, [](int a, int b) { return a > b; });
  CHECK(list == List({9, 6, 5, 5, 2, 1}));
}

TEST_CASE("reverse - List<T>") {
  List<int> list = {1, 2, 3, 4, 5};
  SliceUtils::reverse(list);
  CHECK(list == List({5, 4, 3, 2, 1}));
}

TEST_CASE("reverse - Slice<T>") {
  List<int> list = {1, 2, 3, 4, 5};
  SliceUtils::reverse(Slice<int>(list));
  CHECK(list == List({5, 4, 3, 2, 1}));
}

TEST_SUITE_END();
