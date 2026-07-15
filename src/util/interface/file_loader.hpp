#pragma once

namespace amelia {

struct AbstractString;
template <typename T> class Option;
class RuntimeError;

struct IFileLoader {
  virtual void load_file(AbstractString &output, const AbstractString &file_path) = 0;
  virtual Option<RuntimeError> try_load_file(
      AbstractString &output, const AbstractString &file_path
  ) = 0;
};

} // namespace amelia
