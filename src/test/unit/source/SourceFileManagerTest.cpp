#include <doctest.h>

#include "unit/source/SourceFileManager.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("SourceFileManager");

TEST_CASE("can store and retrieve source file") {
  amelia::SourceFileManager manager;
  amelia::String source = "// hello";
  amelia::file_id id = manager.store_source_file(source);
  const amelia::String &retrieved = manager.get_source_file(id);
  CHECK(source == retrieved);
}

TEST_SUITE_END();
