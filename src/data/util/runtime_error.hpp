#pragma once

#include <exception>

namespace amelia {

class RuntimeError : public std::exception {
public:
  explicit RuntimeError(const char *message);
  const char *what() const noexcept override;

  ~RuntimeError() noexcept override;
private:
  const char *m_message;
};

} // namespace amelia
