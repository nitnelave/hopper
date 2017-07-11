#pragma once

namespace ast {

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

class Type {
 public:
  explicit Type(Identifier id) : id_(std::move(id)) {}
  const Identifier& id() const { return id_; }
  const lexer::Range& location() const { return id_.location(); }

  const std::string& to_string() const { return id_.to_string(); }

 private:
  Identifier id_;
};

}  // namespace ast
