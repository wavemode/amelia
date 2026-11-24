#include "loader/data/loaded_module_submodule_decl.hpp"
#include "resolver/data/resolved_module_import.hpp"
#include "resolver/data/resolver_result.hpp"

namespace amelia {

namespace {

struct ResolverContext {
  Ref<ResolverResult> resolver_result;

  Option<Ref<LoadedModuleDetails>>
  get_module_details(const FilePath &module_path,
                     const ModuleName &submodule_name) {
    auto module_file =
        resolver_result->loader_result.loaded_files.find(module_path);
    if (!module_file.has_value()) {
      return {};
    }

    auto module_details = Ref((*module_file.value())->details);
    for (const String &part : submodule_name.parts()) {
      auto submodule_details = module_details->submodules.find(part.text());
      if (submodule_details.has_value()) {
        module_details = Ref((*submodule_details.value())->details);
      } else {
        return {};
      }
    }
    return module_details;
  }

  void find_module(List<Ref<LoadedModuleDetails>> &results,
                   const ModuleName &module_name) {
    for (const FilePath &search_path :
         resolver_result->loader_result.settings.search_path) {

      FilePath working_path = search_path;
      for (const auto &module_name_part : module_name.parts()) {
        working_path.add_part(module_name_part);
      }
      working_path.set_extension(".am");

      List<String> working_submodule_name_parts;

      while (working_path.num_parts() > search_path.num_parts()) {

        auto module_file =
            resolver_result->loader_result.loaded_files.find(working_path);
        if (module_file.has_value()) {

          auto module_details = Option(Ref((*module_file.value())->details));
          for (size_t i = working_submodule_name_parts.size(); i > 0; --i) {

            const String &submodule_name_part =
                working_submodule_name_parts[i - 1];
            auto submodule_details = module_details.value()->submodules.find(
                submodule_name_part.text());

            if (submodule_details.has_value()) {
              module_details =
                  Option(Ref((*submodule_details.value())->details));
            } else {
              module_details = {};
              break;
            }

          }

          if (module_details.has_value()) {
            results.push_back(module_details.value());
          }

        }

        working_submodule_name_parts.push_back(
            move(working_path.parts()[working_path.num_parts() - 1]));
        working_path.pop_part();
      }
    }
  }
};

} // namespace

} // namespace amelia
