#include "ConsolePrinter.h"

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "util/text/Text.h"

amelia::ConsolePrinter::ConsolePrinter() {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
#endif
}

void amelia::ConsolePrinter::print(Text str) {
  std::cout.write(str.data().data(), str.data().size());
}

void amelia::ConsolePrinter::println(Text str) {
  print(str);
  std::cout << std::endl;
}

void amelia::ConsolePrinter::err_print(Text str) {
  std::cerr.write(str.data().data(), str.data().size());
}

void amelia::ConsolePrinter::err_println(Text str) {
  err_print(str);
  std::cerr << std::endl;
}
