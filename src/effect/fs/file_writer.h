#pragma once

#include "interface/fs/file_writer.h"

namespace amelia {

class FileWriter : public IFileWriter {
public:
  void write_file(const IString &file_path, const IString &contents) override;
  void append_file(const IString &file_path, const IString &contents) override;

private:
};

} // namespace amelia
