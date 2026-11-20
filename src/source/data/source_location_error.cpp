#include "source_location_error.hpp"

#include "source/data/location.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

namespace {

String make_message(const Location &loc, String &&message) {
  message.append(" (at ");
  loc.filename->to_string(message);
  message.append(':');
  TextUtils::to_string(message, loc.line);
  message.append(':');
  TextUtils::to_string(message, loc.column);
  message.append(')');
  return message;
}
} // namespace

SourceLocationError::SourceLocationError(const Location &loc, String &&message)
    : RuntimeError(make_message(loc, move(message))), location(loc) {}

} // namespace amelia
