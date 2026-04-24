#pragma once

#include <stdexcept>

namespace amelia {

/**
 * @class InvalidUTF8Error
 * @brief Exception thrown when a String is constructed or appended with invalid
 * UTF-8 data.
 */
class InvalidUTF8Error : public std::exception {};

} // namespace amelia
