#include "invalid_utf8_error.h"

const char *amelia::InvalidUTF8Error::what() const noexcept { return "Invalid UTF-8 data"; }
