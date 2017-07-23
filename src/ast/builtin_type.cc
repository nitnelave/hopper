#include "ast/builtin_type.h"
namespace ast {
namespace internals {
const lexer::Range builtin_range{"<builtin>", {0, 0}, {0, 0}};
inline BuiltinType make_builtin(const std::string& name) {
  return BuiltinType(Identifier(name, builtin_range, true, false));
}
}  // namespace internals

namespace types {
const BuiltinType void_type = internals::make_builtin("Void");
const BuiltinType boolean = internals::make_builtin("Bool");
const BuiltinType int8 = internals::make_builtin("Int8");
const BuiltinType int16 = internals::make_builtin("Int16");
const BuiltinType int32 = internals::make_builtin("Int32");
const BuiltinType int64 = internals::make_builtin("Int64");

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
}
}  // namespace types
}  // namespace ast
