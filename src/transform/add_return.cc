#include "transform/add_return.h"

#include "ast/block_statement.h"
#include "ast/builtin_type.h"
#include "ast/function_declaration.h"
#include "ast/return_statement.h"
#include "ast/variable_declaration.h"
#include "util/logging.h"

namespace transform {
void VoidFunctionReturnAdder::visit(ast::FunctionDeclaration* node) {
  using StatementsBody = ast::FunctionDeclaration::StatementsBody;
  CHECK(node->type().is_ok()) << "Function return type not deduced.";
  const auto& type = node->type().value_or_die();
  CHECK(type.is_resolved()) << "Function return type not resolved.";
  if (type.get_declaration() == &ast::types::void_type) {
    CHECK(node->body().is<StatementsBody>())
        << "Function with void type should have a statements body.";
    const auto& statements = node->body().get_unchecked<StatementsBody>();
    if (statements->statements().empty() ||
        statements->statements().back()->node_type() !=
            ast::NodeType::RETURN_STATEMENT) {
      statements->statements().emplace_back(
          std::make_unique<ast::ReturnStatement>(lexer::invalid_range(), none));
    }
  }
}
}  // namespace transform
