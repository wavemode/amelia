#pragma once

namespace amelia {

struct LoaderResult;
struct IFileLoader;
struct LoaderSettings;
struct ModuleName;

void load_module(LoaderResult &result, IFileLoader &file_loader,
                 ModuleName module_name);

} // namespace amelia
