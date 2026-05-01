#pragma once

#include <stdexcept>

#include "data/core/string.h"
#include "data/source/location.h"

namespace amelia {

/**
 * @class LexerError
 * @brief Exception thrown by the lexer when it encounters an error.
 */
struct LexerError : public std::exception {
  const String message;

  LexerError(Location loc, String message) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
