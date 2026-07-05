#pragma once

#include <cstddef>
#include <cstdint>

#include "util/data/abstract_list.hpp"
#include "util/data/abstract_string.hpp"
#include "util/data/box.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/deque.hpp"
#include "util/data/invalid_utf8_error.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"
#include "util/data/pair.hpp"
#include "util/data/ref.hpp"
#include "util/data/runtime_error.hpp"
#include "util/data/slice.hpp"
#include "util/data/string.hpp"
#include "util/data/text.hpp"
#include "util/data/utility.hpp"

#include "testing/data/serialize.hpp"

#include "sys/effect/console_printer.hpp"

namespace amelia {
using ModuleId = int32_t;
using BindingId = int32_t;
using NodeId = int32_t;
using TokenId = int32_t;
} // namespace amelia
