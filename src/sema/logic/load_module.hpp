#pragma once

namespace amelia {

struct SemaNamespace;
struct LoaderContext;
class Text;

void load_module(SemaNamespace &result, Text path,
                 const LoaderContext &loaderContext);

} // namespace amelia
