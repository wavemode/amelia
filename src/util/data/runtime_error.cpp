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

RuntimeError::RuntimeError(const RuntimeError &other) : RuntimeError(other.m_message) {}

RuntimeError::RuntimeError(RuntimeError &&other) noexcept : m_message(other.m_message) {
  other.m_message = nullptr;
}

RuntimeError &RuntimeError::operator=(const RuntimeError &other) {
  if (this != &other) {
    delete[] m_message;
    size_t message_length = std::strlen(other.m_message);
    char *buffer = new char[message_length + 1];
    std::strcpy(buffer, other.m_message);
    m_message = buffer;
  }
  return *this;
}

RuntimeError &RuntimeError::operator=(RuntimeError &&other) noexcept {
  if (this != &other) {
    delete[] m_message;
    m_message = other.m_message;
    other.m_message = nullptr;
  }
  return *this;
}

const char *RuntimeError::what() const noexcept {
  return m_message;
}

RuntimeError::~RuntimeError() noexcept {
  delete[] m_message;
}

} // namespace amelia
