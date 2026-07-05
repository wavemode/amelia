#pragma once

#include "fs/interface/file_writer.hpp"

namespace amelia {

class FileWriter : public IFileWriter {
public:
  void write_file(const AbstractString &file_path, const AbstractString &contents) override;
  void append_file(const AbstractString &file_path, const AbstractString &contents) override;

private:
};

} // namespace amelia
