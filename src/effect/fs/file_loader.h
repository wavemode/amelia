#pragma once

#include "interface/fs/file_loader.h"

namespace amelia {

class FileLoader : public IFileLoader {
public:
  void load_file(AbstractString &output, const AbstractString &file_path) override;
};

} // namespace amelia
