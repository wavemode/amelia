#pragma once

#include "prelude.hpp"

namespace amelia {

Serialize serialize_char_literal(uint32_t code_point, bool quoted = true);

}