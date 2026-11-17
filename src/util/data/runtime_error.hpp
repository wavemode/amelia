#pragma once

#include <exception>

#include "util/data/string.hpp"

namespace amelia {

class RuntimeError : public std::exception {
public:
  explicit RuntimeError(String &&message)
      : m_message(static_cast<String &&>(message)) {}

  const char *what() const noexcept override { return m_message.c_str(); }

private:
  String m_message;
};

} // namespace amelia
