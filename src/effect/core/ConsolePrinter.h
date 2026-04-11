#pragma once

#include "interface/core/IPrinter.h"

namespace amelia {

class String;

class ConsolePrinter : public IPrinter {
public:
  ConsolePrinter();

  void print(const String &) override;
  void println(const String &) override;
  void err_print(const String &) override;
  void err_println(const String &) override;
};

} // namespace amelia
