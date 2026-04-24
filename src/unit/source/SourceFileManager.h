#pragma once

#include <cstddef>

#include "interface/source/ISourceFileManager.h"

#include "data/core/List.h"
#include "data/text/String.h"

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
