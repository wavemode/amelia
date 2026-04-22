#include "SourceFileManager.h"

#include "data/core/String.h"

namespace amelia {

FileId SourceFileManager::store_source_file(String src) {
  source_files.push_back(src);
  return source_files.size() - 1;
}

const String &SourceFileManager::get_source_file(FileId id) { return source_files[id]; }

} // namespace amelia
