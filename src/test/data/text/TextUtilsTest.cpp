#include "data/text/TextUtils.h"
#include "Prelude.h"
#include <doctest.h>

TEST_SUITE_BEGIN("TextUtils");

using namespace amelia;

TEST_CASE("split - List<Text>") {
  List<Text> output;

  SUBCASE("basic split") {
    TextUtils::split(output, "a,b,c", ",");
    CHECK(output.size() == 3);
    CHECK(output[0] == "a");
    CHECK(output[1] == "b");
    CHECK(output[2] == "c");
  }

  SUBCASE("delimiter not found") {
    TextUtils::split(output, "abc", ",");
    CHECK(output.size() == 1);
    CHECK(output[0] == "abc");
  }

  SUBCASE("delimiter only") {
    TextUtils::split(output, ",", ",");
    CHECK(output.size() == 2);
    CHECK(output[0] == "");
    CHECK(output[1] == "");
  }

  SUBCASE("delimiter at start and end") {
    TextUtils::split(output, ",abc,", ",");
    CHECK(output.size() == 3);
    CHECK(output[0] == "");
    CHECK(output[1] == "abc");
    CHECK(output[2] == "");
  }

  SUBCASE("empty input") {
    TextUtils::split(output, "", ",");
    CHECK(output.size() == 1);
    CHECK(output[0] == "");
  }

  SUBCASE("empty delimiter") {
    TextUtils::split(output, "abc", "");
    CHECK(output.size() == 3);
    CHECK(output[0] == "a");
    CHECK(output[1] == "b");
    CHECK(output[2] == "c");
  }
}

TEST_CASE("split - List<String>") {
  List<String> output;

  SUBCASE("basic split") {
    TextUtils::split(output, "a,b,c", ",");
    CHECK(output.size() == 3);
    CHECK(output[0] == "a");
    CHECK(output[1] == "b");
    CHECK(output[2] == "c");
  }

  SUBCASE("delimiter not found") {
    TextUtils::split(output, "abc", ",");
    CHECK(output.size() == 1);
    CHECK(output[0] == "abc");
  }

  SUBCASE("delimiter only") {
    TextUtils::split(output, ",", ",");
    CHECK(output.size() == 2);
    CHECK(output[0] == "");
    CHECK(output[1] == "");
  }

  SUBCASE("delimiter at start and end") {
    TextUtils::split(output, ",abc,", ",");
    CHECK(output.size() == 3);
    CHECK(output[0] == "");
    CHECK(output[1] == "abc");
    CHECK(output[2] == "");
  }

  SUBCASE("empty input") {
    TextUtils::split(output, "", ",");
    CHECK(output.size() == 1);
    CHECK(output[0] == "");
  }

  SUBCASE("empty delimiter") {
    TextUtils::split(output, "abc", "");
    CHECK(output.size() == 3);
    CHECK(output[0] == "a");
    CHECK(output[1] == "b");
    CHECK(output[2] == "c");
  }
}

TEST_CASE("join_into") {
  String output;

  SUBCASE("basic join") {
    List<Text> parts = {"a", "b", "c"};
    TextUtils::join_into(output, parts, ",");
    CHECK(output == "a,b,c");
  }

  SUBCASE("single part") {
    List<Text> parts = {"abc"};
    TextUtils::join_into(output, parts, ",");
    CHECK(output == "abc");
  }

  SUBCASE("empty parts") {
    List<Text> parts;
    TextUtils::join_into(output, parts, ",");
    CHECK(output == "");
  }

  SUBCASE("empty delimiter") {
    List<Text> parts = {"a", "b", "c"};
    TextUtils::join_into(output, parts, "");
    CHECK(output == "abc");
  }
}

TEST_CASE("trim") {
  SUBCASE("basic trim") { CHECK(TextUtils::trim("  abc  \n\r\t") == "abc"); }

  SUBCASE("trim with custom chars") { CHECK(TextUtils::trim("--abc--", "-") == "abc"); }

  SUBCASE("no chars to trim") { CHECK(TextUtils::trim("abc") == "abc"); }

  SUBCASE("all chars trimmed") { CHECK(TextUtils::trim("   ", " ") == ""); }

  SUBCASE("empty input") { CHECK(TextUtils::trim("", " ") == ""); }

  SUBCASE("empty chars") { CHECK(TextUtils::trim("abc", "") == "abc"); }
}

TEST_CASE("trim_into") {
  String output;

  SUBCASE("basic trim") {
    TextUtils::trim_into(output, "  abc  \n\r\t");
    CHECK(output == "abc");
  }

  SUBCASE("trim with custom chars") {
    TextUtils::trim_into(output, "--abc--", "-");
    CHECK(output == "abc");
  }

  SUBCASE("no chars to trim") {
    TextUtils::trim_into(output, "abc");
    CHECK(output == "abc");
  }

  SUBCASE("all chars trimmed") {
    TextUtils::trim_into(output, "   ", " ");
    CHECK(output == "");
  }

  SUBCASE("empty input") {
    TextUtils::trim_into(output, "", " ");
    CHECK(output == "");
  }

  SUBCASE("empty chars") {
    TextUtils::trim_into(output, "abc", "");
    CHECK(output == "abc");
  }
}

TEST_CASE("trim_left") {
  SUBCASE("basic trim_left") { CHECK(TextUtils::trim_left("  abc  \n\r\t") == "abc  \n\r\t"); }

  SUBCASE("trim_left with custom chars") { CHECK(TextUtils::trim_left("--abc--", "-") == "abc--"); }

  SUBCASE("no chars to trim") { CHECK(TextUtils::trim_left("abc") == "abc"); }

  SUBCASE("all chars trimmed") { CHECK(TextUtils::trim_left("   ", " ") == ""); }

  SUBCASE("empty input") { CHECK(TextUtils::trim_left("", " ") == ""); }

  SUBCASE("empty chars") { CHECK(TextUtils::trim_left("abc", "") == "abc"); }
}

TEST_CASE("trim_left_into") {
  String output;

  SUBCASE("basic trim_left") {
    TextUtils::trim_left_into(output, "  abc  \n\r\t");
    CHECK(output == "abc  \n\r\t");
  }

  SUBCASE("trim_left with custom chars") {
    TextUtils::trim_left_into(output, "--abc--", "-");
    CHECK(output == "abc--");
  }

  SUBCASE("no chars to trim") {
    TextUtils::trim_left_into(output, "abc");
    CHECK(output == "abc");
  }

  SUBCASE("all chars trimmed") {
    TextUtils::trim_left_into(output, "   ", " ");
    CHECK(output == "");
  }

  SUBCASE("empty input") {
    TextUtils::trim_left_into(output, "", " ");
    CHECK(output == "");
  }

  SUBCASE("empty chars") {
    TextUtils::trim_left_into(output, "abc", "");
    CHECK(output == "abc");
  }
}

