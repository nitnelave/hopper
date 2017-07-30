#include "ast/builtin_type.h"

namespace ast {
namespace internals {
const lexer::Range builtin_range{"<builtin>", {0, 0}, {0, 0}};
inline BuiltinType make_builtin(const std::string& name) {
  return BuiltinType(Identifier(name, builtin_range, true, false));
}
}  // namespace internals

namespace types {
BuiltinType void_type = ::ast::internals::make_builtin("Void");
BuiltinType boolean = ::ast::internals::make_builtin("Bool");
BuiltinType int8 = ::ast::internals::make_builtin("Int8");
BuiltinType int16 = ::ast::internals::make_builtin("Int16");
BuiltinType int32 = ::ast::internals::make_builtin("Int32");
BuiltinType int64 = ::ast::internals::make_builtin("Int64");
const std::vector<BuiltinType*> builtin_types = {
    &void_type, &boolean, &int8, &int16, &int32, &int64,
};

Option<IntWidth> int_type_to_width(const TypeDeclaration* type) {
  if (type == &int64) return IntWidth::W_64;
  if (type == &int32) return IntWidth::W_32;
  if (type == &int16) return IntWidth::W_16;
  if (type == &int8) return IntWidth::W_8;
  return none;
}

const TypeDeclaration* width_to_int_type(IntWidth width) {
  switch (width) {
    case IntWidth::W_8:
      return &int8;
    case IntWidth::W_16:
      return &int16;
    case IntWidth::W_32:
      return &int32;
    case IntWidth::W_64:
      return &int64;
  }

  return nullptr;
}
}  // namespace types
}  // namespace ast
