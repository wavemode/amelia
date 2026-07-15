#pragma once

#include <cstdint>

namespace amelia {

class Serialize;

enum class BindingKind : uint8_t { Variable, Constant, Function, Type, Class, Concept, Module };

Serialize serialize_binding_kind(BindingKind kind);

} // namespace amelia
