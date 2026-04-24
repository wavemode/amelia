#pragma once

namespace amelia {

class IString;

struct IEnvironmentReader {
  virtual void get_env(IString &output, const IString &name) = 0;
};

} // namespace amelia
