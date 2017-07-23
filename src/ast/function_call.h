#pragma once

#include <memory>
#include <vector>

#include "ast/base_types.h"
#include "ast/value.h"
#include "visitor/visitor.h"

namespace ast {
class FunctionCall : public Value {
 public:
  using ArgumentList = std::vector<std::unique_ptr<Value>>;
  FunctionCall(lexer::Range range, std::unique_ptr<Value> base,
               ArgumentList args)
      : Value(std::move(range)),
        base_(std::move(base)),
        args_(std::move(args)) {}

  // The base of the function, or what comes before the arguments.
  Value& base() { return *base_; }

  ArgumentList& arguments() { return args_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  std::unique_ptr<Value> base_;
  ArgumentList args_;
};
}  // namespace ast
