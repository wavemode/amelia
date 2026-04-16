#pragma once

#include <cstddef>
#include <vector>

#include "interface/source/ISourceFileManager.h"

namespace amelia {

class String;

class SourceFileManager : public ISourceFileManager {
public:
  file_id store_source_file(String) override;
  const String &get_source_file(file_id) override;

private:
  std::vector<String> source_files;
};

} // namespace amelia
