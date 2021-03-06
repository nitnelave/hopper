#pragma once

#include <memory>
#include <vector>

#include "ast/value.h"
#include "visitor/visitor.h"

namespace ast {

class Module : public ASTNode {
 public:
  using Declarations = std::vector<std::unique_ptr<ASTNode>>;
  Module(lexer::Range location, Declarations top_level_declarations)
      : ASTNode(std::move(location), NodeType::MODULE),
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
