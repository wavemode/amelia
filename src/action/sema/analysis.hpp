#pragma once

#include "prelude.hpp"

#include "data/sema/sema_result.hpp"

namespace amelia {

void load_module(const Text &module_name, SemaResult &sema_result);

} // namespace amelia
