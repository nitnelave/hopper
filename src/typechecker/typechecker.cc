#include "typechecker/typechecker.h"

#include "ast/base_types.h"
#include "ast/binary_operation.h"
#include "ast/boolean_constant.h"
#include "ast/builtin_type.h"
#include "ast/function_declaration.h"
#include "ast/int_constant.h"
#include "ast/local_variable_declaration.h"
#include "ast/return_statement.h"
#include "ast/variable_reference.h"
#include "util/logging.h"

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

ast::types::IntWidth smallest_int_width_for(int64_t value) {
  using ast::types::IntWidth;
  if (value >= (1l << 32)) return IntWidth::W_64;
  if (value >= (1 << 16)) return IntWidth::W_32;
  if (value >= (1 << 8)) return IntWidth::W_16;
  return IntWidth::W_8;
}

void TypeChecker::visit(ast::IntConstant* node) {
  if (node->value() >= (1l << 32))
    node->type() = Type(&ast::types::int64);
  else
    node->type() = Type(&ast::types::int32);
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
    add_error(node->location(), "Invalid operand types for binary operation `" +
                                    to_string(node->operation()) + "': `" +
                                    left_type.to_string() + "' and `" +
                                    right_type.to_string() + "'");
  }
}

bool is_compatible(const ast::Type& declaration_type, const ast::Value* value) {
  CHECK(value->type().is_ok());
  const auto& value_type = value->type().value_or_die();
  if (declaration_type == value_type) return true;
  if (is_integer(declaration_type) && is_integer(value_type)) {
    auto actual_width =
        ast::types::int_type_to_width(value_type.get_declaration())
            .value_or_die();
    if (value->node_type() == ast::NodeType::INT_CONSTANT) {
      const auto* constant =
          static_cast<const ast::IntConstant*>(value);  // NOLINT
      actual_width = smallest_int_width_for(constant->value());
    }
    return ast::types::int_type_to_width(declaration_type.get_declaration())
               .value_or_die() >= actual_width;
  }
  return false;
}

void TypeChecker::visit(ast::ReturnStatement* node) {
  size_t num_errors = error_list().errors().size();
  ASTVisitor::visit(node);
  if (num_errors < error_list().errors().size()) return;

  auto add_type_error = [&](const auto& value_type) {
    add_error(node->location(),
              "Invalid return type: the function returns `" +
                  function_return_type_.value_or_die().to_string() +
                  "', but the return value is of type `" +
                  value_type.to_string() + "'");
  };

  if (node->value().is_ok()) {
    auto* value = node->value().value_or_die().get();
    CHECK(value->type().is_ok());
    const auto& value_type = value->type().value_or_die();
    if (function_return_type_.is_ok()) {
      if (!is_compatible(function_return_type_.value_or_die(), value)) {
        add_type_error(value_type);
        return;
      }
    } else {
      function_return_type_ = value_type;
    }
  } else {
    using ast::types::void_type;
    if (function_return_type_.is_ok() &&
        function_return_type_.value_or_die().get_declaration() != &void_type) {
      add_type_error(void_type.id());
    }
    function_return_type_ = Type(&void_type);
  }
}

void TypeChecker::visit(ast::FunctionDeclaration* node) {
  // Assumes we ran the FunctionValueBody transformer first.
  size_t num_errors = error_list().errors().size();

  function_return_type_ = node->type();
  // Visit the children.
  ASTVisitor::visit(node);
  if (num_errors < error_list().errors().size())
    // Errors while processing the body.
    return;
  // Visit all the statements, looking for return statements, collect the
  // types.
  if (node->type().is_ok()) return;
  if (!function_return_type_.is_ok())
    function_return_type_ = Type(&ast::types::void_type);
  node->type() = function_return_type_;
}

void TypeChecker::visit(ast::LocalVariableDeclaration* node) {
  ASTVisitor::visit(node);
  if (node->value().is_ok()) {
    CHECK(node->value().value_or_die()->type().is_ok());
    const auto& value_type =
        node->value().value_or_die()->type().value_or_die();
    if (node->type().is_ok()) {
      const auto& node_type = node->type().value_or_die();
      if (!is_compatible(node_type, node->value().value_or_die().get())) {
        add_error(node->location(),
                  "Invalid variable declaration: a variable of type `" +
                      node_type.to_string() +
                      "' cannot contain a value of type `" +
                      value_type.to_string() + "'");
        return;
      }
    } else {
      node->type() = value_type;
    }
  } else {
    CHECK(node->type().is_ok())
        << "Variable declaration with no type and no value?";
  }
}

}  // namespace typechecker
