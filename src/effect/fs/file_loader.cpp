#include <fstream>
#include <sstream>
#include <string>

#include "file_loader.hpp"

namespace amelia {

void FileLoader::load_file(AbstractString &output, const AbstractString &file_path) {
  std::ifstream file(file_path.c_str());
  if (!file) {
    String err("Failed to open file: ");
    err.append(file_path.text());
    throw RuntimeError(err.c_str());
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  output.assign(Text::from(ss.str().c_str()));
}

Option<RuntimeError> FileLoader::try_load_file(
    AbstractString &output, const AbstractString &file_path
) {
  std::ifstream file(file_path.c_str());
  if (!file) {
    String err("Failed to open file: ");
    err.append(file_path.text());
    return Some(RuntimeError(err.c_str()));
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  output.assign(Text::from(ss.str().c_str()));
  return None();
}

} // namespace amelia
