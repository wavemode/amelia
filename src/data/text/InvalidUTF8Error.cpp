#include "InvalidUTF8Error.h"

const char *amelia::InvalidUTF8Error::what() const noexcept { return "Invalid UTF-8 data"; }
