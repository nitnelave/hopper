#pragma once

#include <vector>

#include "error/error.h"
#include "visitor/visitor.h"

namespace ast {

/// An error reported while visiting.
class VisitorError : public GenericError {
 public:
  VisitorError(lexer::Range range, const std::string& message)
      : GenericError(message), range_(std::move(range)) {}

  std::string to_string() const override {
    return message() + "\n At " + range_.to_string();
  }

  const lexer::Range& location() { return range_; }

 private:
  lexer::Range range_;
};

/// Two lists, one of errors, one of warnings. Can be manipulated as a single
/// error as well.
template <typename Err = VisitorError>
class ErrorList : public GenericError {
 public:
  ErrorList() : GenericError("") {}
  const std::vector<Err>& errors() const { return errors_; }
  const std::vector<Err>& warnings() const { return warnings_; }

  std::string to_string() const override {
    std::stringstream ss;
    for (const auto& e : errors()) ss << e;
    return ss.str();
  }

  template <typename... Args>
  void add_error(Args&&... args) {
    errors_.emplace_back(std::forward<Args>(args)...);
  }

  template <typename... Args>
  void add_warning(Args&&... args) {
    warnings_.emplace_back(std::forward<Args>(args)...);
  }

 private:
  std::vector<VisitorError> errors_;
  std::vector<VisitorError> warnings_;
};

/// Visitor with an ErrorList. Only the visitor itself can add warnings and
/// errors, but everyone can read them.
template <typename Err = VisitorError>
class VisitorWithErrors : public ASTVisitor {
 public:
  const ast::ErrorList<>& error_list() const { return error_list_; }

 protected:
  template <typename... Args>
  void add_error(Args&&... args) {
    error_list_.add_error(std::forward<Args>(args)...);
  }

  template <typename... Args>
  void add_warning(Args&&... args) {
    error_list_.add_warning(std::forward<Args>(args)...);
  }

 private:
  ast::ErrorList<> error_list_;
};
}  // namespace ast
