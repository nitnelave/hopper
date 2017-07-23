#pragma once

#include <functional>  // hash
#include <string>

#include "lexer/token.h"
#include "util/option.h"

namespace ast {

class TypeDeclaration;

class Identifier {
 public:
  explicit Identifier(std::string name, lexer::Range location,
                      bool is_uppercase, bool absolute = false)
      : name_(std::move(name)),
        location_(std::move(location)),
        absolute_(absolute),
        is_uppercase_(is_uppercase) {}
  const std::string& to_string() const { return qualified_name(); }
  const std::string& qualified_name() const { return name_; }
  const std::string& short_name() const { return name_; }
  const lexer::Range& location() const { return location_; }

  bool is_uppercase() const { return is_uppercase_; }

 private:
  std::string name_;
  lexer::Range location_;
  bool absolute_;
  bool is_uppercase_;
};

inline bool operator==(const Identifier& left, const Identifier& right) {
  return left.qualified_name() == right.qualified_name();
}

class Type {
 public:
  explicit Type(Identifier id) : id_(std::move(id)) {}
  explicit Type(const TypeDeclaration* decl) : type_(decl) {}

  bool is_resolved() const { return type_.is_ok(); }

  void set_resolution(const TypeDeclaration* decl) { type_ = decl; }

  const TypeDeclaration* get_declaration() const {
    assert(is_resolved());
    return type_.value_or_die();
  }

  const Identifier& id() const;
  const lexer::Range& location() const { return id().location(); }

  const std::string& to_string() const { return id().to_string(); }

 private:
  Option<Identifier> id_;
  Option<const TypeDeclaration*> type_;
};

}  // namespace ast

namespace std {
template <>
struct hash<ast::Identifier> {
  size_t operator()(const ast::Identifier& id) const {
    return std::hash<std::string>{}(id.qualified_name());
  }
};
}  // namespace std
