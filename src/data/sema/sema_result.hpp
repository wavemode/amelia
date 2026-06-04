#pragma once

#include "prelude.hpp"

#include "data/util/map.hpp"
#include "data/sema/module.hpp"

namespace amelia {

struct SemaResult {
  Map<Text, ModuleId> module_ids;
  List<Module> modules;
  List<List<ModuleId>> module_imports;
  List<List<ModuleId>> modules_imported_by;
  List<List<ModuleId>> module_group_ids;
  List<List<List<Binding*>>> module_binding_deps;
};

} // namespace amelia
