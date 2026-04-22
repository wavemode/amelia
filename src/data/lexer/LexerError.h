#pragma once

#include <stdexcept>

#include "data/core/String.h"
#include "data/source/Location.h"

namespace amelia {

/**
 * @class LexerError
 * @brief Exception thrown by the lexer when it encounters an error.
 */
struct LexerError : public std::exception {
  const Location loc;
  const String message;

  LexerError(Location loc, String message) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
