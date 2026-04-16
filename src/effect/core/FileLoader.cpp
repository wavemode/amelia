#include "FileLoader.h"

#include <fstream>
#include <sstream>
#include <string>

#include "util/slice/Slice.h"
#include "util/text/String.h"
#include "util/text/Text.h"

namespace amelia {

String FileLoader::load_file(const Text &file_path) {
  std::ifstream file(std::string(file_path.data().ptr(), file_path.data().size()));
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string str = ss.str();
  return String(Slice(str.c_str(), str.size()));
}

} // namespace amelia
