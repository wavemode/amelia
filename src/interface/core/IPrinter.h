#pragma once

namespace amelia {

class Text;

struct IPrinter {
  virtual void print(Text) = 0;
  virtual void println(Text) = 0;
  virtual void err_print(Text) = 0;
  virtual void err_println(Text) = 0;
};

} // namespace amelia
