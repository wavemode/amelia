#include <doctest.h>

#include "unit/source/SourceFileManager.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("SourceFileManager");

using namespace amelia;

TEST_CASE("can store and retrieve source file") {
  SourceFileManager manager;
  String source("// hello");
  FileId id = manager.store_source_file(source);
  const String &retrieved = manager.get_source_file(id);
  CHECK(source == retrieved);
}

TEST_SUITE_END();
