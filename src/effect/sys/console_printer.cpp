#include "console_printer.h"
#include "prelude.h"

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace amelia {

ConsolePrinter::ConsolePrinter() {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
#endif
}

void ConsolePrinter::print(Text str) { std::cout.write(str.data().ptr(), str.data().size()); }

void ConsolePrinter::println(Text str) {
  print(str);
  std::cout << std::endl;
}

void ConsolePrinter::err_print(Text str) { std::cerr.write(str.data().ptr(), str.data().size()); }

void ConsolePrinter::err_println(Text str) {
  err_print(str);
  std::cerr << std::endl;
}

} // namespace amelia
