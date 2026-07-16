#include "type_error.hpp"

#include "source/data/location.hpp"
#include "util/data/string.hpp"

namespace amelia {

TypeError::TypeError(Location loc, String message) noexcept
    : SourceLocationError(loc, move(message)) {}

} // namespace amelia
