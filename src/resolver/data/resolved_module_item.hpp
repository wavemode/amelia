#pragma once

#include "abstract_resolved_module_item.hpp"
#include "loader/data/loaded_module_item_decl.hpp"
#include "util/data/list.hpp"
#include "util/data/ref.hpp"

namespace amelia {

struct ResolvedModuleItem : AbstractResolvedModuleItem {
  List<Ref<LoadedModuleItemDecl>> decls;
};

} // namespace amelia
