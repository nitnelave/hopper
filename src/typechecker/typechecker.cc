#include "typechecker/typechecker.h"

#include "ast/base_types.h"
#include "ast/binary_operation.h"
#include "ast/boolean_constant.h"
#include "ast/builtin_type.h"
#include "ast/function_declaration.h"
#include "ast/int_constant.h"

namespace typechecker {

using ast::Type;

bool is_integer(const Type& t) {
  auto decl = t.get_declaration();
  return decl == &ast::types::int8 || decl == &ast::types::int16 ||
         decl == &ast::types::int32 || decl == &ast::types::int64;
}

void TypeChecker::visit(ast::BooleanConstant* node) {
  node->type() = Type(&ast::types::boolean);
}

void TypeChecker::visit(ast::IntConstant* node) {
  node->type() = Type(&ast::types::int64);
}

void TypeChecker::visit(ast::BinaryOp* node) {
  node->left_value().accept(*this);
  node->right_value().accept(*this);
  // At this point, all the names are resolved.
  assert(node->left_value().type().is_ok() && "Left value type is not resolved");
  assert(node->right_value().type().is_ok() && "Right value type is not resolved");
  const auto& left_type = node->left_value().type().value_or_die();
  const auto& right_type = node->right_value().type().value_or_die();
  if (is_integer(left_type) && is_integer(right_type)) {
    using namespace ast::types;
    IntWidth result_width =
        std::max(int_type_to_width(left_type.get_declaration()).value_or_die(),
                 int_type_to_width(right_type.get_declaration()).value_or_die());
    node->type() = width_to_int_type(result_width);
  } else {
    error_list_.add_error(node->location(),
                          "Invalid operand types for binary operation `" +
                              to_string(node->operation()) + "': `" +
                              left_type.to_string() + "' and `" +
                              right_type.to_string() + "'");
  }
}

void TypeChecker::visit(ast::FunctionDeclaration* node) {
  // Assumes we ran the FunctionValueBody transformer first.
  size_t num_errors = error_list_.errors().size();
  for (auto& statement :
       node->body().get<ast::FunctionDeclaration::StatementsBody>())
    statement->accept(*this);
  if (num_errors < error_list_.errors().size())
    // Errors while processing the body.
    return;
  // Visit all the statements, looking for return statements, collect the
  // types.
  if (node->type().is_ok()) return;
}

}  // namespace typechecker
