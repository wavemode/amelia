#include "source_location_error.h"
#include "prelude.h"

#include "data/util/text_utils.h"

namespace amelia {

namespace {
String make_message(Location loc, String message) {
  message.append(" (at ");
  message.append(loc.filename);
  message.append(':');
  TextUtils::to_string(message, loc.line);
  message.append(':');
  TextUtils::to_string(message, loc.column);
  message.append(')');
  return message;
}
} // namespace

SourceLocationError::SourceLocationError(Location loc, String message)
    : message(make_message(loc, std::move(message))) {}

const char *SourceLocationError::what() const noexcept { return message.c_str(); }

} // namespace amelia