TEST_CASE("trim_right") {
  SUBCASE("basic trim_right") { CHECK(TextUtils::trim_right("  abc  \n\r\t") == "  abc"); }

  SUBCASE("trim_right with custom chars") {
    CHECK(TextUtils::trim_right("--abc--", "-") == "--abc");
  }

  SUBCASE("no chars to trim") { CHECK(TextUtils::trim_right("abc") == "abc"); }

  SUBCASE("all chars trimmed") { CHECK(TextUtils::trim_right("   ", " ") == ""); }

  SUBCASE("empty input") { CHECK(TextUtils::trim_right("", " ") == ""); }

  SUBCASE("empty chars") { CHECK(TextUtils::trim_right("abc", "") == "abc"); }
}

TEST_CASE("trim_right_into") {
  String output;

  SUBCASE("basic trim_right") {
    TextUtils::trim_right_into(output, "  abc  \n\r\t");
    CHECK(output == "  abc");
  }

  SUBCASE("trim_right with custom chars") {
    TextUtils::trim_right_into(output, "--abc--", "-");
    CHECK(output == "--abc");
  }

  SUBCASE("no chars to trim") {
    TextUtils::trim_right_into(output, "abc");
    CHECK(output == "abc");
  }

  SUBCASE("all chars trimmed") {
    TextUtils::trim_right_into(output, "   ", " ");
    CHECK(output == "");
  }

  SUBCASE("empty input") {
    TextUtils::trim_right_into(output, "", " ");
    CHECK(output == "");
  }

  SUBCASE("empty chars") {
    TextUtils::trim_right_into(output, "abc", "");
    CHECK(output == "abc");
  }
}

TEST_CASE("to_lower") {
  SUBCASE("basic to_lower") {
    String text = "AbC123!@#";
    TextUtils::to_lower(text);
    CHECK(text == "abc123!@#");
  }

  SUBCASE("already lowercase") {
    String text = "abc123!@#";
    TextUtils::to_lower(text);
    CHECK(text == "abc123!@#");
  }

  SUBCASE("empty string") {
    String text = "";
    TextUtils::to_lower(text);
    CHECK(text == "");
  }

  SUBCASE("non-ASCII characters") {
    String text = "HéLLo WörLD";
    TextUtils::to_lower(text);
    CHECK(text == "héllo wörld");
  }
}

TEST_CASE("to_lower_into") {
  String output;

  SUBCASE("basic to_lower_into") {
    TextUtils::to_lower_into(output, "AbC123!@#");
    CHECK(output == "abc123!@#");
  }

  SUBCASE("already lowercase") {
    TextUtils::to_lower_into(output, "abc123!@#");
    CHECK(output == "abc123!@#");
  }

  SUBCASE("empty string") {
    TextUtils::to_lower_into(output, "");
    CHECK(output == "");
  }

  SUBCASE("non-ASCII characters") {
    TextUtils::to_lower_into(output, "HéLLo WörLD");
    CHECK(output == "héllo wörld");
  }
}

TEST_CASE("to_upper") {
  SUBCASE("basic to_upper") {
    String text = "AbC123!@#";
    TextUtils::to_upper(text);
    CHECK(text == "ABC123!@#");
  }

  SUBCASE("already uppercase") {
    String text = "ABC123!@#";
    TextUtils::to_upper(text);
    CHECK(text == "ABC123!@#");
  }

  SUBCASE("empty string") {
    String text = "";
    TextUtils::to_upper(text);
    CHECK(text == "");
  }

  SUBCASE("non-ASCII characters") {
    String text = "HéLLo WörLD";
    TextUtils::to_upper(text);
    CHECK(text == "HéLLO WöRLD");
  }
}

TEST_CASE("to_upper_into") {
  String output;

  SUBCASE("basic to_upper_into") {
    TextUtils::to_upper_into(output, "AbC123!@#");
    CHECK(output == "ABC123!@#");
  }

  SUBCASE("already uppercase") {
    TextUtils::to_upper_into(output, "ABC123!@#");
    CHECK(output == "ABC123!@#");
  }

  SUBCASE("empty string") {
    TextUtils::to_upper_into(output, "");
    CHECK(output == "");
  }

  SUBCASE("non-ASCII characters") {
    TextUtils::to_upper_into(output, "HéLLo WörLD");
    CHECK(output == "HéLLO WöRLD");
  }
}

TEST_CASE("contains") {
  SUBCASE("substring present") { CHECK(TextUtils::contains("hello world", "world")); }

  SUBCASE("substring not present") { CHECK(!TextUtils::contains("hello world", "abc")); }

  SUBCASE("empty substring") { CHECK(TextUtils::contains("hello world", "")); }

  SUBCASE("empty input") { CHECK(!TextUtils::contains("", "abc")); }
}

TEST_CASE("starts_with") {
  SUBCASE("prefix present") { CHECK(TextUtils::starts_with("hello world", "hello")); }

  SUBCASE("prefix not present") { CHECK(!TextUtils::starts_with("hello world", "world")); }

  SUBCASE("empty prefix") { CHECK(TextUtils::starts_with("hello world", "")); }

  SUBCASE("empty input") { CHECK(!TextUtils::starts_with("", "abc")); }
}

TEST_CASE("ends_with") {
  SUBCASE("suffix present") { CHECK(TextUtils::ends_with("hello world", "world")); }

  SUBCASE("suffix not present") { CHECK(!TextUtils::ends_with("hello world", "hello")); }

  SUBCASE("empty suffix") { CHECK(TextUtils::ends_with("hello world", "")); }

  SUBCASE("empty input") { CHECK(!TextUtils::ends_with("", "abc")); }
}

