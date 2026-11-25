#pragma once

#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

template <typename T> class Slice;
template <typename T> class ConstSlice;

struct FilePath {
  FilePath() noexcept;
  explicit FilePath(Text path);

  template <size_t N>
  FilePath(const char (&str)[N])
      : FilePath(
            Text(ConstSlice(str, (N > 0 && str[N - 1] == '\0') ? N - 1 : N))
        ) {}

  bool is_absolute() const noexcept;
  void set_absolute(bool absolute) noexcept;

  bool is_root() const noexcept;

  void add_part(Text part);
  void pop_part() noexcept;

  Slice<String> parts() noexcept;
  ConstSlice<String> parts() const noexcept;
  size_t num_parts() const noexcept;

  Text extension() noexcept;
  void set_extension(Text extension);

  void clear() noexcept;

  void to_string(AbstractString &output) const;

  uint64_t hash_code() const;
  bool operator==(const FilePath &other) const;
  bool operator!=(const FilePath &other) const;

private:
  List<String> m_parts;
  String m_extension;
  bool m_absolute;
};

} // namespace amelia
