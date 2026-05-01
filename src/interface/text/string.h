#pragma once

#include <cstdint>

namespace amelia {

class Text;

struct IString {
  virtual ~IString() = default;

  virtual Text text() const noexcept = 0;
  virtual void assign(Text text) = 0;
  virtual void append(Text text) = 0;
  virtual void append(uint32_t code_point) = 0;
  virtual const char *c_str() const noexcept = 0;
};

} // namespace amelia
