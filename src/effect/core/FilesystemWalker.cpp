#include <filesystem>
#include <iostream>

#include "FilesystemWalker.h"
#include "data/core/List.h"
#include "data/core/String.h"

namespace amelia {

void FilesystemWalker::walk(
    const String &root, List<String> &output, bool regular_files_only, bool ignore_errors
) {
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(root.c_str(), ec), end; it != end;
       it.increment(ec)) {
    if (ec) {
      if (!ignore_errors) {
        throw std::filesystem::filesystem_error("Error iterating directory", ec);
      }
      ec.clear();
      continue;
    }
    if (regular_files_only && !it->is_regular_file()) {
      continue;
    }
    output.push_back(String::from(it->path().string()));
  }
}

} // namespace amelia
