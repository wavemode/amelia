#include <doctest.h>

#include "loader/data/loader_context.hpp"
#include "loader/data/loader_result.hpp"
#include "loader/logic/loader.hpp"
#include "source/data/source_location_error.hpp"
#include "util/data/set.hpp"
#include "util/effect/file_loader.hpp"

TEST_SUITE_BEGIN("load_module");

using namespace amelia;

TEST_CASE("target module is in second directory in path") {
  LoaderContext ctx{Set<String>({"test/loader/logic/empty", "test/loader/logic/basic"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
}

TEST_CASE("imported module is in second directory in path") {
  LoaderContext ctx{Set<String>({"test/loader/logic/a_alone", "test/loader/logic/b_alone"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
}

TEST_CASE("target module does not exist") {
  LoaderContext ctx{Set<String>({"test/loader/logic/empty"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "a", ctx), RuntimeError);
}

TEST_CASE("imported module does not exist") {
  LoaderContext ctx{Set<String>({"test/loader/logic/a_alone"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "a", ctx), SourceLocationError);
}

TEST_CASE("target module has the same name in different directories with same contents") {
  LoaderContext ctx{Set<String>(
      {"test/loader/logic/a_alone", "test/loader/logic/a_alone_dup", "test/loader/logic/b_alone"}
  )};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
}

TEST_CASE("imported module has the same name in different directories with same contents") {
  LoaderContext ctx{Set<String>(
      {"test/loader/logic/c_alone_import_a",
       "test/loader/logic/a_alone",
       "test/loader/logic/a_alone_dup",
       "test/loader/logic/b_alone"}
  )};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "c", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("c"));
}

TEST_CASE("submodule has the same name in different directories with same contents") {
  LoaderContext ctx{Set<String>({"test/loader/logic/basic_sub", "test/loader/logic/basic_sub_dup"})
  };
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));
}

TEST_CASE("target module has the same name in different directories with different contents") {
  LoaderContext ctx{Set<String>(
      {"test/loader/logic/a_alone", "test/loader/logic/a_alone_diff", "test/loader/logic/b_alone"}
  )};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "a", ctx), RuntimeError);
}

TEST_CASE("imported module has the same name in different directories with different contents") {
  LoaderContext ctx{Set<String>(
      {"test/loader/logic/c_alone_import_a",
       "test/loader/logic/a_alone",
       "test/loader/logic/a_alone_diff",
       "test/loader/logic/b_alone"}
  )};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "c", ctx), SourceLocationError);
}

TEST_CASE("submodule has the same name in different directories with different contents") {
  LoaderContext ctx{Set<String>({"test/loader/logic/basic_sub", "test/loader/logic/basic_sub_diff"})
  };
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));
}

