#pragma once

#include <vector>

#include "ast/type_declaration.h"
#include "visitor/visitor.h"

namespace ast {
namespace internals {
extern const lexer::Range builtin_range;
}  // namespace internals

class BuiltinType : public TypeDeclaration {
 public:
  explicit BuiltinType(Identifier id)
      : TypeDeclaration(internals::builtin_range, NodeType::BUILTIN_TYPE,
                        std::move(id)) {}

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

// Conceptually they are const, but in practice we need them mutable for the
// visitors.
extern BuiltinType void_type;
extern BuiltinType boolean;
extern BuiltinType int8;
extern BuiltinType int16;
extern BuiltinType int32;
extern BuiltinType int64;

extern const std::vector<BuiltinType*> builtin_types;

const TypeDeclaration* width_to_int_type(IntWidth width);
Option<IntWidth> int_type_to_width(const TypeDeclaration* type);
}  // namespace types

}  // namespace ast
