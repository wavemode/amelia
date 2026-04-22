#include "FileLoader.h"

#include <fstream>
#include <sstream>
#include <string>

#include "data/core/Slice.h"
#include "data/core/String.h"

namespace amelia {

void FileLoader::load_file(const IString &file_path, IString &output) {
  std::ifstream file(file_path.c_str());
  if (!file) {
    throw std::runtime_error("Failed to open file: " + std::string(file_path.c_str()));
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string str = ss.str();
  output.assign(Text(Slice(str.c_str(), str.size())));
}

} // namespace amelia