TEST_CASE("single file declares two submodules with the same name") {
  LoaderContext ctx{Set<String>({"test/loader/logic/duplicate_submodule"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "a", ctx), SourceLocationError);
}

TEST_CASE("target module is both a submodule 'b::c' within a.am and 'c' within a/b.am") {
  LoaderContext ctx{Set<String>({"test/loader/logic/duplicate_submodule_different_paths"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "a::b::c", ctx), RuntimeError);
}

TEST_CASE("imported module is both a submodule 'b::c' within a.am and 'c' within a/b.am") {
  LoaderContext ctx{Set<String>({"test/loader/logic/duplicate_submodule_different_paths"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  CHECK_THROWS_AS(load_module(file_loader, loader_result, "d", ctx), SourceLocationError);
}

TEST_CASE("a -> b") {
  LoaderContext ctx{Set<String>({"test/loader/logic/basic"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  REQUIRE(file_a->imported_files.has(file_b));
  REQUIRE(file_a->group_files.size() == 0);
  REQUIRE(file_a->imported_by_files.size() == 0);
  REQUIRE(file_b->imported_by_files.has(file_a));
  REQUIRE(file_b->imported_files.size() == 0);
  REQUIRE(file_b->group_files.size() == 0);
}

TEST_CASE("a -> b/c") {
  LoaderContext ctx{Set<String>({"test/loader/logic/subdir"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);

  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));

  // b::c is its own file within a subdirectory (b/c.am) rather than a submodule within b.am
  REQUIRE(!loader_result.file_id_by_module_name.has("b"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_bc = loader_result.get_source_file_by_module_name("b::c");

  REQUIRE(file_a->imported_files.size() == 1);
  REQUIRE(file_a->imported_files.has(file_bc));

  REQUIRE(file_bc->imported_by_files.size() == 1);
  REQUIRE(file_bc->imported_by_files.has(file_a));

  REQUIRE(file_a->group_files.size() == 0);

  REQUIRE(file_bc->group_files.size() == 0);
}

TEST_CASE("b/c -> a") {
  LoaderContext ctx{Set<String>({"test/loader/logic/subdir_rev"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "b::c", ctx);
  REQUIRE(loader_result.file_id_by_module_name.size() == 2);

  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));

  // b::c is its own file within a subdirectory (b/c.am) rather than a submodule within b.am
  REQUIRE(!loader_result.file_id_by_module_name.has("b"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b_c = loader_result.get_source_file_by_module_name("b::c");

  REQUIRE(file_a->imported_files.size() == 0);

  REQUIRE(file_a->imported_by_files.size() == 1);
  REQUIRE(file_a->imported_by_files.has(file_b_c));

  REQUIRE(file_a->group_files.size() == 0);

  REQUIRE(file_b_c->imported_files.size() == 1);
  REQUIRE(file_b_c->imported_files.has(file_a));

  REQUIRE(file_b_c->imported_by_files.size() == 0);

  REQUIRE(file_b_c->group_files.size() == 0);
}

TEST_CASE("a -> b::c") {
  LoaderContext ctx{Set<String>({"test/loader/logic/basic_sub"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_b_c = loader_result.get_source_file_by_module_name("b::c");

  REQUIRE(file_a->imported_files.size() == 1);
  REQUIRE(file_a->imported_files.has(file_b_c));

  // c is simply a submodule of b, so they have the same module ID and metadata
  REQUIRE(loader_result.loaded_files.size() == 2);
  REQUIRE(file_b == file_b_c);
  REQUIRE(file_b_c->imported_by_files.size() == 1);
  REQUIRE(file_b_c->imported_by_files.has(file_a));
}

TEST_CASE("b::c -> a") {
  LoaderContext ctx{Set<String>({"test/loader/logic/sub_rev"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "b::c", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_b_c = loader_result.get_source_file_by_module_name("b::c");

  REQUIRE(file_a->imported_files.size() == 0);

  REQUIRE(file_a->imported_by_files.size() == 1);
  REQUIRE(file_a->imported_by_files.has(file_b_c));

  REQUIRE(file_a->group_files.size() == 0);

  REQUIRE(file_b_c->imported_files.size() == 1);
  REQUIRE(file_b_c->imported_files.has(file_a));

  REQUIRE(file_b_c->imported_by_files.size() == 0);

  REQUIRE(file_b_c->group_files.size() == 0);
}

TEST_CASE("2 circular imports") {
  LoaderContext ctx{Set<String>({"test/loader/logic/two_circular"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 2);
  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");

  REQUIRE(file_a->imported_files.size() == 0);
  REQUIRE(file_b->imported_files.size() == 0);
  REQUIRE(file_a->imported_by_files.size() == 0);
  REQUIRE(file_b->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 1);
  REQUIRE(file_a->group_files.has(file_b));

  REQUIRE(file_b->group_files.size() == 1);
  REQUIRE(file_b->group_files.has(file_a));
}

TEST_CASE("3 circular imports") {
  LoaderContext ctx{Set<String>({"test/loader/logic/three_circular"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_c = loader_result.get_source_file_by_module_name("c");

  REQUIRE(file_a->imported_files.size() == 0);
  REQUIRE(file_b->imported_files.size() == 0);
  REQUIRE(file_c->imported_files.size() == 0);
  REQUIRE(file_a->imported_by_files.size() == 0);
  REQUIRE(file_b->imported_by_files.size() == 0);
  REQUIRE(file_c->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 2);
  REQUIRE(file_a->group_files.has(file_b));
  REQUIRE(file_a->group_files.has(file_c));

  REQUIRE(file_b->group_files.size() == 2);
  REQUIRE(file_b->group_files.has(file_a));
  REQUIRE(file_b->group_files.has(file_c));

  REQUIRE(file_c->group_files.size() == 2);
  REQUIRE(file_c->group_files.has(file_a));
  REQUIRE(file_c->group_files.has(file_b));
}

TEST_CASE("a -> b::c -> a") {
  LoaderContext ctx{Set<String>({"test/loader/logic/sub_circular"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.loaded_files.size() == 2);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("b::c"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_b_c = loader_result.get_source_file_by_module_name("b::c");
  REQUIRE(file_b == file_b_c);

  REQUIRE(file_a->imported_files.size() == 0);
  REQUIRE(file_b_c->imported_files.size() == 0);
  REQUIRE(file_a->imported_by_files.size() == 0);
  REQUIRE(file_b_c->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 1);
  REQUIRE(file_a->group_files.has(file_b_c));

  REQUIRE(file_b_c->group_files.size() == 1);
  REQUIRE(file_b_c->group_files.has(file_a));
}

TEST_CASE("a -> (b <-> c) -> d") {
  LoaderContext ctx{Set<String>({"test/loader/logic/circle_in_chain"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 4);
  REQUIRE(loader_result.loaded_files.size() == 4);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("c"));
  REQUIRE(loader_result.file_id_by_module_name.has("d"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_c = loader_result.get_source_file_by_module_name("c");
  auto file_d = loader_result.get_source_file_by_module_name("d");

  REQUIRE(file_a->imported_files.size() == 1);
  REQUIRE(file_a->imported_files.has(file_b));

  REQUIRE(file_a->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 0);

  REQUIRE(file_b->imported_files.size() == 0);

  REQUIRE(file_b->imported_by_files.size() == 1);
  REQUIRE(file_b->imported_by_files.has(file_a));

  REQUIRE(file_b->group_files.size() == 1);
  REQUIRE(file_b->group_files.has(file_c));

  REQUIRE(file_c->imported_files.size() == 1);
  REQUIRE(file_c->imported_files.has(file_d));

  REQUIRE(file_c->imported_by_files.size() == 0);

  REQUIRE(file_c->group_files.size() == 1);
  REQUIRE(file_c->group_files.has(file_b));

  REQUIRE(file_d->imported_files.size() == 0);

  REQUIRE(file_d->imported_by_files.size() == 1);
  REQUIRE(file_d->imported_by_files.has(file_c));

  REQUIRE(file_d->group_files.size() == 0);
}

TEST_CASE("a -> (b <-> c <-> d) -> e") {
  LoaderContext ctx{Set<String>({"test/loader/logic/three_circle_in_chain"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "a", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 5);
  REQUIRE(loader_result.loaded_files.size() == 5);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("c"));
  REQUIRE(loader_result.file_id_by_module_name.has("d"));
  REQUIRE(loader_result.file_id_by_module_name.has("e"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_c = loader_result.get_source_file_by_module_name("c");
  auto file_d = loader_result.get_source_file_by_module_name("d");
  auto file_e = loader_result.get_source_file_by_module_name("e");

  REQUIRE(file_a->imported_files.size() == 1);
  REQUIRE(file_a->imported_files.has(file_b));

  REQUIRE(file_a->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 0);

  REQUIRE(file_b->imported_files.size() == 0);

  REQUIRE(file_b->imported_by_files.size() == 1);
  REQUIRE(file_b->imported_by_files.has(file_a));

  REQUIRE(file_b->group_files.size() == 2);
  REQUIRE(file_b->group_files.has(file_c));
  REQUIRE(file_b->group_files.has(file_d));

  REQUIRE(file_c->imported_files.size() == 0);
  REQUIRE(file_c->imported_by_files.size() == 0);

  REQUIRE(file_c->group_files.size() == 2);
  REQUIRE(file_c->group_files.has(file_b));
  REQUIRE(file_c->group_files.has(file_d));

  REQUIRE(file_d->imported_files.size() == 1);
  REQUIRE(file_d->imported_files.has(file_e));

  REQUIRE(file_d->imported_by_files.size() == 0);

  REQUIRE(file_d->group_files.size() == 2);
  REQUIRE(file_d->group_files.has(file_b));
  REQUIRE(file_d->group_files.has(file_c));

  REQUIRE(file_e->imported_files.size() == 0);

  REQUIRE(file_e->imported_by_files.size() == 1);
  REQUIRE(file_e->imported_by_files.has(file_d));

  REQUIRE(file_e->group_files.size() == 0);
}

TEST_CASE("(a <-> b) + (a <-> c)") {
  LoaderContext ctx{Set<String>({"test/loader/logic/overlapping"})};
  FileLoader file_loader;
  LoaderResult loader_result;
  load_module(file_loader, loader_result, "c", ctx);

  REQUIRE(loader_result.file_id_by_module_name.size() == 3);
  REQUIRE(loader_result.loaded_files.size() == 3);
  REQUIRE(loader_result.file_id_by_module_name.has("a"));
  REQUIRE(loader_result.file_id_by_module_name.has("b"));
  REQUIRE(loader_result.file_id_by_module_name.has("c"));

  auto file_a = loader_result.get_source_file_by_module_name("a");
  auto file_b = loader_result.get_source_file_by_module_name("b");
  auto file_c = loader_result.get_source_file_by_module_name("c");

  REQUIRE(file_a->imported_files.size() == 0);
  REQUIRE(file_a->imported_by_files.size() == 0);

  REQUIRE(file_a->group_files.size() == 2);
  REQUIRE(file_a->group_files.has(file_b));
  REQUIRE(file_a->group_files.has(file_c));

  REQUIRE(file_b->imported_files.size() == 0);
  REQUIRE(file_b->imported_by_files.size() == 0);

  // group should transitively include c despite only directly importing a
  REQUIRE(file_b->group_files.size() == 2);
  REQUIRE(file_b->group_files.has(file_a));
  REQUIRE(file_b->group_files.has(file_c));

  REQUIRE(file_c->imported_files.size() == 0);
  REQUIRE(file_c->imported_by_files.size() == 0);

  // group should transitively include b despite only directly importing a
  REQUIRE(file_c->group_files.size() == 2);
  REQUIRE(file_c->group_files.has(file_a));
  REQUIRE(file_c->group_files.has(file_b));
}

TEST_SUITE_END();
