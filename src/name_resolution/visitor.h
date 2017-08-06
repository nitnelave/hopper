#pragma once

#include <unordered_map>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/builtin_type.h"
#include "util/option.h"
#include "visitor/error_visitor.h"

namespace name_resolution {
class NameResolver : public ast::ASTVisitor {
 public:
  using ErrorList = ast::ErrorList<ast::VisitorError>;
  explicit NameResolver(Option<NameResolver*> parent = none)
      : parent_(std::move(parent)) {
    for (auto type : ast::types::builtin_types) {
      type_map_.emplace(type->id(), type);
    }
  }

  const ErrorList& error_list() const { return error_list_; }

  void visit(ast::LocalVariableDeclaration* node) override;
  void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::FunctionArgumentDeclaration* node) override;
  void visit(ast::VariableReference* node) override;

 private:
  void visit_variable_declaration(ast::VariableDeclaration* node);
  void resolve_option_type(Option<ast::Type>* maybe_type);
  ErrorList error_list_;
  std::unordered_map<ast::Identifier, ast::TypeDeclaration*> type_map_;
  std::unordered_map<ast::Identifier, ast::Declaration*> name_map_;
  Option<NameResolver*> parent_;
};
}  // namespace name_resolution
