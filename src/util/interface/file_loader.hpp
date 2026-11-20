#pragma once

namespace amelia {

struct AbstractString;
template <typename T> class Option;
class RuntimeError;
struct FilePath;

struct IFileLoader {
  virtual void load_file(AbstractString &output,
                         const AbstractString &file_path) = 0;

  void load_file(AbstractString &output, const FilePath &file_path);

  virtual Option<RuntimeError>
  try_load_file(AbstractString &output, const AbstractString &file_path) = 0;

  Option<RuntimeError> try_load_file(AbstractString &output,
                                     const FilePath &file_path);
};

} // namespace amelia
