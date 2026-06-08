#include <doctest.h>

#include "prelude.hpp"

#include "action/sema/load_module.hpp"
#include "data/source/source_location_error.hpp"
#include "effect/fs/file_loader.hpp"

TEST_SUITE_BEGIN("load_module");

using namespace amelia;

TEST_CASE("target module is in second directory in path") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/empty", "src/test/action/sema/basic"})
  };
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);
  REQUIRE(sema_result.module_ids.size() == 2);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
}

TEST_CASE("imported module is in second directory in path") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/a_alone", "src/test/action/sema/basic"}
  )};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);
  REQUIRE(sema_result.module_ids.size() == 2);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
}

TEST_CASE("target module does not exist") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/empty"})};
  FileLoader file_loader;
  SemaResult sema_result;
  CHECK_THROWS_AS(load_module(file_loader, sema_result, "a", ctx), RuntimeError);
}

TEST_CASE("imported module does not exist") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/a_alone"})};
  FileLoader file_loader;
  SemaResult sema_result;
  CHECK_THROWS_AS(load_module(file_loader, sema_result, "a", ctx), SourceLocationError);
}

TEST_CASE("a -> b") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/basic"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);
  REQUIRE(sema_result.module_ids.size() == 2);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_a_id));
  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.size() == 0);
}

TEST_CASE("a -> b/c") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/subdir"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);
  REQUIRE(sema_result.module_ids.size() == 2);

  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b::c"));

  // b::c is its own file within a subdirectory (b/c.am) rather than a submodule within b.am
  REQUIRE(!sema_result.module_ids.has("b"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_c_id = sema_result.module_ids.get("b::c");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_c_id].group_module_ids.size() == 0);
}

TEST_CASE("b/c -> a") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/subdir_rev"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "b::c", ctx);
  REQUIRE(sema_result.module_ids.size() == 2);

  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b::c"));

  // b::c is its own file within a subdirectory (b/c.am) rather than a submodule within b.am
  REQUIRE(!sema_result.module_ids.has("b"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_c_id = sema_result.module_ids.get("b::c");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 0);

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imported_by.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_c_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imports.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_c_id].group_module_ids.size() == 0);
}

TEST_CASE("a -> b::c") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/basic_sub"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 3);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
  REQUIRE(sema_result.module_ids.has("b::c"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_b_c_id = sema_result.module_ids.get("b::c");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_c_id));

  // c is simply a submodule of b, so they have the same module ID and metadata
  REQUIRE(sema_result.module_meta.size() == 2);
  REQUIRE(module_b_id == module_b_c_id);
  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.has(module_a_id));
}

TEST_CASE("b::c -> a") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/sub_rev"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "b::c", ctx);

  REQUIRE(sema_result.module_ids.size() == 3);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
  REQUIRE(sema_result.module_ids.has("b::c"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_b_c_id = sema_result.module_ids.get("b::c");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 0);

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imported_by.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_c_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imports.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_c_id].group_module_ids.size() == 0);
}

TEST_CASE("2 circular imports") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/two_circular"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 2);
  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_b_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].imports.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imported_by.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_b_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_a_id));
}

TEST_CASE("3 circular imports") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/three_circular"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 3);
  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_c_id = sema_result.module_ids.get("c");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_b_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].imports.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_c_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_c_id].imports.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imported_by.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_b_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_c_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_c_id].imported_by.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_a_id));
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.has(module_a_id));
  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.has(module_b_id));
}

TEST_CASE("a -> b::c -> a") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/sub_circular"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 3);
  REQUIRE(sema_result.module_meta.size() == 2);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
  REQUIRE(sema_result.module_ids.has("b::c"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_b_c_id = sema_result.module_ids.get("b::c");
  REQUIRE(module_b_id == module_b_c_id);

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imports.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imported_by.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].imported_by.has(module_a_id));

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.has(module_b_c_id));

  REQUIRE(sema_result.module_meta[module_b_c_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_c_id].group_module_ids.has(module_a_id));
}

