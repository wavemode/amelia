#pragma once

namespace amelia {

class AbstractString;

struct IEnvironmentReader {
  virtual void get_env(AbstractString &output, const AbstractString &name) = 0;
};

} // namespace amelia
