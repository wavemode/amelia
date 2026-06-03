#pragma once

#include <cstdint>
#include <cstddef>

#include "data/util/abstract_list.hpp"
#include "data/util/abstract_string.hpp"
#include "data/util/char_iterator.hpp"
#include "data/util/invalid_utf8_error.hpp"
#include "data/util/runtime_error.hpp"
#include "data/util/list.hpp"
#include "data/util/map.hpp"
#include "data/util/option.hpp"
#include "data/util/ref.hpp"
#include "data/util/set.hpp"
#include "data/util/slice.hpp"
#include "data/util/string.hpp"
#include "data/util/text.hpp"

namespace amelia {
  using ModuleId = int32_t;
  using TypeId = int32_t;
  using NodeId = int32_t;
  using TokenId = int32_t;
}
