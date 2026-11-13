#pragma once

#include <cstdint>

namespace amelia {

struct AbstractString;
class ParserResult;
using NodeId = int32_t;

void get_identifier_text(AbstractString &output, const ParserResult &ast,
                         NodeId node_id);

} // namespace amelia
