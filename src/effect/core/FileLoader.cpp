#include "FileLoader.h"

#include <fstream>
#include <sstream>
#include <string>

#include "util/slice/Slice.h"
#include "util/text/String.h"

namespace amelia {

String FileLoader::load_file(const String &file_path) {
  std::ifstream file(file_path.c_str());
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string str = ss.str();
  return String(Slice(str.c_str(), str.size()));
}

} // namespace amelia
