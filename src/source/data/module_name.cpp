#include "module_name.hpp"

#include "util/data/char_iterator.hpp"
#include "util/data/slice.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

ModuleName::ModuleName() noexcept = default;

ModuleName::ModuleName(Text name) {
  if (name.size() == 0) {
    return;
  }

  List<Text> split_parts;
  TextUtils::split(split_parts, name, "::");

  for (const auto &part : split_parts) {
    m_parts.emplace_back(part);
  }
}

void ModuleName::add_part(Text part) { m_parts.emplace_back(part); }

void ModuleName::pop_part() noexcept {
  if (m_parts.size() > 0) {
    m_parts.pop_back();
  }
}

Text ModuleName::short_name() const noexcept {
  return m_parts.size() > 0 ? m_parts[m_parts.size() - 1] : Text();
}

bool ModuleName::is_submodule_of(const ModuleName &other) const noexcept {
  if (other.m_parts.size() >= m_parts.size()) {
    return false;
  }

  for (size_t i = 0; i < other.m_parts.size(); ++i) {
    if (m_parts[i] != other.m_parts[i]) {
      return false;
    }
  }

  return true;
}

Slice<String> ModuleName::parts() noexcept {
  return {m_parts.data().ptr(), m_parts.size()};
}

ConstSlice<String> ModuleName::parts() const noexcept {
  return {m_parts.data().ptr(), m_parts.size()};
}

size_t ModuleName::num_parts() const noexcept { return m_parts.size(); }

void ModuleName::validate() const {
  if (m_parts.size() == 0) {
    throw_runtime_error("Module name cannot be empty");
  }

  for (const auto &part : m_parts) {
    if (part.size() == 0) {
      throw_runtime_error("Module name segment cannot be empty");
    }

    for (char ch : part.data()) {
      if (!(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z') &&
          !(ch >= '0' && ch <= '9') && ch != '_') {
        throw_runtime_error("Illegal character in module name");
      }
    }
  }
}

void ModuleName::to_string(AbstractString &output) const {
  for (size_t i = 0; i < m_parts.size(); ++i) {
    output.append(m_parts[i]);
    if (i < m_parts.size() - 1) {
      output.append("::");
    }
  }
}

uint64_t ModuleName::hash_code() const noexcept {
  HashState st;
  st.init();
  for (size_t i = 0; i < m_parts.size(); ++i) {
    st.update(m_parts[i].data().ptr(), m_parts[i].data().size());
    if (i < m_parts.size() - 1) {
      st.update("::", 2);
    }
  }
  return st.finalize();
}

bool ModuleName::operator==(const ModuleName &other) const noexcept {
  return m_parts == other.m_parts;
}

bool ModuleName::operator!=(const ModuleName &other) const noexcept {
  return !(*this == other);
}

} // namespace amelia
