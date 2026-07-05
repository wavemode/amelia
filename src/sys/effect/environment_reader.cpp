#include <cstdlib>
#include <cstring>

#include "environment_reader.hpp"
#include "prelude.hpp"

namespace amelia {
void EnvironmentReader::get_env(AbstractString &output, const AbstractString &name) {
  const char *env_value = std::getenv(name.c_str());
  if (env_value) {
    output.append(Text::from(env_value));
  }
}

} // namespace amelia
