#include <filesystem>
#include <iostream>

#include "filesystem_walker.h"
#include "prelude.h"

namespace amelia {

void FilesystemWalker::walk(
    AbstractList<String> &output, Text root, bool regular_files_only, bool ignore_errors
) {
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(String(root).c_str(), ec), end; it != end;
       it.increment(ec)) {
    if (ec) {
      if (!ignore_errors) {
        String err("Error iterating directory: ");
        err.append(Text::from(ec.message()));
        throw std::runtime_error(err.c_str());
      }
      ec.clear();
      continue;
    }
    if (regular_files_only && !it->is_regular_file()) {
      continue;
    }
    output.push_back(String::from(it->path().string().c_str()));
  }
}

} // namespace amelia
