#pragma once

#include <unordered_map>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "util/option.h"
#include "visitor/error_visitor.h"

namespace name_resolution {
class NameResolver : public ast::ASTVisitor {
 public:
  using ErrorList = ast::ErrorList<ast::VisitorError>;
  explicit NameResolver(Option<NameResolver*> parent = none)
      : parent_(std::move(parent)) {}

  const ErrorList& error_list() const { return error_list_; }

  // void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::VariableDeclaration* node) override;
  void visit(ast::VariableReference* node) override;

 private:
  ErrorList error_list_;
  std::unordered_map<ast::Identifier, ast::TypeDeclaration*> type_map_;
  std::unordered_map<ast::Identifier, ast::Declaration*> name_map_;
  Option<NameResolver*> parent_;
};
}  // namespace name_resolution
