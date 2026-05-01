#pragma once

#include "interface/sys/environment_reader.h"

namespace amelia {

class EnvironmentReader : public IEnvironmentReader {
public:
  void get_env(IString &output, const IString &name) override;

private:
};

} // namespace amelia
