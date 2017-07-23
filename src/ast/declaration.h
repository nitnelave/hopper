#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "util/option.h"

namespace ast {
class Declaration : public ASTNode {
 public:
  Declaration(lexer::Range location, Identifier id, Option<Type> type)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        type_(std::move(type)) {}
  ~Declaration() override = default;

  const Identifier& id() const { return id_; }

  Option<Type>& type() { return type_; }

 private:
  Identifier id_;
  Option<Type> type_;
};
}  // namespace ast
