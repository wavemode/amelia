#pragma once

#include "data/core/List.h"
#include <cstddef>

#include "data/core/List.h"
#include "interface/source/ISourceFileManager.h"

namespace amelia {

class String;

class SourceFileManager : public ISourceFileManager {
public:
  FileId store_source_file(String) override;
  const String &get_source_file(FileId) override;

private:
  List<String> source_files;
};

} // namespace amelia
