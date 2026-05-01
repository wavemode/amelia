#include <doctest.h>

#include "data/source/number_literal.h"
#include "prelude.h"

#include "data/source/number_literal_read_error.h"
#include "data/text/text_utils.h"

TEST_SUITE_BEGIN("NumberLiteral");

using namespace amelia;

namespace {
NumberLiteral read_text(Text text) {
  CharIterator it(text);
  return NumberLiteral::read(it);
}
} // namespace

TEST_CASE("failure on empty input") { CHECK_THROWS_AS(read_text(""), RuntimeError); }

TEST_CASE("general examples") {
  CHECK(
      read_text("42") ==
      NumberLiteral{
          .has_decimal_point = false,
          .integer_digits = "42",
      }
  );
  CHECK(
      read_text("4_2") ==
      NumberLiteral{
          .has_decimal_point = false,
          .integer_digits = "4_2",
      }
  );
  CHECK(
      read_text("0600") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0",
          .integer_digits = "600",
      }
  );
  CHECK(
      read_text("0_600") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0",
          .integer_digits = "_600",
      }
  );
  CHECK(
      read_text("0o600") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0o",
          .integer_digits = "600",
      }
  );
  CHECK(
      read_text("0O600") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0O",
          .integer_digits = "600",
      }
  );
  CHECK(
      read_text("0xBadFace") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0x",
          .integer_digits = "BadFace",
      }
  );
  CHECK(
      read_text("0xBad_Face") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0x",
          .integer_digits = "Bad_Face",
      }
  );
  CHECK(
      read_text("0x_67_7a_2f_cc_40_c6") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0x",
          .integer_digits = "_67_7a_2f_cc_40_c6",
      }
  );
  CHECK(
      read_text("170141183460469231731687303715884105727") ==
      NumberLiteral{
          .has_decimal_point = false,
          .integer_digits = "170141183460469231731687303715884105727",
      }
  );
  CHECK(
      read_text("170_141183_460469_231731_687303_715884_105727") ==
      NumberLiteral{
          .has_decimal_point = false,
          .integer_digits = "170_141183_460469_231731_687303_715884_105727",
      }
  );

  CHECK(
      read_text("0.") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "",
          .integer_digits = "0",
      }
  );
  CHECK(
      read_text("72.40") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "",
          .integer_digits = "72",
          .fractional_digits = "40",
      }
  );
  CHECK(
      read_text("072.40       ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "",
          .integer_digits = "072",
          .fractional_digits = "40",
      }
  );
  CHECK(
      read_text("2.71828") ==
      NumberLiteral{
          .has_decimal_point = true,
          .integer_digits = "2",
          .fractional_digits = "71828",
      }
  );
  CHECK(
      read_text("1.e+0") ==
      NumberLiteral{
          .has_decimal_point = true,
          .integer_digits = "1",
          .exponent_prefix = "e",
          .exponent_sign = "+",
          .exponent_digits = "0",
      }
  );
  CHECK(
      read_text("6.67428e-11") ==
      NumberLiteral{
          .has_decimal_point = true,
          .integer_digits = "6",
          .fractional_digits = "67428",
          .exponent_prefix = "e",
          .exponent_sign = "-",
          .exponent_digits = "11",
      }
  );
  CHECK(
      read_text("1E6") ==
      NumberLiteral{
          .has_decimal_point = false,
          .integer_digits = "1",
          .exponent_prefix = "E",
          .exponent_digits = "6",
      }
  );
  CHECK(
      read_text(".25") ==
      NumberLiteral{
          .has_decimal_point = true,
          .fractional_digits = "25",
      }
  );
  CHECK(
      read_text(".12345E+5") ==
      NumberLiteral{
          .has_decimal_point = true,
          .fractional_digits = "12345",
          .exponent_prefix = "E",
          .exponent_sign = "+",
          .exponent_digits = "5",
      }
  );
  CHECK(
      read_text("1_5.        ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .integer_digits = "1_5",
      }
  );
  CHECK(
      read_text("0.15e+0_2   ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "",
          .integer_digits = "0",
          .fractional_digits = "15",
          .exponent_prefix = "e",
          .exponent_sign = "+",
          .exponent_digits = "0_2",
      }
  );
  CHECK(
      read_text("0x1p-2       ") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0x",
          .integer_digits = "1",
          .exponent_prefix = "p",
          .exponent_sign = "-",
          .exponent_digits = "2",
      }
  );
  CHECK(
      read_text("0x2.p10      ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "0x",
          .integer_digits = "2",
          .exponent_prefix = "p",
          .exponent_digits = "10",
      }
  );
  CHECK(
      read_text("0x1.Fp+0     ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "0x",
          .integer_digits = "1",
          .fractional_digits = "F",
          .exponent_prefix = "p",
          .exponent_sign = "+",
          .exponent_digits = "0",
      }
  );
  CHECK(
      read_text("0X.8p-0      ") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "0X",
          .fractional_digits = "8",
          .exponent_prefix = "p",
          .exponent_sign = "-",
          .exponent_digits = "0",
      }
  );
  CHECK(
      read_text("0X_1FFFP-16  ") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0X",
          .integer_digits = "_1FFF",
          .exponent_prefix = "P",
          .exponent_sign = "-",
          .exponent_digits = "16",
      }
  );
  CHECK(
      read_text("0x15e-2      ") ==
      NumberLiteral{
          .has_decimal_point = false,
          .base_prefix = "0x",
          .integer_digits = "15e",
      }
  );
  CHECK(
      read_text("0x1.5e-2") ==
      NumberLiteral{
          .has_decimal_point = true,
          .base_prefix = "0x",
          .integer_digits = "1",
          .fractional_digits = "5e",
      }
  );
  CHECK_THROWS_WITH(read_text("0x.p1"), "Number literal must have at least one digit");
  CHECK_THROWS_WITH(
      read_text("1p-2"), "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
  );
  CHECK_THROWS_WITH(read_text("1_.5"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("1._5"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("1.5_e1"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("1.5e_1"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("1.5e1_"), "Underscore must separate successive digits");

  CHECK_THROWS_WITH(read_text("_42"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("42_"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("4__2"), "Underscore must separate successive digits");
  CHECK_THROWS_WITH(read_text("0_xBadFace"), "Invalid character 'x' in number literal");
}

TEST_CASE("decimal integer") {
  SUBCASE("simple") {
    CHECK(
        read_text("1234") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "1234",
        }
    );
  }

  SUBCASE("single zero") {
    CHECK(
        read_text("0") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "0",
        }
    );
  }

  SUBCASE("single zero following by non-number character") {
    CHECK(
        read_text("0{") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "0",
        }
    );
  }

  SUBCASE("decimal integer with underscores") {
    CHECK(
        read_text("1_234_567") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "1_234_567",
        }
    );
  }

  SUBCASE("failure if decimal integer contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("1__234"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if decimal integer starts with underscore") {
    CHECK_THROWS_WITH(read_text("_1234"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if decimal integer ends with underscore") {
    CHECK_THROWS_WITH(read_text("1234_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if decimal integer contains non digit characters") {
    CHECK_THROWS_WITH(read_text("1234a"), "Invalid digit 'a' for base 10");
  }
}

TEST_CASE("hexadecimal integer") {
  SUBCASE("simple lowercase") {
    CHECK(
        read_text("0x1a2b") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0x",
            .integer_digits = "1a2b",
        }
    );
  }

  SUBCASE("simple uppercase") {
    CHECK(
        read_text("0X1A2B") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0X",
            .integer_digits = "1A2B",
        }
    );
  }

  SUBCASE("hexadecimal integer with underscores") {
    CHECK(
        read_text("0x1a_2b_3c") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0x",
            .integer_digits = "1a_2b_3c",
        }
    );
  }

  SUBCASE("failure if hexadecimal integer contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0x1a__2b"), "Underscore must separate successive digits");
  }

  SUBCASE("okay if hexadecimal integer starts with underscore") {
    CHECK(
        read_text("0x_1a2b") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0x",
            .integer_digits = "_1a2b",
        }
    );
  }

  SUBCASE("failure if hexadecimal integer ends with underscore") {
    CHECK_THROWS_WITH(read_text("0x1a2b_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if hexadecimal integer contains non hexadecimal characters") {
    CHECK_THROWS_WITH(read_text("0x1a2g"), "Invalid character 'g' in number literal");
  }
}

TEST_CASE("binary integer") {
  SUBCASE("simple lowercase") {
    CHECK(
        read_text("0b1011") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0b",
            .integer_digits = "1011",
        }
    );
  }

  SUBCASE("simple uppercase") {
    CHECK(
        read_text("0B1011") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0B",
            .integer_digits = "1011",
        }
    );
  }

  SUBCASE("binary integer with underscores") {
    CHECK(
        read_text("0b10_11_00") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0b",
            .integer_digits = "10_11_00",
        }
    );
  }

  SUBCASE("failure if binary integer contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0b10__11"), "Underscore must separate successive digits");
  }

  SUBCASE("okay if binary integer starts with underscore") {
    CHECK(
        read_text("0b_1011") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0b",
            .integer_digits = "_1011",
        }
    );
  }

  SUBCASE("failure if binary integer ends with underscore") {
    CHECK_THROWS_WITH(read_text("0b1011_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if binary integer contains non binary characters") {
    CHECK_THROWS_WITH(read_text("0b1021"), "Invalid digit '2' for base 2");
  }
}

TEST_CASE("octal integer") {
  SUBCASE("simple lowercase") {
    CHECK(
        read_text("0o755") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0o",
            .integer_digits = "755",
        }
    );
  }

  SUBCASE("simple uppercase") {
    CHECK(
        read_text("0O755") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0O",
            .integer_digits = "755",
        }
    );
  }

  SUBCASE("simple assumed octal") {
    CHECK(
        read_text("0755") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0",
            .integer_digits = "755",
        }
    );
  }

  SUBCASE("octal integer with underscores") {
    CHECK(
        read_text("0o7_5_5") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0o",
            .integer_digits = "7_5_5",
        }
    );
  }

  SUBCASE("failure if octal integer contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0o7__55"), "Underscore must separate successive digits");
  }

  SUBCASE("okay if octal integer starts with underscore") {
    CHECK(
        read_text("0o_755") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0o",
            .integer_digits = "_755",
        }
    );
  }

  SUBCASE("failure if octal integer ends with underscore") {
    CHECK_THROWS_WITH(read_text("0o755_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if octal integer contains non octal characters") {
    CHECK_THROWS_WITH(read_text("0o7581"), "Invalid digit '8' for base 8");
  }

  SUBCASE("failure if assumed octal integer contains non octal characters") {
    CHECK_THROWS_WITH(read_text("0758"), "Invalid digit '8' for base 8");
  }

  SUBCASE("okay if assumed octal starts with underscore") {
    CHECK(
        read_text("0_755") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "0",
            .integer_digits = "_755",
        }
    );
  }

  SUBCASE("failure if assumed octal integer ends with underscore") {
    CHECK_THROWS_WITH(read_text("0755_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if octal integer has exponent") {
    CHECK_THROWS_WITH(read_text("0o755e2"), "Invalid digit 'e' for base 8");
  }

  SUBCASE("assumed octal integer with exponent will instead be parsed as a decimal float") {
    CHECK(
        read_text("0755e2") ==
        NumberLiteral{
            .has_decimal_point = false,
            .base_prefix = "",
            .integer_digits = "0755",
            .exponent_prefix = "e",
            .exponent_digits = "2",
        }
    );
  }
}

TEST_CASE("decimal floating point") {
  SUBCASE("simple") {
    CHECK(
        read_text("3.14") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "3",
            .fractional_digits = "14",
        }
    );
  }

  SUBCASE("with negative exponent") {
    CHECK(
        read_text("3.14e-2") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "3",
            .fractional_digits = "14",
            .exponent_prefix = "e",
            .exponent_sign = "-",
            .exponent_digits = "2",
        }
    );
  }

  SUBCASE("with positive exponent") {
    CHECK(
        read_text("3.14E+2") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "3",
            .fractional_digits = "14",
            .exponent_prefix = "E",
            .exponent_sign = "+",
            .exponent_digits = "2",
        }
    );
  }

  SUBCASE("with signless exponent") {
    CHECK(
        read_text("3.14e2") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "3",
            .fractional_digits = "14",
            .exponent_prefix = "e",
            .exponent_sign = "",
            .exponent_digits = "2",
        }
    );
  }

  SUBCASE("whole number float with positive exponent") {
    CHECK(
        read_text("1e+3") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "1",
            .exponent_prefix = "e",
            .exponent_sign = "+",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("whole number float with negative exponent") {
    CHECK(
        read_text("1e-3") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "1",
            .exponent_prefix = "e",
            .exponent_sign = "-",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("whole number float with signless exponent") {
    CHECK(
        read_text("1e3") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "1",
            .exponent_prefix = "e",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("float beginning with decimal point") {
    CHECK(
        read_text(".5") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "",
            .fractional_digits = "5",
        }
    );
  }

  SUBCASE("float ending with decimal point") {
    CHECK(
        read_text("2.") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "2",
            .fractional_digits = "",
        }
    );
  }

  SUBCASE("float with underscores") {
    CHECK(
        read_text("3_0.1_4e-2_0") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "3_0",
            .fractional_digits = "1_4",
            .exponent_prefix = "e",
            .exponent_sign = "-",
            .exponent_digits = "2_0",
        }
    );
  }

  SUBCASE("leading zeros with decimal point") {
    CHECK(
        read_text("03.14") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "03",
            .fractional_digits = "14",
        }
    );
  }

  SUBCASE("leading zeros with exponent") {
    CHECK(
        read_text("03e2") ==
        NumberLiteral{
            .has_decimal_point = false,
            .integer_digits = "03",
            .exponent_prefix = "e",
            .exponent_digits = "2",
        }
    );
  }

  SUBCASE("leading zeros with decimal point and exponent") {
    CHECK(
        read_text("03.14e2") ==
        NumberLiteral{
            .has_decimal_point = true,
            .integer_digits = "03",
            .fractional_digits = "14",
            .exponent_prefix = "e",
            .exponent_digits = "2",
        }
    );
  }

  SUBCASE("failure if whole number part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("3_.14"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if whole number part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("3__0.14"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part starts with an underscore") {
    CHECK_THROWS_WITH(read_text("3._14"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("3.14_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("3.1__4"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part starts with an underscore") {
    CHECK_THROWS_WITH(read_text("3.14e-_2"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("3.14e2_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("3.14e2__0"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent prefix is 'p' after whole number part") {
    CHECK_THROWS_WITH(
        read_text("3p2"), "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
    );
  }

  SUBCASE("failure if exponent prefix is 'P' after whole number part") {
    CHECK_THROWS_WITH(
        read_text("3P2"), "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
    );
  }

  SUBCASE("failure if exponent prefix is 'p' after fractional part") {
    CHECK_THROWS_WITH(
        read_text("3.14p2"), "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
    );
  }

  SUBCASE("failure if exponent prefix is 'P' after fractional part") {
    CHECK_THROWS_WITH(
        read_text("3.14P2"), "Only hexadecimal literals may use 'p' or 'P' as the exponent prefix"
    );
  }
}

TEST_CASE("hexadecimal floating point") {
  SUBCASE("simple") {
    CHECK(
        read_text("0xa.bp3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "a",
            .fractional_digits = "b",
            .exponent_prefix = "p",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("with negative exponent") {
    CHECK(
        read_text("0x1.2p-3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1",
            .fractional_digits = "2",
            .exponent_prefix = "p",
            .exponent_sign = "-",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("with positive exponent") {
    CHECK(
        read_text("0x1.2p+3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1",
            .fractional_digits = "2",
            .exponent_prefix = "p",
            .exponent_sign = "+",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("with underscores") {
    CHECK(
        read_text("0x1_0.2_0p-3_0") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1_0",
            .fractional_digits = "2_0",
            .exponent_prefix = "p",
            .exponent_sign = "-",
            .exponent_digits = "3_0",
        }
    );
  }

  SUBCASE("failure if whole number part contains non hexadecimal characters") {
    CHECK_THROWS_WITH(read_text("0x1g.2p3"), "Invalid character 'g' in number literal");
  }

  SUBCASE("failure if fractional part contains non hexadecimal characters") {
    CHECK_THROWS_WITH(read_text("0x1.2gp3"), "Invalid character 'g' in number literal");
  }

  SUBCASE("failure if exponent part contains non-decimal characters") {
    CHECK_THROWS_WITH(read_text("0x1.2pe3"), "Invalid character 'e' in exponent");
  }

  SUBCASE("failure if whole number part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("0x1_.2p3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if whole number part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0x1__0.2p3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part starts with an underscore") {
    CHECK_THROWS_WITH(read_text("0x1._2p3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("0x1.2_p3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if fractional part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0x1.2__0p3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part starts with an underscore") {
    CHECK_THROWS_WITH(read_text("0x1.2p-_3"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part ends with an underscore") {
    CHECK_THROWS_WITH(read_text("0x1.2p3_"), "Underscore must separate successive digits");
  }

  SUBCASE("failure if exponent part contains consecutive underscores") {
    CHECK_THROWS_WITH(read_text("0x1.2p3__0"), "Underscore must separate successive digits");
  }

  SUBCASE("'e' in whole number part is parsed as a digit") {
    CHECK(
        read_text("0x1e.2p3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1e",
            .fractional_digits = "2",
            .exponent_prefix = "p",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("'E' in whole number part is parsed as a digit") {
    CHECK(
        read_text("0x1E4.2p3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1E4",
            .fractional_digits = "2",
            .exponent_prefix = "p",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("'e' in fractional part is parsed as a digit") {
    CHECK(
        read_text("0x1.2e2p3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1",
            .fractional_digits = "2e2",
            .exponent_prefix = "p",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }

  SUBCASE("'E' in fractional part is parsed as a digit") {
    CHECK(
        read_text("0x1.2E2p3") ==
        NumberLiteral{
            .has_decimal_point = true,
            .base_prefix = "0x",
            .integer_digits = "1",
            .fractional_digits = "2E2",
            .exponent_prefix = "p",
            .exponent_sign = "",
            .exponent_digits = "3",
        }
    );
  }
}

TEST_SUITE_END();
