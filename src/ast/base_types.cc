#include "ast/base_types.h"
#include "ast/type_declaration.h"

namespace ast {

const Identifier& Type::id() const {
  if (id_.is_ok()) return id_.value_or_die();
  // If no Identifier is given, then we have a TypeDeclaration.
  return type_.value_or_die()->id();
}

}  // namespace ast
