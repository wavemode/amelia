#pragma once

#include "prelude.hpp"

#include "fs/interface/file_loader.hpp"
#include "sema/data/module_loader_context.hpp"
#include "sema/data/sema_result.hpp"

namespace amelia {

void load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx
);

} // namespace amelia