TEST_CASE("find") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "world";
    CharIterator it = TextUtils::find(input, substring);
    CHECK(!it.at_end());
    CHECK(it.peek() == 'w');
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "abc";
    CharIterator it = TextUtils::find(input, substring);
    CHECK(it.at_end());
  }

  SUBCASE("empty substring") {
    Text input = "hello world";
    Text substring = "";
    CharIterator it = TextUtils::find(input, substring);
    CHECK(!it.at_end());
    CHECK(it.peek() == 'h');
  }

  SUBCASE("empty input") {
    Text input = "";
    Text substring = "abc";
    CharIterator it = TextUtils::find(input, substring);
    CHECK(it.at_end());
  }

  SUBCASE("unicode chars") {
    Text input = "héllo wörld";
    Text substring = "wör";
    CharIterator it = TextUtils::find(input, substring);
    CHECK(!it.at_end());
    CHECK(it.peek() == 'w');
  }
}

TEST_CASE("find_byte") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "world";
    int64_t byte_index = TextUtils::find_byte(input, substring);
    CHECK(byte_index == 6);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "abc";
    int64_t byte_index = TextUtils::find_byte(input, substring);
    CHECK(byte_index == -1);
  }

  SUBCASE("empty substring") {
    Text input = "hello world";
    Text substring = "";
    int64_t byte_index = TextUtils::find_byte(input, substring);
    CHECK(byte_index == 0);
  }

  SUBCASE("empty input") {
    Text input = "";
    Text substring = "abc";
    int64_t byte_index = TextUtils::find_byte(input, substring);
    CHECK(byte_index == -1);
  }

  SUBCASE("unicode chars") {
    Text input = "héllo wörld";
    Text substring = "wör";
    int64_t byte_index = TextUtils::find_byte(input, substring);
    CHECK(byte_index == 7);
  }
}

TEST_CASE("find_after") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "o";
    CharIterator it = TextUtils::find_after(input, substring, input.begin());
    CHECK(!it.at_end());
    CHECK(it.data().ptr() - input.data().ptr() == 4);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "helllo";
    CharIterator it = TextUtils::find_after(input, substring, input.begin());
    CHECK(it.at_end());
  }

  SUBCASE("substring not present after iterator") {
    Text input = "hello world";
    Text substring = "hello";
    CharIterator it = TextUtils::find_after(input, substring, input.begin().plus(5));
    CHECK(it.at_end());
  }
}

TEST_CASE("find_after_byte") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "o";
    int64_t byte_index = TextUtils::find_after_byte(input, substring, 0);
    CHECK(byte_index == 4);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "helllo";
    int64_t byte_index = TextUtils::find_after_byte(input, substring, 0);
    CHECK(byte_index == -1);
  }

  SUBCASE("substring not present after byte index") {
    Text input = "hello world";
    Text substring = "hello";
    int64_t byte_index = TextUtils::find_after_byte(input, substring, 5);
    CHECK(byte_index == -1);
  }
}

TEST_CASE("find_before") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "o";
    CharIterator it = TextUtils::find_before(input, substring, input.end());
    CHECK(!it.at_end());
    CHECK(it.data().ptr() - input.data().ptr() == 7);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "helllo";
    CharIterator it = TextUtils::find_before(input, substring, input.end());
    CHECK(it.at_end());
  }

  SUBCASE("substring not present before iterator") {
    Text input = "hello world";
    Text substring = "world";
    CharIterator it = TextUtils::find_before(input, substring, input.begin().plus(10));
    CHECK(it.at_end());
  }

  SUBCASE("empty input") {
    Text input = "";
    Text substring = "abc";
    CharIterator it = TextUtils::find_before(input, substring, input.end());
    CHECK(it.at_end());
  }

  SUBCASE("empty substring") {
    Text input = "hello world";
    Text substring = "";
    CharIterator it = TextUtils::find_before(input, substring, input.begin().plus(5));
    CHECK(!it.at_end());
    CHECK(it.data().ptr() - input.data().ptr() == 5);
  }
}

TEST_CASE("find_before_byte") {
  SUBCASE("substring present") {
    Text input = "hello world";
    Text substring = "o";
    int64_t byte_index = TextUtils::find_before_byte(input, substring, input.size());
    CHECK(byte_index == 7);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "helllo";
    int64_t byte_index = TextUtils::find_before_byte(input, substring, input.size());
    CHECK(byte_index == -1);
  }

  SUBCASE("substring not present before byte index") {
    Text input = "hello world";
    Text substring = "world";
    int64_t byte_index = TextUtils::find_before_byte(input, substring, 10);
    CHECK(byte_index == -1);
  }

  SUBCASE("empty input") {
    Text input = "";
    Text substring = "abc";
    int64_t byte_index = TextUtils::find_before_byte(input, substring, 0);
    CHECK(byte_index == -1);
  }

  SUBCASE("empty substring") {
    Text input = "hello world";
    Text substring = "";
    int64_t byte_index = TextUtils::find_before_byte(input, substring, 5);
    CHECK(byte_index == 5);
  }
}

TEST_CASE("find_char") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    CharIterator it = TextUtils::find_char(input, code_point);
    CHECK(!it.at_end());
    CHECK(it.peek() == 'o');
  }

  SUBCASE("char present multiple times") {
    Text input = "hello world";
    uint32_t code_point = 'l';
    CharIterator it = TextUtils::find_char(input, code_point);
    CHECK(!it.at_end());
    CHECK(it.peek() == 'l');
    ++it;
    CHECK(!it.at_end());
    CHECK(it.peek() == 'l');
  }

  SUBCASE("unicode char present") {
    Text input = "héllo wörld";
    CharIterator it = TextUtils::find_char(input, U'ö');
    CHECK(!it.at_end());
    CHECK(it.peek() == U'ö');
  }

  SUBCASE("char not present") {
    Text input = "hello world";
    uint32_t code_point = 'x';
    CharIterator it = TextUtils::find_char(input, code_point);
    CHECK(it.at_end());
  }

  SUBCASE("empty input") {
    Text input = "";
    uint32_t code_point = 'a';
    CharIterator it = TextUtils::find_char(input, code_point);
    CHECK(it.at_end());
  }
}

TEST_CASE("find_char_byte") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    int64_t byte_index = TextUtils::find_char_byte(input, code_point);
    CHECK(byte_index == 4);
  }

  SUBCASE("char present multiple times") {
    Text input = "hello world";
    uint32_t code_point = 'l';
    int64_t byte_index = TextUtils::find_char_byte(input, code_point);
    CHECK(byte_index == 2);
  }

  SUBCASE("unicode char present") {
    Text input = "hello wörld";
    int64_t byte_index = TextUtils::find_char_byte(input, U'ö');
    CHECK(byte_index == 7);
  }

  SUBCASE("char not present") {
    Text input = "hello world";
    uint32_t code_point = 'x';
    int64_t byte_index = TextUtils::find_char_byte(input, code_point);
    CHECK(byte_index == -1);
  }

  SUBCASE("empty input") {
    Text input = "";
    uint32_t code_point = 'a';
    int64_t byte_index = TextUtils::find_char_byte(input, code_point);
    CHECK(byte_index == -1);
  }
}

