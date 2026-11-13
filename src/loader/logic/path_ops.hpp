#pragma once

namespace amelia {

struct AbstractString;
template <typename T> class ConstSlice;
class Text;

void append_paths(AbstractString &output, ConstSlice<Text> paths);
Text directory_name(Text path);

} // namespace amelia
