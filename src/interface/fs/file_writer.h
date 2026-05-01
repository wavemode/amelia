#pragma once

namespace amelia {

class IString;

struct IFileWriter {
  virtual void write_file(const IString &file_path, const IString &contents) = 0;
  virtual void append_file(const IString &file_path, const IString &contents) = 0;
};

} // namespace amelia