TEST_CASE("find_char_after") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    CharIterator it = TextUtils::find_char_after(input, code_point, input.begin());
    CHECK(!it.at_end());
    CHECK(it.peek() == 'o');
  }

  SUBCASE("char present after iterator") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    CharIterator it = TextUtils::find_char_after(input, code_point, input.begin().plus(5));
    CHECK(!it.at_end());
    CHECK(it.peek() == 'o');
  }

  SUBCASE("char not present after iterator") {
    Text input = "hello world";
    uint32_t code_point = 'h';
    CharIterator it = TextUtils::find_char_after(input, code_point, input.begin().plus(1));
    CHECK(it.at_end());
  }
}

TEST_CASE("find_char_after_byte") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    int64_t byte_index = TextUtils::find_char_after_byte(input, code_point, 0);
    CHECK(byte_index == 4);
  }

  SUBCASE("char present after byte index") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    int64_t byte_index = TextUtils::find_char_after_byte(input, code_point, 5);
    CHECK(byte_index == 7);
  }

  SUBCASE("char not present after byte index") {
    Text input = "hello world";
    uint32_t code_point = 'h';
    int64_t byte_index = TextUtils::find_char_after_byte(input, code_point, 1);
    CHECK(byte_index == -1);
  }
}

TEST_CASE("find_char_before") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    CharIterator it = TextUtils::find_char_before(input, code_point, input.end());
    CHECK(!it.at_end());
    CHECK(it.peek() == 'o');
    // should have found the second 'o', not the first one
    CHECK(it.data().ptr() - input.data().ptr() == 7);
  }

  SUBCASE("char present before iterator") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    CharIterator it = TextUtils::find_char_before(input, code_point, input.begin().plus(5));
    CHECK(!it.at_end());
    CHECK(it.peek() == 'o');
    // should have found the first 'o', not the second one
    CHECK(it.data().ptr() - input.data().ptr() == 4);
  }

  SUBCASE("char not present before iterator") {
    Text input = "hello world";
    uint32_t code_point = 'd';
    CharIterator it = TextUtils::find_char_before(input, code_point, input.begin().plus(10));
    CHECK(it.at_end());
  }
}

TEST_CASE("find_char_before_byte") {
  SUBCASE("char present") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    int64_t byte_index = TextUtils::find_char_before_byte(input, code_point, input.size());
    CHECK(byte_index == 7);
  }

  SUBCASE("char present before byte index") {
    Text input = "hello world";
    uint32_t code_point = 'o';
    int64_t byte_index = TextUtils::find_char_before_byte(input, code_point, 5);
    CHECK(byte_index == 4);
  }

  SUBCASE("char not present before byte index") {
    Text input = "hello world";
    uint32_t code_point = 'e';
    int64_t byte_index = TextUtils::find_char_before_byte(input, code_point, 1);
    CHECK(byte_index == -1);
  }
}

TEST_CASE("find_all") {
  SUBCASE("substring present once") {
    Text input = "hello world";
    Text substring = "world";
    List<CharIterator> iters;
    TextUtils::find_all(iters, input, substring);
    CHECK(iters.size() == 1);
    CHECK(iters[0].data().ptr() - input.data().ptr() == 6);
  }

  SUBCASE("substring present multiple times") {
    Text input = "hello world, hello everyone";
    Text substring = "hello";
    List<CharIterator> iters;
    TextUtils::find_all(iters, input, substring);
    CHECK(iters.size() == 2);
    CHECK(iters[0].data().ptr() - input.data().ptr() == 0);
    CHECK(iters[1].data().ptr() - input.data().ptr() == 13);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "abc";
    List<CharIterator> iters;
    TextUtils::find_all(iters, input, substring);
    CHECK(iters.size() == 0);
  }

  SUBCASE("empty substring") {
    Text input = "abc";
    Text substring = "";
    List<CharIterator> iters;
    TextUtils::find_all(iters, input, substring);
    CHECK(iters.size() == 3);
    CHECK(iters[0].data().ptr() - input.data().ptr() == 0);
    CHECK(iters[1].data().ptr() - input.data().ptr() == 1);
    CHECK(iters[2].data().ptr() - input.data().ptr() == 2);
  }
}

TEST_CASE("find_all_bytes") {
  SUBCASE("substring present once") {
    Text input = "hello world";
    Text substring = "world";
    List<int64_t> byte_indices;
    TextUtils::find_all_bytes(byte_indices, input, substring);
    CHECK(byte_indices.size() == 1);
    CHECK(byte_indices[0] == 6);
  }

  SUBCASE("substring present multiple times") {
    Text input = "hello world, hello everyone";
    Text substring = "hello";
    List<int64_t> byte_indices;
    TextUtils::find_all_bytes(byte_indices, input, substring);
    CHECK(byte_indices.size() == 2);
    CHECK(byte_indices[0] == 0);
    CHECK(byte_indices[1] == 13);
  }

  SUBCASE("substring not present") {
    Text input = "hello world";
    Text substring = "abc";
    List<int64_t> byte_indices;
    TextUtils::find_all_bytes(byte_indices, input, substring);
    CHECK(byte_indices.size() == 0);
  }

  SUBCASE("empty substring") {
    Text input = "abc";
    Text substring = "";
    List<int64_t> byte_indices;
    TextUtils::find_all_bytes(byte_indices, input, substring);
    CHECK(byte_indices.size() == 3);
    CHECK(byte_indices[0] == 0);
    CHECK(byte_indices[1] == 1);
    CHECK(byte_indices[2] == 2);
  }
}

TEST_CASE("find_all_chars") {
  SUBCASE("char present once") {
    Text input = "hello world";
    uint32_t code_point = 'w';
    List<CharIterator> iters;
    TextUtils::find_all_chars(iters, input, code_point);
    CHECK(iters.size() == 1);
    CHECK(iters[0].peek() == 'w');
  }

  SUBCASE("char present multiple times") {
    Text input = "hello world";
    uint32_t code_point = 'l';
    List<CharIterator> iters;
    TextUtils::find_all_chars(iters, input, code_point);
    CHECK(iters.size() == 3);
    CHECK(iters[0].data().ptr() - input.data().ptr() == 2);
    CHECK(iters[1].data().ptr() - input.data().ptr() == 3);
    CHECK(iters[2].data().ptr() - input.data().ptr() == 9);
  }

  SUBCASE("char not present") {
    Text input = "hello world";
    uint32_t code_point = 'x';
    List<CharIterator> iters;
    TextUtils::find_all_chars(iters, input, code_point);
    CHECK(iters.size() == 0);
  }
}

