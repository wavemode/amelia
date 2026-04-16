#pragma once

#include "interface/core/IFileLoader.h"

namespace amelia {

class String;

class FileLoader : public IFileLoader {
public:
  String load_file(const String &file_path) override;
};

} // namespace amelia
