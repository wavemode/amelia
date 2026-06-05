#include "analysis.hpp"

#include "data/util/text_utils.hpp"

namespace amelia {

void load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const Text &module_name,
    const ModuleLoaderContext &ctx
) {
  List<String> module_name_parts;
  TextUtils::split(module_name_parts, module_name, "::");
}

} // namespace amelia
