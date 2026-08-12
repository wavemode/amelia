#pragma once

namespace amelia {

struct LoaderResult;
struct IFileLoader;
struct LoaderContext;
class String;

void load_module(
    IFileLoader &file_loader,
    LoaderResult &loader_result,
    const String &module_name,
    const LoaderContext &ctx
);

} // namespace amelia
