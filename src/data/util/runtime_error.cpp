#include "runtime_error.hpp"

#include <cstddef>
#include <cstring>

namespace amelia {
RuntimeError::RuntimeError(const char *message) {
  size_t message_length = std::strlen(message);
  char *buffer = new char[message_length + 1];
  std::strcpy(const_cast<char *>(buffer), message);
  m_message = buffer;
}

const char *RuntimeError::what() const noexcept {
  return m_message;
}

RuntimeError::~RuntimeError() noexcept {
  delete[] m_message;
}

} // namespace amelia
