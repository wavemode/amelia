#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct ConceptType : Type {
  ConceptType();
  // TODO
};

bool is_concept_type(const Type &type);

} // namespace amelia
