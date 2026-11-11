#include "invalid_utf8_error.hpp"

namespace amelia {

InvalidUTF8Error::InvalidUTF8Error() : RuntimeError("Invalid UTF-8 data") {}

} // namespace amelia
