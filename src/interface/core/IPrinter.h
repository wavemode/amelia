#pragma once

namespace amelia {

class String;

class IPrinter {
public:
  virtual void print(const String &) = 0;
  virtual void println(const String &) = 0;
  virtual void err_print(const String &) = 0;
  virtual void err_println(const String &) = 0;
};

} // namespace amelia
