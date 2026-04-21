#include <vector>

#include "LexerTestCaseRunner.h"
#include "data/lexer/LexerContext.h"
#include "data/testing/CompilerTestCase.h"
#include "data/source/Token.h"
#include "interface/text/IString.h"
#include "unit/lexer/Lexer.h"
#include "util/text/Text.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(CompilerTestCase input, IString &output) {
  std::vector<Token> tokens;
  Lexer lexer;
  lexer.tokenize(
    LexerContext{input.filename},
    input.input,
    tokens
  );
  for (const auto &token : tokens) {
    token.to_string(output);
    output.append("\n");
  }
}

} // namespace amelia
