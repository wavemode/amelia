#include "file_path.hpp"

#include "util/data/char_iterator.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

FilePath::FilePath() noexcept : m_absolute(true) {}

FilePath::FilePath(Text path) : m_absolute(false) {
  if (path.size() == 0) {
    return;
  }

  if (path.data()[0] == '/') {
    m_absolute = true;
  }

  path = TextUtils::trim(path, "/");
  List<Text> split_parts;
  TextUtils::split(m_parts, path, "/");
  for (const auto &part : split_parts) {
    if (part.size() > 0) {
      m_parts.emplace_back(part);
    }
  }
}

bool FilePath::is_absolute() const noexcept { return m_absolute; }

void FilePath::set_absolute(bool absolute) noexcept { m_absolute = absolute; }

bool FilePath::is_root() const noexcept {
  return m_parts.size() == 0 && m_absolute;
}

void FilePath::add_part(Text part) { m_parts.emplace_back(part); }

void FilePath::pop_part() noexcept {
  if (m_parts.size() > 0) {
    m_parts.pop_back();
  }
}

Slice<String> FilePath::parts() noexcept { return m_parts.data(); }

ConstSlice<String> FilePath::parts() const noexcept { return m_parts.data(); }

size_t FilePath::num_parts() const noexcept { return m_parts.size(); }

Text FilePath::extension() noexcept { return m_extension.text(); }

void FilePath::set_extension(Text extension) { m_extension.assign(extension); }

void FilePath::clear() noexcept { m_parts.clear(); }

void FilePath::to_string(AbstractString &output) const {
  if (m_absolute) {
    output.append('/');
  }
  for (size_t i = 0; i < m_parts.size(); ++i) {
    output.append(m_parts[i]);
    if (i < m_parts.size() - 1) {
      output.append('/');
    }
  }
  if (m_extension.size() > 0) {
    output.append(m_extension);
  }
}

uint64_t FilePath::hash_code() const {
  HashState st;
  st.init();
  if (m_absolute) {
    st.update("/", 1);
  }
  for (const auto &path_part : m_parts) {
    st.update(path_part.data().ptr(), path_part.data().size());
  }
  st.update(m_extension.data().ptr(), m_extension.data().size());
  return st.finalize();
}

bool FilePath::operator==(const FilePath &other) const {
  return m_absolute == other.m_absolute && m_parts == other.m_parts;
}

bool FilePath::operator!=(const FilePath &other) const {
  return !(*this == other);
}

} // namespace amelia
