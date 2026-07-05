#include <cstdio>

#include "identifier.hpp"

#include "lexer/data/lexer.hpp"
#include "source/data/char_literal.hpp"
#include "util/data/set.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

namespace {

const Set<Text> ADDITIONAL_RESERVED_WORDS = Set<Text>({
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "asm",
    "atomic_cancel",
    "atomic_commit",
    "atomic_noexcept",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char8_t",
    "char16_t",
    "char32_t",
    "class",
    "compl",
    "concept",
    "const",
    "consteval",
    "constexpr",
    "constinit",
    "const_cast",
    "continue",
    "contract_assert",
    "co_await",
    "co_return",
    "co_yield",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reflexpr",
    "register",
    "reinterpret_cast",
    "requires",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "synchronized",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    "final",
    "override",
    "transaction_safe",
    "transaction_safe_dynamic",
    "import",
    "module",
    "pre",
    "post",
    "std",
    "alignas",
    "alignof",
    "auto",
    "bool",
    "break",
    "case",
    "char",
    "const",
    "constexpr",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "false",
    "float",
    "for",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "nullptr",
    "register",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "struct",
    "switch",
    "thread_local",
    "true",
    "typedef",
    "typeof",
    "typeof_unqual",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
});

} // namespace

Identifier::Identifier(Text name) {
  if (TextUtils::starts_with(name, "`")) {
    name = TextUtils::substr_bytes(name, 1, name.size() - 1);
  }
  bool needs_quote = false;

  if (RESERVED_WORDS.has(name) || ADDITIONAL_RESERVED_WORDS.has(name)) {
    needs_quote = true;
  } else {
    size_t index = 0;
    bool previous_char_was_underscore = false;
    for (uint32_t ch : name) {
      if (ch == '_') {
        if (index == 0 || previous_char_was_underscore) {
          needs_quote = true;
          break;
        } else {
          previous_char_was_underscore = true;
        }
      } else {
        previous_char_was_underscore = false;
        if (ch >= '0' && ch <= '9') {
          if (index == 0) {
            needs_quote = true;
            break;
          }
        } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
          // always valid
        } else {
          needs_quote = true;
          break;
        }
      }
      ++index;
    }
  }

  if (needs_quote) {
    m_name.append('`');
  }
  m_name.append(name);
  if (needs_quote) {
    m_name.append('`');
  }
}

Text Identifier::literal() const noexcept {
  return m_name.text();
}

void Identifier::pretty_print(AbstractString &out, bool quoted, bool escaped) const {
  Text name = m_name.text();

  if (!quoted && TextUtils::starts_with(name, "`")) {
    name = TextUtils::substr_bytes(name, 1, name.size() - 1);
  }

  if (!escaped) {
    out.append(name);
    return;
  }

  size_t index = 0;
  for (uint32_t ch : name) {
    switch (ch) {
    case '`':
      if (index != 0 && index != name.size() - 1) {
        out.append('\\');
      }
      out.append('`');
      break;
    default:
      serialize_char_literal(ch, false).to_string(out);
      break;
    }
  }
}

} // namespace amelia
