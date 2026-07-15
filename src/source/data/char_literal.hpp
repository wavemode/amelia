#pragma once

#include <cstdint>

namespace amelia {

class Serialize;

Serialize serialize_char_literal(uint32_t code_point, bool quoted = true);

} // namespace amelia