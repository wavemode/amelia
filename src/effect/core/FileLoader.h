#pragma once

#include "interface/core/IFileLoader.h"

namespace amelia {

class String;
class Text;

class FileLoader : public IFileLoader {
public:
  String load_file(const Text &file_path) override;
};

} // namespace amelia
