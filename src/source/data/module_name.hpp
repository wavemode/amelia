#pragma once

#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

class Text;
template <typename T> class Slice;
template <typename T> class ConstSlice;

struct ModuleName {
  ModuleName() noexcept;
  ModuleName(Text name);

  void add_part(Text part);
  void pop_part() noexcept;

  Slice<String> parts() noexcept;
  ConstSlice<String> parts() const noexcept;
  size_t num_parts() const noexcept;

  void validate() const;
  void to_string(AbstractString &output) const;
  uint64_t hash_code() const noexcept;

  bool operator==(const ModuleName &other) const noexcept;
  bool operator!=(const ModuleName &other) const noexcept;

private:
  List<String> m_parts;
};

} // namespace amelia
