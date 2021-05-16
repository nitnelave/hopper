#include "transform/add_destroy_variable.h"

#include <iterator>

#include "ast/block_statement.h"
#include "ast/function_declaration.h"
#include "ast/local_variable_declaration.h"
#include "ast/variable_destruction.h"

namespace transform {

namespace {

template <typename InsertIterator>
InsertIterator destroy_variables_in_scope(
    InsertIterator iter,
    const VariableDestructorAdder::ScopeVariables& variables) {
  for (auto it = variables.rbegin(); it != variables.rend(); ++it)
    *iter = std::make_unique<ast::VariableDestruction>(*it);
  return iter;
}

template <typename InsertIterator>
InsertIterator destroy_all_variables(
    InsertIterator iter,
    const VariableDestructorAdder::ScopeStack& scope_stack) {
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
    iter = destroy_variables_in_scope(iter, *it);
  return iter;
}

}  // namespace

void VariableDestructorAdder::visit(ast::FunctionDeclaration* node) {
  std::vector<ast::VariableDeclaration*> arguments;
  for (const auto& arg : node->arguments()) {
    arguments.emplace_back(arg.get());
  }
  visit_block(node->body()
                  .get_unchecked<ast::FunctionDeclaration::StatementsBody>()
                  .get(),
              arguments);
}

void VariableDestructorAdder::visit_block(
    ast::BlockStatement* node,
    const std::vector<ast::VariableDeclaration*>& function_arguments) {
  using ast::NodeType;
  enter_scope();
  std::move(function_arguments.begin(), function_arguments.end(),
            std::back_inserter(current_scope()));
  auto& statements = node->statements();
  for (const auto& statement : statements) {
    if (statement->node_type() == NodeType::LOCAL_VARIABLE_DECLARATION) {
      // static_cast is safe because of the tag.
      auto* var_decl = static_cast<ast::LocalVariableDeclaration*>(  // NOLINT
          statement.get());
      current_scope().emplace_back(var_decl);
    }
    statement->accept(*this);
  }

  auto last_type = statements.back()->node_type();
  if (last_type == NodeType::RETURN_STATEMENT) {
    // Insert the destruction nodes before the return statement.
    destroy_all_variables(
        std::inserter(statements, std::prev(statements.end())),
        variables_in_scope_);
  } else if (last_type != NodeType::UNREACHABLE) {
    // If it's unreachable, no need to destroy the variables.
    destroy_variables_in_scope(back_inserter(statements), current_scope());
  }
  exit_scope();
}

void VariableDestructorAdder::visit(ast::BlockStatement* node) {
  visit_block(node);
}

}  // namespace transform
