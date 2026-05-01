#pragma once

#include "interface/sys/printer.h"

namespace amelia {

class Text;

class SilentPrinter : public IPrinter {
public:
  SilentPrinter();

  void print(Text) override;
  void println(Text) override;
  void err_print(Text) override;
  void err_println(Text) override;
};

} // namespace amelia