TEST_CASE("find_all_char_bytes") {
  SUBCASE("char present once") {
    Text input = "hello world";
    uint32_t code_point = 'w';
    List<int64_t> byte_indices;
    TextUtils::find_all_char_bytes(byte_indices, input, code_point);
    CHECK(byte_indices.size() == 1);
    CHECK(byte_indices[0] == 6);
  }

  SUBCASE("char present multiple times") {
    Text input = "hello world";
    uint32_t code_point = 'l';
    List<int64_t> byte_indices;
    TextUtils::find_all_char_bytes(byte_indices, input, code_point);
    CHECK(byte_indices.size() == 3);
    CHECK(byte_indices[0] == 2);
    CHECK(byte_indices[1] == 3);
    CHECK(byte_indices[2] == 9);
  }

  SUBCASE("char not present") {
    Text input = "hello world";
    uint32_t code_point = 'x';
    List<int64_t> byte_indices;
    TextUtils::find_all_char_bytes(byte_indices, input, code_point);
    CHECK(byte_indices.size() == 0);
  }
}

TEST_CASE("replace") {
  SUBCASE("basic replace with limit") {
    String input = "hello world, hello everyone";
    TextUtils::replace(input, "hello", "hi", 1);
    CHECK(input == "hi world, hello everyone");
  }

  SUBCASE("replace with limit more than occurrences") {
    String input = "hello world, hello everyone";
    TextUtils::replace(input, "hello", "hi", 5);
    CHECK(input == "hi world, hi everyone");
  }

  SUBCASE("replace with limit zero") {
    String input = "hello world, hello everyone";
    TextUtils::replace(input, "hello", "hi", 0);
    CHECK(input == "hello world, hello everyone");
  }

  SUBCASE("replace with limit negative") {
    String input = "hello world, hello everyone";
    TextUtils::replace(input, "hello", "hi", -1);
    CHECK(input == "hi world, hi everyone");
  }
}

TEST_CASE("replace_into") {
  String output;

  SUBCASE("basic replace_into") {
    TextUtils::replace_into(output, "hello world, hello everyone", "hello", "hi", 1);
    CHECK(output == "hi world, hello everyone");
  }

  SUBCASE("replace_into with limit more than occurrences") {
    TextUtils::replace_into(output, "hello world, hello everyone", "hello", "hi", 5);
    CHECK(output == "hi world, hi everyone");
  }

  SUBCASE("replace_into with limit zero") {
    TextUtils::replace_into(output, "hello world, hello everyone", "hello", "hi", 0);
    CHECK(output == "hello world, hello everyone");
  }

  SUBCASE("replace_into with limit negative") {
    TextUtils::replace_into(output, "hello world, hello everyone", "hello", "hi", -1);
    CHECK(output == "hi world, hi everyone");
  }
}

TEST_CASE("replace_all") {
  SUBCASE("basic replace_all") {
    String input = "hello world";
    TextUtils::replace_all(input, "world", "everyone");
    CHECK(input == "hello everyone");
  }

  SUBCASE("replace_all with empty string") {
    String input = "hello world";
    TextUtils::replace_all(input, "world", "");
    CHECK(input == "hello ");
  }

  SUBCASE("replace_all empty string") {
    String input = "abc";
    TextUtils::replace_all(input, "", "-");
    CHECK(input == "-a-b-c-");
  }

  SUBCASE("no occurrences to replace_all") {
    String input = "hello world";
    TextUtils::replace_all(input, "abc", "xyz");
    CHECK(input == "hello world");
  }
}

TEST_CASE("replace_all_into") {
  String output;

  SUBCASE("basic replace_all_into") {
    TextUtils::replace_all_into(output, "hello world", "world", "everyone");
    CHECK(output == "hello everyone");
  }

  SUBCASE("replace_all_into with empty string") {
    TextUtils::replace_all_into(output, "hello world", "world", "");
    CHECK(output == "hello ");
  }

  SUBCASE("replace_all_into empty string") {
    TextUtils::replace_all_into(output, "abc", "", "-");
    CHECK(output == "-a-b-c-");
  }

  SUBCASE("no occurrences to replace_all_into") {
    TextUtils::replace_all_into(output, "hello world", "abc", "xyz");
    CHECK(output == "hello world");
  }
}

TEST_CASE("pad_left") {
  SUBCASE("basic pad_left") {
    String text = "abc";
    TextUtils::pad_left(text, 5, "x");
    CHECK(text == "xxabc");
  }

  SUBCASE("pad_left with width less than text length") {
    String text = "abc";
    TextUtils::pad_left(text, 2, "x");
    CHECK(text == "abc");
  }

  SUBCASE("pad_left with width equal to text length") {
    String text = "abc";
    TextUtils::pad_left(text, 3, "x");
    CHECK(text == "abc");
  }

  SUBCASE("pad_left with empty string") {
    String text = "";
    TextUtils::pad_left(text, 4, "x");
    CHECK(text == "xxxx");
  }

  SUBCASE("pad_left with empty padding string") {
    String text = "abc";
    TextUtils::pad_left(text, 5, "");
    CHECK(text == "abc");
  }

  SUBCASE("pad_left with multi-character padding string") {
    String text = "abc";
    TextUtils::pad_left(text, 6, "xy");
    CHECK(text == "xyxyabc");
  }
}

TEST_CASE("pad_left_into") {
  String output;

  SUBCASE("basic pad_left_into") {
    TextUtils::pad_left_into(output, "abc", 5, "x");
    CHECK(output == "xxabc");
  }

  SUBCASE("pad_left_into with width less than text length") {
    TextUtils::pad_left_into(output, "abc", 2, "x");
    CHECK(output == "abc");
  }

  SUBCASE("pad_left_into with width equal to text length") {
    TextUtils::pad_left_into(output, "abc", 3, "x");
    CHECK(output == "abc");
  }

  SUBCASE("pad_left_into with empty string") {
    TextUtils::pad_left_into(output, "", 4, "x");
    CHECK(output == "xxxx");
  }

  SUBCASE("pad_left_into with empty padding string") {
    TextUtils::pad_left_into(output, "abc", 5, "");
    CHECK(output == "abc");
  }

  SUBCASE("pad_left_into with multi-character padding string") {
    TextUtils::pad_left_into(output, "abc", 6, "xy");
    CHECK(output == "xyxyabc");
  }
}

