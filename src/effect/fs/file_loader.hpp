#pragma once

#include "interface/fs/file_loader.hpp"

namespace amelia {

class FileLoader : public IFileLoader {
public:
  void load_file(AbstractString &output, const AbstractString &file_path) override;
  Option<RuntimeError> try_load_file(
      AbstractString &output, const AbstractString &file_path
  ) override;
};

} // namespace amelia
