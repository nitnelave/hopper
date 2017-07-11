#pragma once

#include <memory>
#include <vector>

#include "ast/value.h"

namespace ast {

class Module : public Value {
 public:
  using Declarations = std::vector<std::unique_ptr<ASTNode>>;
  Module(lexer::Range location, Declarations top_level_declarations)
      : Value(std::move(location)),
        top_level_declarations_(std::move(top_level_declarations)) {}
  ~Module() override = default;

  const Declarations& top_level_declarations() const {
    return top_level_declarations_;
  }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  Declarations top_level_declarations_;
};

}  // namespace ast
