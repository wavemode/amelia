#pragma once

#include <cstddef>
#include <vector>

#include "interface/source/ISourceFileManager.h"

namespace amelia {

class String;

class SourceFileManager : public ISourceFileManager {
public:
  FileId store_source_file(String) override;
  const String &get_source_file(FileId) override;

private:
  std::vector<String> source_files;
};

} // namespace amelia
