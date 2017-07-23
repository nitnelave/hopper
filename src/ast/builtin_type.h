#pragma once

#include "ast/type_declaration.h"
#include "visitor/visitor.h"

namespace ast {
namespace internals {
extern const lexer::Range builtin_range;
}  // namespace internals

class BuiltinType : public TypeDeclaration {
 public:
  explicit BuiltinType(Identifier id)
      : TypeDeclaration(internals::builtin_range, std::move(id)) {}

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

namespace types {

// The order is important, as it allows to use max.
enum class IntWidth {
  W_8,
  W_16,
  W_32,
  W_64,
};

extern const BuiltinType void_type;
extern const BuiltinType boolean;
extern const BuiltinType int8;
extern const BuiltinType int16;
extern const BuiltinType int32;
extern const BuiltinType int64;

const TypeDeclaration* width_to_int(IntWidth width);
Option<IntWidth> int_to_width(const TypeDeclaration* type);
}  // namespace types

}  // namespace ast
