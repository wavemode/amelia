#include "environment_reader.h"

#include "prelude.h"

#include <cstdlib>
#include <cstring>

namespace amelia {
void EnvironmentReader::get_env(IString &output, const IString &name) {
  const char *env_value = std::getenv(name.c_str());
  output.append(Text(Slice(env_value, env_value ? std::strlen(env_value) : 0)));
}

} // namespace amelia
