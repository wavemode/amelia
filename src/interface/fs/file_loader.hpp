#pragma once

#include "prelude.hpp"

namespace amelia {

struct AbstractString;

struct IFileLoader {
  virtual void load_file(AbstractString &output, const AbstractString &file_path) = 0;
  virtual Option<RuntimeError> try_load_file(
      AbstractString &output, const AbstractString &file_path
  ) = 0;
};

} // namespace amelia
