#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/statement.h"
#include "util/option.h"

namespace ast {

enum CallingConvention { Normal = 0, C = 1 };

class Declaration : public Statement {
 public:
  Declaration(lexer::Range location, NodeType node_type, Identifier id,
              Option<Type> type, CallingConvention calling_convention)
      : Statement(std::move(location), node_type),
        id_(std::move(id)),
        type_(std::move(type)),
        calling_convention_(calling_convention) {}
  ~Declaration() override = default;

  const Identifier& id() const { return id_; }

  Option<Type>& type() { return type_; }

  bool calling_convention() const { return calling_convention_; }

 private:
  Identifier id_;
  Option<Type> type_;
  CallingConvention calling_convention_ = CallingConvention::Normal;
};
}  // namespace ast
