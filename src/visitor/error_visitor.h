#pragma once

#include <vector>

#include "error/error.h"
#include "visitor/visitor.h"

namespace ast {

class VisitorError : public GenericError {
 public:
  VisitorError(const lexer::Range& range, const std::string& message)
      : GenericError(message), range_(range) {}

  std::string to_string() const override {
    return message() + " in " + range_.to_string();
  }

  const lexer::Range& location() { return range_; }

 private:
  const lexer::Range& range_;
};

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
}  // namespace ast