TEST_CASE("pad_right") {
  SUBCASE("basic pad_right") {
    String text = "abc";
    TextUtils::pad_right(text, 5, "x");
    CHECK(text == "abcxx");
  }

  SUBCASE("pad_right with width less than text length") {
    String text = "abc";
    TextUtils::pad_right(text, 2, "x");
    CHECK(text == "abc");
  }

  SUBCASE("pad_right with width equal to text length") {
    String text = "abc";
    TextUtils::pad_right(text, 3, "x");
    CHECK(text == "abc");
  }

  SUBCASE("pad_right with empty string") {
    String text = "";
    TextUtils::pad_right(text, 4, "x");
    CHECK(text == "xxxx");
  }

  SUBCASE("pad_right with empty padding string") {
    String text = "abc";
    TextUtils::pad_right(text, 5, "");
    CHECK(text == "abc");
  }

  SUBCASE("pad_right with multi-character padding string") {
    String text = "abc";
    TextUtils::pad_right(text, 6, "xy");
    CHECK(text == "abcxyxy");
  }
}

TEST_CASE("pad_right_into") {
  String output;

  SUBCASE("basic pad_right_into") {
    TextUtils::pad_right_into(output, "abc", 5, "x");
    CHECK(output == "abcxx");
  }

  SUBCASE("pad_right_into with width less than text length") {
    TextUtils::pad_right_into(output, "abc", 2, "x");
    CHECK(output == "abc");
  }

  SUBCASE("pad_right_into with width equal to text length") {
    TextUtils::pad_right_into(output, "abc", 3, "x");
    CHECK(output == "abc");
  }

  SUBCASE("pad_right_into with empty string") {
    TextUtils::pad_right_into(output, "", 4, "x");
    CHECK(output == "xxxx");
  }

  SUBCASE("pad_right_into with empty padding string") {
    TextUtils::pad_right_into(output, "abc", 5, "");
    CHECK(output == "abc");
  }

  SUBCASE("pad_right_into with multi-character padding string") {
    TextUtils::pad_right_into(output, "abc", 6, "xy");
    CHECK(output == "abcxyxy");
  }
}

TEST_CASE("remove") {
  SUBCASE("basic remove with limit") {
    String input = "hello world, hello everyone";
    TextUtils::remove(input, "hello", 1);
    CHECK(input == " world, hello everyone");
  }

  SUBCASE("remove with limit more than occurrences") {
    String input = "hello world, hello everyone";
    TextUtils::remove(input, "hello", 5);
    CHECK(input == " world,  everyone");
  }

  SUBCASE("remove with limit zero") {
    String input = "hello world, hello everyone";
    TextUtils::remove(input, "hello", 0);
    CHECK(input == "hello world, hello everyone");
  }

  SUBCASE("remove with limit negative") {
    String input = "hello world, hello everyone";
    TextUtils::remove(input, "hello", -1);
    CHECK(input == " world,  everyone");
  }
}

TEST_CASE("remove_into") {
  String output;

  SUBCASE("basic remove_into") {
    TextUtils::remove_into(output, "hello world, hello everyone", "hello", 1);
    CHECK(output == " world, hello everyone");
  }

  SUBCASE("remove_into with limit more than occurrences") {
    TextUtils::remove_into(output, "hello world, hello everyone", "hello", 5);
    CHECK(output == " world,  everyone");
  }

  SUBCASE("remove_into with limit zero") {
    TextUtils::remove_into(output, "hello world, hello everyone", "hello", 0);
    CHECK(output == "hello world, hello everyone");
  }

  SUBCASE("remove_into with limit negative") {
    TextUtils::remove_into(output, "hello world, hello everyone", "hello", -1);
    CHECK(output == " world,  everyone");
  }
}

TEST_CASE("remove_all") {
  SUBCASE("basic remove_all") {
    String input = "hello world";
    TextUtils::remove_all(input, "world");
    CHECK(input == "hello ");
  }

  SUBCASE("remove_all with empty string") {
    String input = "hello world";
    TextUtils::remove_all(input, "");
    CHECK(input == "hello world");
  }

  SUBCASE("remove_all with no occurrences") {
    String input = "hello world";
    TextUtils::remove_all(input, "abc");
    CHECK(input == "hello world");
  }
}

TEST_CASE("remove_all_into") {
  String output;

  SUBCASE("basic remove_all_into") {
    TextUtils::remove_all_into(output, "hello world", "world");
    CHECK(output == "hello ");
  }

  SUBCASE("remove_all_into with empty string") {
    TextUtils::remove_all_into(output, "hello world", "");
    CHECK(output == "hello world");
  }

  SUBCASE("remove_all_into with no occurrences") {
    TextUtils::remove_all_into(output, "hello world", "abc");
    CHECK(output == "hello world");
  }
}

TEST_CASE("repeat") {
  SUBCASE("basic repeat") {
    String text = "abc";
    TextUtils::repeat(text, 3);
    CHECK(text == "abcabcabc");
  }

  SUBCASE("repeat with count zero") {
    String text = "abc";
    TextUtils::repeat(text, 0);
    CHECK(text == "");
  }

  SUBCASE("repeat with count one") {
    String text = "abc";
    TextUtils::repeat(text, 1);
    CHECK(text == "abc");
  }
}

TEST_CASE("repeat_into") {
  String output;

  SUBCASE("basic repeat_into") {
    TextUtils::repeat_into(output, "abc", 3);
    CHECK(output == "abcabcabc");
  }

  SUBCASE("repeat_into with count zero") {
    TextUtils::repeat_into(output, "abc", 0);
    CHECK(output == "");
  }

  SUBCASE("repeat_into with count one") {
    TextUtils::repeat_into(output, "abc", 1);
    CHECK(output == "abc");
  }
}

TEST_CASE("reverse") {
  SUBCASE("basic reverse") {
    String text = "hello";
    TextUtils::reverse(text);
    CHECK(text == "olleh");
  }

  SUBCASE("reverse with unicode") {
    String text = "héllo";
    TextUtils::reverse(text);
    CHECK(text == "olléh");
  }

  SUBCASE("reverse empty string") {
    String text = "";
    TextUtils::reverse(text);
    CHECK(text == "");
  }
}

