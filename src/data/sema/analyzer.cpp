#include "analyzer.hpp"

namespace amelia {

namespace {

Type unknown_type() {
  return Type(Type::Primitive{PrimitiveKind::Unknown});
}

class SemaState {
public:
  SemaState(SemaResult &sema_result) : m_sema_result(sema_result) {}

  void typecheck() {
    Option<ModuleId> module_id;
    while ((module_id = select_module_to_typecheck()).has_value()) {
      typecheck_module(m_sema_result.modules[module_id.value()]);
    }
  }

private:
  void typecheck_module(Module &module_obj) {}

  void collect_bindings(Module &module_obj) {
    const ModuleNode &module_node = module_obj.ast.get_node(module_obj.ast_root).as_ModuleNode();
    for (NodeId decl_node_id : module_node.decls) {
      String name;
      get_binding_name(name, module_obj, decl_node_id);
    }
  }

  static void get_binding_name(String &out, const Module &module_obj, NodeId decl_node_id) {
    const Node &node = module_obj.ast.get_node(decl_node_id);
    switch (node.type()) {
    case NodeType::LetDeclNode: {
      const auto &let_decl = node.as_LetDeclNode();
      const auto &name_node = module_obj.ast.get_node(let_decl.target).as_IdentifierNode();
      const auto &name_token = module_obj.tokens.get_token(name_node.token);
      out.append(identifier_name(name_token));
      break;
    }
    default:
      throw RuntimeError("Unexpected declaration node type");
    }
  }

  void set_up_dep_counts() {
    while (m_module_dep_counts.size() < m_sema_result.modules.size()) {
      m_module_dep_counts.push_back(0);
    }
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      m_module_dep_counts[i] = m_sema_result.modules[i].imported_ids.size();
    }
  }

  Option<ModuleId> select_module_to_typecheck() {
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      Module &module_obj = m_sema_result.modules[i];
      if (module_obj.scope->bindings.size() != 0) {
        // already analyzed
        continue;
      }
      if (m_module_dep_counts[i] == 0) {
        if (module_obj.group_module_ids.size() != 0) {
          // For cooperation with other threads, we should only ever analyze the "head" of a
          // module group - that is, the module in the group with the lowest ID.
          for (ModuleId group_module_id : module_obj.group_module_ids) {
            if (group_module_id < static_cast<ModuleId>(i)) {
              goto cont;
            }
          }
        }
        return Some(i);
      }
    cont:;
    }
    return None();
  }

  SemaResult &m_sema_result;
  List<uint32_t> m_module_dep_counts;
};

} // namespace

void Analyzer::typecheck(SemaResult &sema_result) {}

} // namespace amelia
