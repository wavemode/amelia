#pragma once

namespace amelia {

struct IFileLoader;
struct SemaResult;
struct ModuleLoaderContext;
class String;

void load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx
);

} // namespace amelia
