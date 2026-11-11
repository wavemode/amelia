#pragma once

#include "util/data/runtime_error.hpp"

namespace amelia {

/**
 * @class InvalidUTF16Error
 * @brief Exception thrown when a String is constructed or appended with invalid
 * UTF-16 data.
 */
struct InvalidUTF16Error : public RuntimeError {
  InvalidUTF16Error();
};

} // namespace amelia
