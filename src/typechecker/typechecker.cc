#include "typechecker/typechecker.h"

#include "ast/base_types.h"
#include "ast/binary_operation.h"
#include "ast/boolean_constant.h"
#include "ast/builtin_type.h"
#include "ast/function_declaration.h"
#include "ast/int_constant.h"
#include "ast/return_statement.h"
#include "ast/variable_reference.h"

namespace typechecker {

using ast::Type;

bool is_integer(const Type& t) {
  auto decl = t.get_declaration();
  return decl == &ast::types::int8 || decl == &ast::types::int16 ||
         decl == &ast::types::int32 || decl == &ast::types::int64;
}

bool is_boolean(const Type& t) {
  auto decl = t.get_declaration();
  return decl == &ast::types::boolean;
}

bool is_boolean_operator(ast::BinaryOperator op) {
  using ast::BinaryOperator;
  switch (op) {
    case BinaryOperator::OR:
    case BinaryOperator::AND:
      return true;
    default:
      return false;
  }
}

bool is_integer_operator(ast::BinaryOperator op) {
  using ast::BinaryOperator;
  switch (op) {
    case BinaryOperator::BITOR:
    case BinaryOperator::BITXOR:
    case BinaryOperator::BITAND:
    case BinaryOperator::GREATER:
    case BinaryOperator::GREATER_OR_EQUAL:
    case BinaryOperator::LESS:
    case BinaryOperator::LESS_OR_EQUAL:
    case BinaryOperator::BITSHIFT_LEFT:
    case BinaryOperator::BITSHIFT_RIGHT:
    case BinaryOperator::PLUS:
    case BinaryOperator::MINUS:
    case BinaryOperator::TIMES:
    case BinaryOperator::DIVIDE:
    case BinaryOperator::INT_DIVIDE:
    case BinaryOperator::MODULO:
      return true;
    default:
      return false;
  }
}

void TypeChecker::visit(ast::VariableReference* node) {
  assert(node->is_resolved() && "Variable was not resolved");
  const auto& declaration_type = node->resolution().value_or_die()->type();
  assert(declaration_type.is_ok() && "Declaration did not have a type");
  node->type() = Type(declaration_type.value_or_die().get_declaration());
}

void TypeChecker::visit(ast::BooleanConstant* node) {
  node->type() = Type(&ast::types::boolean);
}

void TypeChecker::visit(ast::IntConstant* node) {
  node->type() = Type(&ast::types::int64);
}

void TypeChecker::visit(ast::BinaryOp* node) {
  // Visit sub-trees.
  ASTVisitor::visit(node);
  // At this point, all the names are resolved.
  assert(node->left_value().type().is_ok() &&
         "Left value type is not resolved");
  assert(node->right_value().type().is_ok() &&
         "Right value type is not resolved");
  const auto& left_type = node->left_value().type().value_or_die();
  const auto& right_type = node->right_value().type().value_or_die();
  if (is_integer_operator(node->operation()) && is_integer(left_type) &&
      is_integer(right_type)) {
    ast::types::IntWidth result_width =
        std::max(ast::types::int_type_to_width(left_type.get_declaration())
                     .value_or_die(),
                 ast::types::int_type_to_width(right_type.get_declaration())
                     .value_or_die());
    node->type() = width_to_int_type(result_width);
  } else if (is_boolean_operator(node->operation()) && is_boolean(left_type) &&
             is_boolean(right_type)) {
    node->type() = &ast::types::boolean;
  } else {
    error_list_.add_error(node->location(),
                          "Invalid operand types for binary operation `" +
                              to_string(node->operation()) + "': `" +
                              left_type.to_string() + "' and `" +
                              right_type.to_string() + "'");
  }
}

void TypeChecker::visit(ast::ReturnStatement* node) {
  size_t num_errors = error_list_.errors().size();
  ASTVisitor::visit(node);
  if (num_errors < error_list_.errors().size()) return;

  const Type value_type = [&]() {
    if (node->value().is_ok()) {
      const auto& maybe_type = node->value().value_or_die()->type();
      assert(maybe_type.is_ok());
      return maybe_type.value_or_die();
    }
    return Type(&ast::types::void_type);
  }();
  if (function_return_type_.is_ok()) {
    if (function_return_type_.value_or_die() != value_type) {
      error_list_.add_error(
          node->location(),
          "Invalid return type: the function returns `" +
              function_return_type_.value_or_die().to_string() +
              "', but the return value is of type `" + value_type.to_string() +
              "'");
      return;
    }
  }
  function_return_type_ = value_type;
}

void TypeChecker::visit(ast::FunctionDeclaration* node) {
  // Assumes we ran the FunctionValueBody transformer first.
  size_t num_errors = error_list_.errors().size();

  function_return_type_ = node->type();
  // Visit the children.
  ASTVisitor::visit(node);
  if (num_errors < error_list_.errors().size())
    // Errors while processing the body.
    return;
  // Visit all the statements, looking for return statements, collect the
  // types.
  if (node->type().is_ok()) return;
  if (!function_return_type_.is_ok())
    function_return_type_ = Type(&ast::types::void_type);
  node->type() = function_return_type_;
}

}  // namespace typechecker