TEST_CASE("reverse_into") {
  String output;

  SUBCASE("basic reverse_into") {
    TextUtils::reverse_into(output, "hello");
    CHECK(output == "olleh");
  }

  SUBCASE("reverse_into with unicode") {
    TextUtils::reverse_into(output, "héllo");
    CHECK(output == "olléh");
  }

  SUBCASE("reverse_into empty string") {
    TextUtils::reverse_into(output, "");
    CHECK(output == "");
  }
}

TEST_CASE("strip_prefix") {
  SUBCASE("basic strip_prefix") {
    CHECK(TextUtils::strip_prefix("hello world", "hello ") == "world");
  }

  SUBCASE("strip_prefix with non-matching prefix") {
    CHECK(TextUtils::strip_prefix("hello world", "world") == "hello world");
  }

  SUBCASE("strip_prefix with empty prefix") {
    CHECK(TextUtils::strip_prefix("hello world", "") == "hello world");
  }

  SUBCASE("strip_prefix with prefix equal to text") {
    CHECK(TextUtils::strip_prefix("hello", "hello") == "");
  }
}

TEST_CASE("strip_suffix") {
  SUBCASE("basic strip_suffix") {
    CHECK(TextUtils::strip_suffix("hello world", " world") == "hello");
  }

  SUBCASE("strip_suffix with non-matching suffix") {
    CHECK(TextUtils::strip_suffix("hello world", "hello") == "hello world");
  }

  SUBCASE("strip_suffix with empty suffix") {
    CHECK(TextUtils::strip_suffix("hello world", "") == "hello world");
  }

  SUBCASE("strip_suffix with suffix equal to text") {
    CHECK(TextUtils::strip_suffix("world", "world") == "");
  }
}

TEST_CASE("slice") {
  SUBCASE("basic slice") {
    Text input = "hello world";
    CharIterator world = input.begin().plus(6);
    Text slice = TextUtils::slice(input, world, 5);
    CHECK(slice == "world");
  }

  SUBCASE("slice with length zero") {
    Text input = "hello world";
    CharIterator it = input.begin().plus(6);
    Text slice = TextUtils::slice(input, it, 0);
    CHECK(slice == "");
  }

  SUBCASE("slice with length greater than remaining") {
    Text input = "hello world";
    CharIterator it = input.begin().plus(6);
    CHECK_THROWS_AS(TextUtils::slice(input, it, 10), RuntimeError);
  }
}

TEST_CASE("slice_bytes") {
  SUBCASE("basic slice_bytes") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text slice = TextUtils::slice_bytes(input, byte_index, 5);
    CHECK(slice == "world");
  }

  SUBCASE("slice_bytes with length zero") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text slice = TextUtils::slice_bytes(input, byte_index, 0);
    CHECK(slice == "");
  }

  SUBCASE("slice_bytes with length greater than remaining") {
    Text input = "hello world";
    int64_t byte_index = 6;
    CHECK_THROWS_AS(TextUtils::slice_bytes(input, byte_index, 10), RuntimeError);
  }
}

TEST_CASE("substr") {
  SUBCASE("basic substr") {
    Text input = "hello world";
    CharIterator world = input.begin().plus(6);
    Text substr = TextUtils::substr(input, world, input.end());
    CHECK(substr == "world");
  }

  SUBCASE("substr with empty range") {
    Text input = "hello world";
    CharIterator it = input.begin().plus(6);
    Text substr = TextUtils::substr(input, it, it);
    CHECK(substr == "");
  }

  SUBCASE("substr with end before start") {
    Text input = "hello world";
    CharIterator start = input.begin().plus(6);
    CharIterator end = input.begin().plus(5);
    CHECK_THROWS_AS(TextUtils::substr(input, start, end), RuntimeError);
  }
}

TEST_CASE("substr_bytes") {
  SUBCASE("basic substr_bytes") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text substr = TextUtils::substr_bytes(input, byte_index, input.size());
    CHECK(substr == "world");
  }

  SUBCASE("substr_bytes with empty range") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text substr = TextUtils::substr_bytes(input, byte_index, byte_index);
    CHECK(substr == "");
  }

  SUBCASE("substr_bytes with end before start") {
    Text input = "hello world";
    int64_t start_byte_index = 6;
    int64_t end_byte_index = 5;
    CHECK_THROWS_AS(TextUtils::substr_bytes(input, start_byte_index, end_byte_index), RuntimeError);
  }
}

TEST_CASE("head") {
  SUBCASE("basic head") {
    Text input = "hello world";
    CharIterator world = input.begin().plus(6);
    Text head = TextUtils::head(input, world);
    CHECK(head == "hello ");
  }

  SUBCASE("head with iterator at beginning") {
    Text input = "hello world";
    CharIterator it = input.begin();
    Text head = TextUtils::head(input, it);
    CHECK(head == "");
  }

  SUBCASE("head with iterator at end") {
    Text input = "hello world";
    CharIterator it = input.end();
    Text head = TextUtils::head(input, it);
    CHECK(head == "hello world");
  }
}

TEST_CASE("head_bytes") {
  SUBCASE("basic head_bytes") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text head = TextUtils::head_bytes(input, byte_index);
    CHECK(head == "hello ");
  }

  SUBCASE("head_bytes with byte index at beginning") {
    Text input = "hello world";
    int64_t byte_index = 0;
    Text head = TextUtils::head_bytes(input, byte_index);
    CHECK(head == "");
  }

  SUBCASE("head_bytes with byte index at end") {
    Text input = "hello world";
    int64_t byte_index = input.size();
    Text head = TextUtils::head_bytes(input, byte_index);
    CHECK(head == "hello world");
  }
}

TEST_CASE("tail") {
  SUBCASE("basic tail") {
    Text input = "hello world";
    CharIterator world = input.begin().plus(6);
    Text tail = TextUtils::tail(input, world);
    CHECK(tail == "world");
  }

  SUBCASE("tail with iterator at beginning") {
    Text input = "hello world";
    CharIterator it = input.begin();
    Text tail = TextUtils::tail(input, it);
    CHECK(tail == "hello world");
  }

  SUBCASE("tail with iterator at end") {
    Text input = "hello world";
    CharIterator it = input.end();
    Text tail = TextUtils::tail(input, it);
    CHECK(tail == "");
  }
}

