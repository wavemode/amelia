#include "SourceFileManager.h"

#include "util/text/String.h"

namespace amelia {

file_id SourceFileManager::store_source_file(String src) {
  source_files.push_back(src);
  return source_files.size() - 1;
}

const String &SourceFileManager::get_source_file(file_id id) { return source_files[id]; }

} // namespace amelia
