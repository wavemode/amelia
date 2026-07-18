#pragma once

#include <cstdint>

#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct FunctionSignature;
struct Binding;

using NodeId = int32_t;

struct ModuleAnalysisContext {
  Option<FunctionSignature *> current_function_signature;
  Option<Binding *> binding_currently_analyzing;
  Option<NodeId> intro_decls_currently_analyzing;
};

};