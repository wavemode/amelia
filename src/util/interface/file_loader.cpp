#include "file_loader.hpp"

#include "util/data/file_path.hpp"
#include "util/data/option.hpp"
#include "util/data/runtime_error.hpp"
#include "util/data/string.hpp"

namespace amelia {

void IFileLoader::load_file(AbstractString &output, const FilePath &file_path) {
  String path_str;
  file_path.to_string(path_str);
  load_file(output, path_str);
}

Option<RuntimeError>
IFileLoader::try_load_file(AbstractString &output, const FilePath &file_path) {
  String path_str;
  file_path.to_string(path_str);
  return try_load_file(output, path_str);
}

} // namespace amelia
