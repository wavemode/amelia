#include "file_loader.h"
#include "prelude.h"

#include <fstream>
#include <sstream>
#include <string>

namespace amelia {

void FileLoader::load_file(IString &output, const IString &file_path) {
  std::ifstream file(file_path.c_str());
  if (!file) {
    String err("Failed to open file: ");
    err.append(file_path.text());
    throw RuntimeError(std::move(err));
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  output.assign(Text::from(ss.str()));
}

} // namespace amelia
