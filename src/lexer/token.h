#pragma once

#include <assert.h>
#include <sstream>
#include <string>

namespace lexer {

#define TOKEN_ENUM(X)                                 \
  X(END_OF_FILE, "EOF")                               \
  /* Identifiers. */                                  \
  X(LOWER_CASE_IDENT, "identifier")                   \
  X(MACRO_IDENT, "#macro")                            \
  X(UPPER_CASE_IDENT, "Identifier")                   \
  /* Literals. */                                     \
  X(INT, "int_literal")                               \
  X(HEX, "hex_literal")                               \
  X(OCT, "oct_literal")                               \
  X(BINARY_NUMBER, "bin_literal")                     \
  X(FLOAT, "float_literal")                           \
  X(STRING, "string")                                 \
  X(RAWSTRING, "rawstring")                           \
  X(CODESTRING, "codestring")                         \
  X(RAWCODESTRING, "rawcodestring")                   \
  /* Binary operators. */                             \
  X(PLUS, "+")                                        \
  X(MINUS, "-")                                       \
  X(DIVIDE, "/")                                      \
  X(STAR, "*")                                        \
  X(OR, "||")                                         \
  X(AND, "&&")                                        \
  X(GREATER, ">")                                     \
  X(LESS, "<")                                        \
  X(GREATER_OR_EQUAL, ">=")                           \
  X(LESS_OR_EQUAL, "<=")                              \
  X(EQUAL, "==")                                      \
  X(DIFFERENT, "!=")                                  \
  X(BITSHIFT_RIGHT, "|>")                             \
  X(BITSHIFT_LEFT, "<|")                              \
  X(BITOR, "|")                                       \
  X(BITXOR, "^")                                      \
  X(ASSIGN, "=")                                      \
  X(PLUS_ASSIGN, "+=")                                \
  X(MINUS_ASSIGN, "-=")                               \
  X(TIMES_ASSIGN, "*=")                               \
  X(DIVIDE_ASSIGN, "/=")                              \
  X(OR_ASSIGN, "|=")                                  \
  X(XOR_ASSIGN, "^=")                                 \
  X(AND_ASSIGN, "&=")                                 \
  /* Unary operators. */                              \
  X(AMPERSAND, "&")                                   \
  X(BANG, "!")                                        \
  X(TILDE, "~")                                       \
  X(INCREMENT, "++")                                  \
  X(DECREMENT, "--")                                  \
  /* Delimiters. */                                   \
  X(OPEN_PAREN, "(")                                  \
  X(CLOSE_PAREN, ")")                                 \
  X(OPEN_BRACKET, "[")                                \
  X(CLOSE_BRACKET, "]")                               \
  X(OPEN_BRACE, "{")                                  \
  X(CLOSE_BRACE, "}")                                 \
  X(SEMICOLON, ";")                                   \
  X(COLON, ":")                                       \
  X(COMMA, ",")                                       \
  X(ARROW, "->")                                      \
  /* Keywords (alphabetical order). */                \
  X(__KEYWORDS_START__, "") /* Not a token */         \
  X(AS, "as")                                         \
  X(DO, "do")                                         \
  X(ENUM, "enum")                                     \
  X(DATA, "data")                                     \
  X(CLASS, "class")                                   \
  X(PRIVATE, "private")                               \
  X(PUBLIC, "public")                                 \
  X(FROM, "from")                                     \
  X(FOR, "for")                                       \
  X(FORWARD, "forward") /* for forward-references. */ \
  X(FUN, "fun")                                       \
  X(IF, "if")                                         \
  X(IMPORT, "import")                                 \
  X(IS, "is")                                         \
  X(MUT, "mut")                                       \
  X(RETURN, "return")                                 \
  X(VAL, "val")                                       \
  X(WHEN, "when")                                     \
  X(WHILE, "while")                                   \
  X(__KEYWORDS_END__, "") /* Not a token */           \
  /* Misc. */                                         \
  X(ANNOTATION, "@annotation")                        \
  X(COMMENT, "// comment")                            \
  X(MACRO_CALL, "#macro(arguments)")

enum class TokenType : int {
#define MAKE_ENUM(VAR, TEXT) VAR,
  TOKEN_ENUM(MAKE_ENUM)
#undef MAKE_ENUM
      __NUMBER_TOKENS__
};

namespace internals {
const char* const token_type_names[] = {
#define MAKE_STRINGS(VAR, TEXT) #VAR,
    TOKEN_ENUM(MAKE_STRINGS)
#undef MAKE_STRINGS
};

const char* const token_type_symbols[] = {
#define MAKE_STRINGS(VAR, TEXT) TEXT,
    TOKEN_ENUM(MAKE_STRINGS)
#undef MAKE_STRINGS
};
}  // namespace internals
#undef TOKEN_ENUM

namespace internals {
inline int token_type_to_index(TokenType t) {
  int index = static_cast<int>(t);
  assert(index >= 0 && index < static_cast<int>(TokenType::__NUMBER_TOKENS__) &&
         t != TokenType::__KEYWORDS_START__ &&
         t != TokenType::__KEYWORDS_END__);
  return index;
}
}  // namespace internals

inline std::string to_string(TokenType t) {
  return internals::token_type_names[internals::token_type_to_index(t)];
}

inline std::string to_symbol(TokenType t) {
  return internals::token_type_symbols[internals::token_type_to_index(t)];
}

inline std::ostream& operator<<(std::ostream& os, TokenType tt) {
  return os << to_string(tt);
}

struct Location {
  const std::string file;
  int line;
  int column;

  std::string to_string() const {
    std::stringstream ss;
    ss << file << ':' << line << ':' << column;
    return ss.str();
  }
};

inline std::ostream& operator<<(std::ostream& os, const Location& loc) {
  return os << loc.to_string();
}

struct Token {
  const TokenType type;
  const std::string text;
  const Location begin;
  const Location end;
};

inline std::ostream& operator<<(std::ostream& os, const Token& tok) {
  os << "{" << tok.type;
  if (!tok.text.empty()) os << ": " << tok.text;
  os << "at " << tok.begin << " to " << tok.end.line << ":" << tok.end.column;
  return os;
}

}  // namespace lexer