TEST_CASE("tail_bytes") {
  SUBCASE("basic tail_bytes") {
    Text input = "hello world";
    int64_t byte_index = 6;
    Text tail = TextUtils::tail_bytes(input, byte_index);
    CHECK(tail == "world");
  }

  SUBCASE("tail_bytes with byte index at beginning") {
    Text input = "hello world";
    int64_t byte_index = 0;
    Text tail = TextUtils::tail_bytes(input, byte_index);
    CHECK(tail == "hello world");
  }

  SUBCASE("tail_bytes with byte index at end") {
    Text input = "hello world";
    int64_t byte_index = input.size();
    Text tail = TextUtils::tail_bytes(input, byte_index);
    CHECK(tail == "");
  }
}

TEST_CASE("is_digit - code point") {
  SUBCASE("basic is_digit") {
    CHECK(TextUtils::is_digit('0'));
    CHECK(TextUtils::is_digit('5'));
    CHECK(TextUtils::is_digit('9'));
  }

  SUBCASE("is_digit with non-digit characters") {
    CHECK(!TextUtils::is_digit('a'));
    CHECK(!TextUtils::is_digit(' '));
    CHECK(!TextUtils::is_digit('-'));
  }
}

TEST_CASE("is_digit - Text") {
  SUBCASE("basic is_digit") { CHECK(TextUtils::is_digit("12345")); }

  SUBCASE("is_digit with non-digit characters") {
    CHECK(!TextUtils::is_digit("123a5"));
    CHECK(!TextUtils::is_digit(" "));
    CHECK(!TextUtils::is_digit("-123"));
  }

  SUBCASE("is_digit with empty string") { CHECK(!TextUtils::is_digit("")); }
}

TEST_CASE("is_alpha - code point") {
  SUBCASE("basic is_alpha") {
    CHECK(TextUtils::is_alpha('a'));
    CHECK(TextUtils::is_alpha('Z'));
    CHECK(TextUtils::is_alpha('m'));
  }

  SUBCASE("is_alpha with non-alpha characters") {
    CHECK(!TextUtils::is_alpha('1'));
    CHECK(!TextUtils::is_alpha(' '));
    CHECK(!TextUtils::is_alpha('-'));
  }
}

TEST_CASE("is_alpha - Text") {
  SUBCASE("basic is_alpha") {
    CHECK(TextUtils::is_alpha("Hello"));
    CHECK(TextUtils::is_alpha("World"));
    CHECK(TextUtils::is_alpha("Test"));
  }

  SUBCASE("is_alpha with non-alpha characters") {
    CHECK(!TextUtils::is_alpha("Hello123"));
    CHECK(!TextUtils::is_alpha(" "));
    CHECK(!TextUtils::is_alpha("-Test"));
  }

  SUBCASE("is_alpha with empty string") { CHECK(!TextUtils::is_alpha("")); }
}

TEST_CASE("is_alnum - code point") {
  SUBCASE("basic is_alnum") {
    CHECK(TextUtils::is_alnum('a'));
    CHECK(TextUtils::is_alnum('Z'));
    CHECK(TextUtils::is_alnum('5'));
  }

  SUBCASE("is_alnum with non-alphanumeric characters") {
    CHECK(!TextUtils::is_alnum(' '));
    CHECK(!TextUtils::is_alnum('-'));
    CHECK(!TextUtils::is_alnum('@'));
  }
}

TEST_CASE("is_alnum - Text") {
  SUBCASE("basic is_alnum") {
    CHECK(TextUtils::is_alnum("Hello123"));
    CHECK(TextUtils::is_alnum("World"));
    CHECK(TextUtils::is_alnum("Test5"));
  }

  SUBCASE("is_alnum with non-alphanumeric characters") {
    CHECK(!TextUtils::is_alnum("Hello 123"));
    CHECK(!TextUtils::is_alnum("-Test"));
    CHECK(!TextUtils::is_alnum("@World"));
  }

  SUBCASE("is_alnum with empty string") { CHECK(!TextUtils::is_alnum("")); }
}

TEST_CASE("is_ascii - code point") {
  SUBCASE("basic is_ascii") {
    CHECK(TextUtils::is_ascii('a'));
    CHECK(TextUtils::is_ascii('Z'));
    CHECK(TextUtils::is_ascii('5'));
  }

  SUBCASE("is_ascii with non-ASCII characters") {
    CHECK(!TextUtils::is_ascii(U'é'));
    CHECK(!TextUtils::is_ascii(U'ö'));
    CHECK(!TextUtils::is_ascii(U'中'));
  }
}

TEST_CASE("is_ascii - Text") {
  SUBCASE("basic is_ascii") {
    CHECK(TextUtils::is_ascii("Hello123"));
    CHECK(TextUtils::is_ascii("World"));
    CHECK(TextUtils::is_ascii("Test5"));
  }

  SUBCASE("is_ascii with non-ASCII characters") {
    CHECK(!TextUtils::is_ascii("Héllo"));
    CHECK(!TextUtils::is_ascii("Wörld"));
    CHECK(!TextUtils::is_ascii("测试"));
  }

  SUBCASE("is_ascii with empty string") { CHECK(TextUtils::is_ascii("")); }
}

TEST_CASE("count_chars") {
  SUBCASE("basic count_chars") {
    Text input = "hello world";
    int64_t count = TextUtils::count_chars(input);
    CHECK(count == 11);
  }

  SUBCASE("count_chars with unicode") {
    Text input = "héllo wörld";
    int64_t count = TextUtils::count_chars(input);
    CHECK(count == 11);
  }
}

TEST_CASE("count") {
  SUBCASE("basic count") {
    Text input = "hello world, hello everyone";
    Text substring = "hello";
    int64_t count = TextUtils::count(input, substring);
    CHECK(count == 2);
  }

  SUBCASE("count with no occurrences") {
    Text input = "hello world";
    Text substring = "abc";
    int64_t count = TextUtils::count(input, substring);
    CHECK(count == 0);
  }

  SUBCASE("count with empty substring") {
    Text input = "abc";
    Text substring = "";
    int64_t count = TextUtils::count(input, substring);
    CHECK(count == 4); // counts the positions before, between, and after characters
  }

  SUBCASE("count with unicode") {
    Text input = "héééllo wörld, héllo everyone";
    Text substring = "éé";
    int64_t count = TextUtils::count(input, substring);
    CHECK(count == 1);
  }
}

TEST_SUITE_END();
