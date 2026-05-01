#pragma once

namespace amelia {

class AbstractString;

struct IFileLoader {
  virtual void load_file(AbstractString &output, const AbstractString &file_path) = 0;
};

} // namespace amelia
