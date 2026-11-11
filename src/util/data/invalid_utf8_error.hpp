#pragma once

#include "util/data/runtime_error.hpp"

namespace amelia {

/**
 * @class InvalidUTF8Error
 * @brief Exception thrown when a String is constructed or appended with invalid
 * UTF-8 data.
 */
struct InvalidUTF8Error : public RuntimeError {
  InvalidUTF8Error();
};

} // namespace amelia
