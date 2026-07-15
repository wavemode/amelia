#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "console_printer.hpp"

#include "util/data/text.hpp"

namespace amelia {

ConsolePrinter::ConsolePrinter() {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
#endif
}

void ConsolePrinter::print(Text str) {
  std::fwrite(str.data().ptr(), 1, str.data().size(), stdout);
}

void ConsolePrinter::println(Text str) {
  print(str);
  std::fputc('\n', stdout);
}

void ConsolePrinter::err_print(Text str) {
  std::fwrite(str.data().ptr(), 1, str.data().size(), stderr);
}

void ConsolePrinter::err_println(Text str) {
  err_print(str);
  std::fputc('\n', stderr);
}

} // namespace amelia
