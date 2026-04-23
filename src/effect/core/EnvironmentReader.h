#pragma once

#include "interface/core/IEnvironmentReader.h"

namespace amelia {

class EnvironmentReader : public IEnvironmentReader {
public:
  void get_env(IString &output, const IString &name) override;

private:
};

} // namespace amelia
