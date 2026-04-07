#include "ConsolePrinter.h"

#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#endif

#include "util/string/String.h"

amelia::ConsolePrinter::ConsolePrinter() {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
#endif
}

void amelia::ConsolePrinter::print(const String &str) { std::cout << str.c_str(); }

void amelia::ConsolePrinter::println(const String &str) { std::cout << str.c_str() << std::endl; }

void amelia::ConsolePrinter::err_print(const String &str) { std::cerr << str.c_str(); }

void amelia::ConsolePrinter::err_println(const String &str) {
  std::cerr << str.c_str() << std::endl;
}
