#include <filesystem>
#include <fstream>
#include <iostream>

#include "file_writer.hpp"

#include "prelude.hpp"

namespace amelia {

namespace {

void write_to_file(
    const AbstractString &path,
    const AbstractString &content,
    bool overwrite,
    bool create_directories
) {
  std::filesystem::path p(path.c_str());

  if (create_directories) {
    std::filesystem::path dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }
  }

  std::ofstream outfile(
      p, overwrite ? (std::ios::out | std::ios::trunc) : (std::ios::out | std::ios::app)
  );

  if (outfile.is_open()) {
    outfile << content.c_str();
    outfile.close();
  }
}

} // namespace

void FileWriter::write_file(const AbstractString &file_path, const AbstractString &contents) {
  write_to_file(file_path, contents, true, true);
}

void FileWriter::append_file(const AbstractString &file_path, const AbstractString &contents) {
  write_to_file(file_path, contents, false, true);
}

} // namespace amelia
