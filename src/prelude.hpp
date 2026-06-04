#pragma once

#include <cstddef>
#include <cstdint>

#include "data/util/abstract_list.hpp"
#include "data/util/abstract_string.hpp"
#include "data/util/char_iterator.hpp"
#include "data/util/invalid_utf8_error.hpp"
#include "data/util/list.hpp"
#include "data/util/option.hpp"
#include "data/util/pair.hpp"
#include "data/util/ref.hpp"
#include "data/util/runtime_error.hpp"
#include "data/util/slice.hpp"
#include "data/util/string.hpp"
#include "data/util/text.hpp"
#include "data/util/utility.hpp"

namespace amelia {
using ModuleId = int32_t;
using TypeId = int32_t;
using NodeId = int32_t;
using TokenId = int32_t;
} // namespace amelia
