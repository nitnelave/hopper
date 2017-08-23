#include "name_resolution/visitor.h"

#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/local_variable_declaration.h"
#include "ast/variable_reference.h"

namespace name_resolution {
void NameResolver::resolve_option_type(Option<ast::Type>* maybe_type) {
  if (maybe_type->is_ok()) {
    auto& type = maybe_type->value_or_die();
    if (!type.is_resolved()) {
      auto it = type_map_.find(type.id());
      if (it == type_map_.end()) {
        add_error(type.location(),
                  "Could not resolve type: " + type.to_string());
        return;
      }
      type.set_resolution(it->second);
    }
  }
}

void NameResolver::visit(ast::LocalVariableDeclaration* node) {
  ASTVisitor::visit(node);  // Recurse into the value.
  visit_variable_declaration(node);
}

void NameResolver::visit(ast::FunctionArgumentDeclaration* node) {
  ASTVisitor::visit(node);  // Recurse into the value.
  visit_variable_declaration(node);
}

void NameResolver::visit_variable_declaration(ast::VariableDeclaration* node) {
  resolve_option_type(&node->type());
  auto it = name_map_.find(node->id());
  if (it != name_map_.end())
    add_warning(node->id().location(),
                "Shadowing of a previously declared variable: " +
                    node->id().to_string());
  name_map_[node->id()] = node;
}

void NameResolver::visit(ast::VariableReference* node) {
  auto it = name_map_.find(node->id());
  if (it == name_map_.end())
    add_error(node->id().location(),
              "No variable named `" + node->id().to_string() + "'");
  else
    node->resolution() = it->second;
}

void NameResolver::visit(ast::FunctionDeclaration* node) {
  resolve_option_type(&node->type());
  ASTVisitor::visit(node);
}
}  // namespace name_resolution
