#include "LexerTestCaseRunner.h"
#include "Prelude.h"

#include "data/lexer/LexerContext.h"
#include "data/source/Token.h"
#include "data/testing/CompilerTestCase.h"

#include "data/lexer/Lexer.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(IString &output, CompilerTestCase test_case) {
  List<Token> tokens;
  CharIterator iter(test_case.input);
  Lexer::tokenize(tokens, iter, LexerContext{test_case.filename});
  for (const auto &token : tokens) {
    token.to_string(output);
    output.append("\n");
  }
}

} // namespace amelia
