#pragma once

#include "interface/core/IFileLoader.h"

namespace amelia {

class FileLoader : public IFileLoader {
public:
  void load_file(const IString &file_path, IString &output) override;
};

} // namespace amelia
