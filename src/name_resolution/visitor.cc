#include "name_resolution/visitor.h"
#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/variable_declaration.h"
#include "ast/variable_reference.h"

namespace name_resolution {
void NameResolver::visit(ast::VariableDeclaration* node) {
  ASTVisitor::visit(node);  // Recurse into the value.
  if (node->type().is_ok()) {
    auto& type = node->type().value_or_die();
    if (!type.is_resolved()) {
      auto it = type_map_.find(type.id());
      if (it == type_map_.end()) {
        error_list_.add_error(type.location(),
                              "Could not resolve type: " + type.to_string());
        return;
      }
      type.set_resolution(it->second);
    }
  }
  auto it = name_map_.find(node->id());
  if (it != name_map_.end())
    error_list_.add_warning(node->id().location(),
                            "Shadowing of a previously declared variable: " +
                                node->id().to_string());
  name_map_[node->id()] = node;
}

void NameResolver::visit(ast::VariableReference* node) {
  auto it = name_map_.find(node->id());
  if (it == name_map_.end())
    error_list_.add_error(node->id().location(),
                          "No variable named `" + node->id().to_string() + "'");
  else
    node->resolution() = it->second;
}
}  // namespace name_resolution
