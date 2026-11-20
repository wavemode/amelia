
#include "loader/data/loaded_module_import_decl.hpp"
#include "resolver/data/resolved_module_import.hpp"
#include "resolver/data/resolver_result.hpp"

namespace amelia {
namespace {

struct ResolverContext {
  Ref<ResolverResult> loader_result;
  Option<Ref<ResolvedModuleDetails>> module_details;

  void process_top_level_decls() {
    // loop over module_details->details->decls and add them to the resolved
    // details decl mapping by name. group adjacent items with the same name.
    // error if an imported name conflicts with a declared name, or if two
    // nonadjacent declared names conflict with each other.
    throw_runtime_error("not implemented");
  }

  Option<Ref<ResolvedModuleDetails>> resolve_module(Text module_name) {
    // return immediately if already resolved
    // otherwise, loop through all possible loaded files which could contain the
    // module (if more than one possibility is found, throw an error) then build
    // a ResolvedModuleDetails object for the module and store it in the
    // LoaderResult
    throw_runtime_error("not implemented");
  }

  Option<Ref<AbstractResolvedModuleItem>> resolve_module_item(Text module_name,
                                                              Text item_name) {
    // resolve module name
    // then check resolved item. raise an error if it is a null import item,
    // or if it is not accessible to the current module.
    // then return the initial resolution
    throw_runtime_error("not implemented");
  }

  bool is_visibile(const LoadedDeclarationVisibility &visibility) {
    // check if visibility is either public, or scoped to a module which is a
    // parent of the current module if visibility is default, imported items are
    // local by default and other items are public by default
    throw_runtime_error("not implemented");
  }

  void wildcard_import_from_module(Text module_name) {
    // resolve module name
    // then loop over all visibile items of the module and add them to the
    // current module as imported items
  }
};


} // namespace
} // namespace amelia
