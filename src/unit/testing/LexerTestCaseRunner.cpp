#include "LexerTestCaseRunner.h"
#include "Prelude.h"

#include "data/lexer/LexerContext.h"
#include "data/source/Token.h"
#include "data/testing/CompilerTestCase.h"

#include "unit/lexer/Lexer.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(IString &output, CompilerTestCase input) {
  List<Token> tokens;
  Lexer lexer;
  lexer.tokenize(LexerContext{input.filename}, input.input, tokens);
  for (const auto &token : tokens) {
    token.to_string(output);
    output.append("\n");
  }
}

} // namespace amelia