TEST_CASE("a -> (b <-> c) -> d") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/circle_in_chain"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 4);
  REQUIRE(sema_result.module_meta.size() == 4);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
  REQUIRE(sema_result.module_ids.has("c"));
  REQUIRE(sema_result.module_ids.has("d"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_c_id = sema_result.module_ids.get("c");
  ModuleId module_d_id = sema_result.module_ids.get("d");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 0);

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].imports.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_b_id].imported_by.size() == 2);
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_a_id));
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_c_id].imports.size() == 2);
  REQUIRE(sema_result.module_meta[module_c_id].imports.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_c_id].imports.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_c_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_c_id].imported_by.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.size() == 1);
  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_d_id].imports.size() == 0);

  REQUIRE(sema_result.module_meta[module_d_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_d_id].imported_by.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_d_id].group_module_ids.size() == 0);
}

TEST_CASE("a -> (b <-> c <-> d) -> e") {
  ModuleLoaderContext ctx{Set<String>({"src/test/action/sema/three_circle_in_chain"})};
  FileLoader file_loader;
  SemaResult sema_result;
  load_module(file_loader, sema_result, "a", ctx);

  REQUIRE(sema_result.module_ids.size() == 5);
  REQUIRE(sema_result.module_meta.size() == 5);
  REQUIRE(sema_result.module_ids.has("a"));
  REQUIRE(sema_result.module_ids.has("b"));
  REQUIRE(sema_result.module_ids.has("c"));
  REQUIRE(sema_result.module_ids.has("d"));
  REQUIRE(sema_result.module_ids.has("e"));

  ModuleId module_a_id = sema_result.module_ids.get("a");
  ModuleId module_b_id = sema_result.module_ids.get("b");
  ModuleId module_c_id = sema_result.module_ids.get("c");
  ModuleId module_d_id = sema_result.module_ids.get("d");
  ModuleId module_e_id = sema_result.module_ids.get("e");

  REQUIRE(sema_result.module_meta[module_a_id].imports.size() == 1);
  REQUIRE(sema_result.module_meta[module_a_id].imports.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_a_id].imported_by.size() == 0);

  REQUIRE(sema_result.module_meta[module_a_id].group_module_ids.size() == 0);

  REQUIRE(sema_result.module_meta[module_b_id].imports.size() == 2);
  REQUIRE(sema_result.module_meta[module_b_id].imports.has(module_c_id));
  REQUIRE(sema_result.module_meta[module_b_id].imports.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_b_id].imported_by.size() == 3);
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_a_id));
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_c_id));
  REQUIRE(sema_result.module_meta[module_b_id].imported_by.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_c_id));
  REQUIRE(sema_result.module_meta[module_b_id].group_module_ids.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_c_id].imports.size() == 2);
  REQUIRE(sema_result.module_meta[module_c_id].imports.has(module_d_id));
  REQUIRE(sema_result.module_meta[module_c_id].imports.has(module_b_id));

  REQUIRE(sema_result.module_meta[module_c_id].imported_by.size() == 2);
  REQUIRE(sema_result.module_meta[module_c_id].imported_by.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_c_id].imported_by.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_c_id].group_module_ids.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_d_id].imports.size() == 3);
  REQUIRE(sema_result.module_meta[module_d_id].imports.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_d_id].imports.has(module_c_id));
  REQUIRE(sema_result.module_meta[module_d_id].imports.has(module_e_id));

  REQUIRE(sema_result.module_meta[module_d_id].imported_by.size() == 2);
  REQUIRE(sema_result.module_meta[module_d_id].imported_by.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_d_id].imported_by.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_d_id].group_module_ids.size() == 2);
  REQUIRE(sema_result.module_meta[module_d_id].group_module_ids.has(module_b_id));
  REQUIRE(sema_result.module_meta[module_d_id].group_module_ids.has(module_c_id));

  REQUIRE(sema_result.module_meta[module_e_id].imports.size() == 0);

  REQUIRE(sema_result.module_meta[module_e_id].imported_by.size() == 1);
  REQUIRE(sema_result.module_meta[module_e_id].imported_by.has(module_d_id));

  REQUIRE(sema_result.module_meta[module_e_id].group_module_ids.size() == 0);
}

TEST_SUITE_END();
