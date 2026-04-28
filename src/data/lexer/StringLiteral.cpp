#include <vector>

#include "Prelude.h"
#include "StringLiteral.h"

#include "data/lexer/StringLiteralReadError.h"

namespace amelia {

namespace {
uint32_t read_hex_chars(size_t num_chars, CharIterator &iter) {
  uint32_t result = 0;
  for (size_t i = 0; i < num_chars; ++i) {
    if (iter.at_end()) {
      throw StringLiteralReadError("Unexpected end of input in hexadecimal escape sequence");
    }
    uint32_t ch = iter.next();
    result <<= 4;
    if (ch >= '0' && ch <= '9') {
      result |= (ch - '0');
    } else if (ch >= 'a' && ch <= 'f') {
      result |= (ch - 'a' + 10);
    } else if (ch >= 'A' && ch <= 'F') {
      result |= (ch - 'A' + 10);
    } else {
      throw StringLiteralReadError("Invalid character in hexadecimal escape sequence");
    }
  }
  return result;
}
} // namespace

void StringLiteral::read(IString &output, CharIterator &iter, bool is_raw) {
  std::vector<char> result;
  while (!iter.at_end()) {
    uint32_t ch = iter.next();
    if (ch == '\\' && !is_raw) {
      if (iter.at_end()) {
        throw StringLiteralReadError("Unexpected end of input after backslash in string literal");
      }
      ch = iter.next();
      switch (ch) {
      case 'a':
        result.push_back(static_cast<char>('\a'));
        break;
      case 'b':
        result.push_back(static_cast<char>('\b'));
        break;
      case 'f':
        result.push_back(static_cast<char>('\f'));
        break;
      case 'n':
        result.push_back(static_cast<char>('\n'));
        break;
      case 'r':
        result.push_back(static_cast<char>('\r'));
        break;
      case 't':
        result.push_back(static_cast<char>('\t'));
        break;
      case 'v':
        result.push_back(static_cast<char>('\v'));
        break;
      case '\\':
        result.push_back(static_cast<char>('\\'));
        break;
      case '\'':
        result.push_back(static_cast<char>('\''));
        break;
      case '"':
        result.push_back(static_cast<char>('\"'));
        break;
      case '\n':
        break;
      case 'x':
        CharIterator::append(read_hex_chars(2, iter), result);
        break;
      case 'u':
        CharIterator::append(read_hex_chars(4, iter), result);
        break;
      case 'U':
        CharIterator::append(read_hex_chars(8, iter), result);
        break;
      default:
        String msg = "Invalid escape sequence: '\\";
        msg.append(ch);
        msg.append('\'');
        throw StringLiteralReadError(std::move(msg));
      }
    } else if (ch == '\r') {
      // skip
    } else {
      CharIterator::append(ch, result);
    }
  }
  output.append(Text(Slice(static_cast<const char *>(result.data()), result.size())));
}

} // namespace amelia
