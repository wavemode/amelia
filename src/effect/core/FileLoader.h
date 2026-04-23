#pragma once

#include "interface/core/IFileLoader.h"

namespace amelia {

class FileLoader : public IFileLoader {
public:
  void load_file(IString &output, const IString &file_path) override;
};

} // namespace amelia
