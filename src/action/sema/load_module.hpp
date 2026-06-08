#pragma once

#include "prelude.hpp"

#include "data/sema/module_loader_context.hpp"
#include "data/sema/sema_result.hpp"
#include "interface/fs/file_loader.hpp"

namespace amelia {

void load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx
);

} // namespace amelia
