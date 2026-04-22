#include "LexerTestCaseRunner.h"
#include "data/core/List.h"
#include "data/core/Text.h"
#include "data/lexer/LexerContext.h"
#include "data/source/Token.h"
#include "data/testing/CompilerTestCase.h"
#include "interface/text/IString.h"
#include "unit/lexer/Lexer.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(CompilerTestCase input, IString &output) {
  List<Token> tokens;
  Lexer lexer;
  lexer.tokenize(LexerContext{input.filename}, input.input, tokens);
  for (const auto &token : tokens) {
    token.to_string(output);
    output.append("\n");
  }
}

} // namespace amelia
