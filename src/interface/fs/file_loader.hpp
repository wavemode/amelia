#pragma once

namespace amelia {

struct AbstractString;

struct IFileLoader {
  virtual void load_file(AbstractString &output, const AbstractString &file_path) = 0;
};

} // namespace amelia
