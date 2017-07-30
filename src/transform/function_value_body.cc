#include "transform/function_value_body.h"

#include "ast/function_declaration.h"
#include "ast/return_statement.h"
#include "ast/statement.h"
#include "ast/block.h"

namespace transform {
void FunctionValueBodyTransformer::visit(ast::FunctionDeclaration* node) {
  using ValueBody = ast::FunctionDeclaration::ValueBody;
  if (node->body().is<ValueBody>()) {
    std::unique_ptr<ast::Value> value =
        node->body().consume_unchecked<ValueBody>();
    std::vector<std::unique_ptr<ast::Statement>> new_body;
    new_body.emplace_back(std::make_unique<ast::ReturnStatement>(
        value->location(), std::move(value)));
    node->body() = std::make_unique<ast::Block>(value->location(), std::move(new_body));
  }
}
}  // namespace transform
