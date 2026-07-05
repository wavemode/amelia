#pragma once

#include <exception>

namespace amelia {

/**
 * @class InvalidUTF8Error
 * @brief Exception thrown when a String is constructed or appended with invalid
 * UTF-8 data.
 */
struct InvalidUTF8Error : public std::exception {
  const char *what() const noexcept override;
};

} // namespace amelia
