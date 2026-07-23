#include "type/data/type.hpp"
#include "binding/data/binding.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct ModuleType : Type {
  Map<Text, Flex<Binding>> scope;
  Map<Text, Flex<Binding>> implicit_scope;
};

} // namespace amelia
