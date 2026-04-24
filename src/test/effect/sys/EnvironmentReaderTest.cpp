#include "Prelude.h"
#include <doctest.h>

#include "effect/sys/EnvironmentReader.h"

#include <cstdlib>

TEST_SUITE_BEGIN("EnvironmentReader");

using namespace amelia;

TEST_CASE("get_env") {
  EnvironmentReader env_reader;
  String output;

  SUBCASE("existing var") {
    String test_env_var_name = "AMELIA_TEST_ENV_VAR";
    String test_env_var_value = "Hello, World!";
    setenv(test_env_var_name.c_str(), test_env_var_value.c_str(), 1);

    env_reader.get_env(output, test_env_var_name);
    CHECK(output == test_env_var_value);
  }

  SUBCASE("nonexistent var") {
    String non_existent_var_name = "AMELIA_NON_EXISTENT_ENV_VAR";

    env_reader.get_env(output, non_existent_var_name);
    CHECK(output == "");
  }
}

TEST_SUITE_END();
