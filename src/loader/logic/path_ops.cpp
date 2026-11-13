#include "path_ops.hpp"

#include "util/data/abstract_string.hpp"
#include "util/data/list.hpp"
#include "util/data/slice.hpp"
#include "util/data/text.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

void append_paths(AbstractString &output, ConstSlice<Text> paths) {
  for (size_t i = 0; i < paths.size(); ++i) {
    TextUtils::trim_right_into(output, paths[i], "/");
    if (i < paths.size() - 1) {
      output.append('/');
    }
  }
}

Text directory_name(Text path) {
  path = TextUtils::trim_right(path, "/");
  List<size_t> slash_positions;
  TextUtils::find_all_char_bytes(slash_positions, path, '/');
  if (slash_positions.size() == 0) {
    return "/";
  }
  return TextUtils::trim_right(
      TextUtils::head_bytes(path, slash_positions[slash_positions.size() - 1]),
      "/");
}

} // namespace amelia
