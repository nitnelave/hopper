#pragma once

namespace ast {

class Identifier {
 public:
  explicit Identifier(std::string name) : name_(std::move(name)) {}
  const std::string& to_string() const { return qualified_name(); }
  const std::string& qualified_name() const { return name_; }
  const std::string& short_name() const { return name_; }

 private:
  std::string name_;
};

class Type {
 public:
  explicit Type(Identifier id) : id_(std::move(id)) {}
  const Identifier& id() const { return id_; }

 private:
  Identifier id_;
};

}  // namespace ast
