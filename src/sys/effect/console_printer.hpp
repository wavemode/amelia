#pragma once

#include "sys/interface/printer.hpp"

namespace amelia {

class Text;

class ConsolePrinter : public IPrinter {
public:
  ConsolePrinter();

  void print(Text) override;
  void println(Text) override;
  void err_print(Text) override;
  void err_println(Text) override;
};

} // namespace amelia
