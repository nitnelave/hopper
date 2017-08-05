#pragma once

#include <memory>

#include "ast/declaration.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class VariableDeclaration : public Declaration {
 public:
  VariableDeclaration(lexer::Range location, Identifier id, Option<Type> type,
                      Option<std::unique_ptr<Value>> value, bool mut)
      : Declaration(std::move(location), std::move(id), std::move(type)),
        value_(std::move(value)),
        mut_(mut) {}

  bool is_mutable() const { return mut_; }

  const Option<std::unique_ptr<Value>>& value() const { return value_; }

  ~VariableDeclaration() override = default;

 private:
  Option<std::unique_ptr<Value>> value_;
  bool mut_;
};

}  // namespace ast
