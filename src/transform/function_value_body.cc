#include "transform/function_value_body.h"

#include "ast/block_statement.h"
#include "ast/function_declaration.h"
#include "ast/return_statement.h"
#include "ast/statement.h"

namespace transform {
void FunctionValueBodyTransformer::visit(ast::FunctionDeclaration* node) {
  using ValueBody = ast::FunctionDeclaration::ValueBody;
  if (node->body().is<ValueBody>()) {
    std::unique_ptr<ast::Value> value =
        node->body().consume_unchecked<ValueBody>();
    ast::BlockStatement::StatementList new_body;

    auto value_location = value->location();
    new_body.emplace_back(std::make_unique<ast::ReturnStatement>(
        value_location, std::move(value)));
    node->body() = std::make_unique<ast::BlockStatement>(value_location,
                                                         std::move(new_body));
  }
}
}  // namespace transform
