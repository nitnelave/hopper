#include "transform/add_return.h"

#include "ast/block_statement.h"
#include "ast/builtin_type.h"
#include "ast/function_declaration.h"
#include "ast/if_statement.h"
#include "ast/return_statement.h"
#include "ast/variable_declaration.h"
#include "util/logging.h"

namespace transform {
void VoidFunctionReturnAdder::visit(ast::IfStatement* node) {
  if (node->else_statement().is_ok()) {
    visit(node->body().get());
    bool if_returned = has_returned_;
    has_returned_ = false;
    visit(node->else_statement().value_or_die().get());
    has_returned_ = has_returned_ && if_returned;
  }
}

void VoidFunctionReturnAdder::visit(ast::ReturnStatement* /*unused*/) {
  has_returned_ = true;
}

void VoidFunctionReturnAdder::visit(ast::BlockStatement* node) {
  CHECK(!has_returned_);
  auto& statements = node->statements();
  for (auto stmt_it = statements.begin(); stmt_it != statements.end();
       ++stmt_it) {
    const auto& statement = *stmt_it;
    if (has_returned_) {
      error_list_.add_error(statement->location(), "Unreachable code");
      statements.erase(stmt_it, statements.end());
      break;
    }
    statement->accept(*this);
  }
}

void VoidFunctionReturnAdder::visit(ast::FunctionDeclaration* node) {
  has_returned_ = false;
  CHECK(node->type().is_ok()) << "Function return type not deduced: "
                              << node->name();
  const auto& type = node->type().value_or_die();
  CHECK(type.is_resolved()) << "Function return type not resolved: "
                            << node->name();


  using StatementsBody = ast::FunctionDeclaration::StatementsBody;
  CHECK(node->body().is<StatementsBody>())
      << "Function should have a statements body:" << node->name();
  const auto& statements = node->body().get_unchecked<StatementsBody>();
  visit(statements.get());

  if (has_returned_) {
    CHECK(!statements->statements().empty()) << "Empty function that returned: "
                                             << node->name();
  } else {
    if (type.get_declaration() == &ast::types::void_type) {
      statements->statements().emplace_back(
          std::make_unique<ast::ReturnStatement>(lexer::invalid_range(), none));
    } else {
      error_list_.add_error(node->location(),
                            "Reached the end of a function not returning void");
    }
  }
}
}  // namespace transform
