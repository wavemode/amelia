#pragma once

namespace amelia {

struct AbstractString;

struct IFileWriter {
  virtual void write_file(const AbstractString &file_path, const AbstractString &contents) = 0;
  virtual void append_file(const AbstractString &file_path, const AbstractString &contents) = 0;
};

} // namespace amelia
