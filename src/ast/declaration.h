#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/statement.h"
#include "util/option.h"

namespace ast {
class Declaration : public Statement {
 public:
  Declaration(lexer::Range location, NodeType node_type, Identifier id,
              Option<Type> type)
      : Statement(std::move(location), node_type),
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
