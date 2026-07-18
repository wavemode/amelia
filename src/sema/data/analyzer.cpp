#include <climits>

#include "analyzer.hpp"

#include "binding/logic/analysis.hpp"
#include "sema/data/module.hpp"
#include "sema/data/scope.hpp"
#include "sema/data/sema_result.hpp"
#include "sema/data/type_error.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "sema/data/module_analysis_context.hpp"

namespace amelia {

namespace {

class SemaWorkerState : IModuleAnalysisState {
public:
  SemaWorkerState(Module &module_obj) : m_module_obj(module_obj) {}

  Flex<Binding> get_binding_by_id(BindingId binding_id) override {
    Scope &scope = *m_module_obj.scope;
    if (binding_id < 0 || binding_id >= static_cast<BindingId>(scope.active_bindings.size())) {
      throw RuntimeError("Invalid binding ID");
    }
    return scope.active_bindings[binding_id];
  }

  Option<BindingId> get_binding_id_by_name(Text name) override {
    Scope &scope = *m_module_obj.scope;
    return scope.active_binding_ids.find(name);
  }

  void push_binding(Flex<Binding> binding) override {
    Scope &scope = *m_module_obj.scope;
    Text name = binding->name;
    const Option<BindingId> existing_binding_id = scope.active_binding_ids.find(name);
    BindingId new_binding_id = scope.active_bindings.size();
    binding->shadowed_binding_id = existing_binding_id;
    binding->id = new_binding_id;
    scope.active_bindings.push_back(binding);
    scope.active_binding_ids.set(name, new_binding_id);
    if (m_context.binding_currently_analyzing.has_value()) {
      m_context.binding_currently_analyzing.value()->child_bindings.push_back(binding);
    }
  }

  Binding &pop_binding() override {
    Scope &scope = *m_module_obj.scope;
    if (scope.active_bindings.size() == 0) {
      throw RuntimeError("Attempted to pop binding from empty scope");
    }
    Binding &binding = scope.active_bindings[scope.active_bindings.size() - 1];
    if (!is_binding_analyzed(*this, binding)) {
      // TODO: warn about unused and un-analyzed?
    }
    if (!binding.shadowed_binding_id.has_value()) {
      scope.active_binding_ids.remove(binding.name);
    } else {
      scope.active_binding_ids.set(binding.name, binding.shadowed_binding_id.value());
    }
    scope.active_bindings.pop_back();
    return binding;
  }

  size_t get_binding_stack_size() const override {
    return m_module_obj.scope->active_bindings.size();
  }

  void analyze_module() {
    m_module_obj.analyzed = true;
    collect_top_level_bindings(*this, m_module_obj.ast_root);
    List<Text> binding_names;
    for (const auto &[binding_name, binding_id] : m_module_obj.scope->active_binding_ids) {
      binding_names.push_back(binding_name);
    }
    binding_names.sort();
    for (Text binding_name : binding_names) {
      Binding
          &binding = *m_module_obj.scope
                          ->active_bindings[m_module_obj.scope->active_binding_ids[binding_name]];
      analyze_top_level_binding(*this, binding);
    }
  }

  const Node &get_node(NodeId node_id) const override {
    return m_module_obj.ast.get_node(node_id);
  }

  Module &current_module() override {
    return m_module_obj;
  }

  ModuleAnalysisContext &current_context() override {
    return m_context;
  }

private:
  [[noreturn]] void raise_type_error_at_node(NodeId node_id, String &&error_message) override {
    const Node &node = get_node(node_id);
    const Token &token = m_module_obj.tokens.get_token(node.start_token());
    throw TypeError(token.location, move(error_message));
  }

  Module &m_module_obj;
  ModuleAnalysisContext m_context;
};

class SemaState {
public:
  SemaState(SemaResult &sema_result) : m_sema_result(sema_result) {}

  void analyze() {
    set_up_dep_counts();
    Option<ModuleId> module_id;
    while ((module_id = select_module_to_analyze()).has_value()) {
      SemaWorkerState(m_sema_result.modules[module_id.value()]).analyze_module();
    }
  }

private:
  void set_up_dep_counts() {
    while (m_module_dep_counts.size() < m_sema_result.modules.size()) {
      m_module_dep_counts.push_back(0);
    }
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      m_module_dep_counts[i] = m_sema_result.modules[i].imported_ids.size();
    }
  }

  Option<ModuleId> select_module_to_analyze() {
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      Module &module_obj = m_sema_result.modules[i];
      if (module_obj.analyzed) {
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

void Analyzer::analyze(SemaResult &sema_result) {
  SemaState(sema_result).analyze();
}

} // namespace amelia
