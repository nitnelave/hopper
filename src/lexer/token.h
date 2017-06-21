#pragma once

#include <assert.h>
#include <sstream>
#include <string>

namespace lexer {

#define TOKEN_ENUM(DO)                                 \
  DO(END_OF_FILE, "EOF")                               \
  /* Identifiers. */                                   \
  DO(LOWER_CASE_IDENT, "identifier")                   \
  DO(MACRO_IDENT, "#macro")                            \
  DO(UPPER_CASE_IDENT, "Identifier")                   \
  /* Literals. */                                      \
  DO(INT, "int_literal")                               \
  DO(HEX, "hex_literal")                               \
  DO(OCT, "oct_literal")                               \
  DO(BINARY_NUMBER, "bin_literal")                     \
  DO(FLOAT, "float_literal")                           \
  DO(STRING, "string")                                 \
  DO(RAWSTRING, "rawstring")                           \
  DO(CODESTRING, "codestring")                         \
  DO(RAWCODESTRING, "rawcodestring")                   \
  /* Binary operators. */                              \
  DO(PLUS, "+")                                        \
  DO(MINUS, "-")                                       \
  DO(DIVIDE, "/")                                      \
  DO(STAR, "*")                                        \
  DO(OR, "||")                                         \
  DO(AND, "&&")                                        \
  DO(GREATER, ">")                                     \
  DO(LESS, "<")                                        \
  DO(GREATER_OR_EQUAL, ">=")                           \
  DO(LESS_OR_EQUAL, "<=")                              \
  DO(EQUAL, "==")                                      \
  DO(DIFFERENT, "!=")                                  \
  DO(BITSHIFT_RIGHT, "|>")                             \
  DO(BITSHIFT_LEFT, "<|")                              \
  DO(BITOR, "|")                                       \
  DO(BITXOR, "^")                                      \
  DO(ASSIGN, "=")                                      \
  DO(PLUS_ASSIGN, "+=")                                \
  DO(MINUS_ASSIGN, "-=")                               \
  DO(TIMES_ASSIGN, "*=")                               \
  DO(DIVIDE_ASSIGN, "/=")                              \
  DO(OR_ASSIGN, "|=")                                  \
  DO(XOR_ASSIGN, "^=")                                 \
  DO(AND_ASSIGN, "&=")                                 \
  /* Unary operators. */                               \
  DO(AMPERSAND, "&")                                   \
  DO(BANG, "!")                                        \
  DO(TILDE, "~")                                       \
  DO(INCREMENT, "++")                                  \
  DO(DECREMENT, "--")                                  \
  /* Delimiters. */                                    \
  DO(OPEN_PAREN, "(")                                  \
  DO(CLOSE_PAREN, ")")                                 \
  DO(OPEN_BRACKET, "[")                                \
  DO(CLOSE_BRACKET, "]")                               \
  DO(OPEN_BRACE, "{")                                  \
  DO(CLOSE_BRACE, "}")                                 \
  DO(SEMICOLON, ";")                                   \
  DO(COLON, ":")                                       \
  DO(COMMA, ",")                                       \
  DO(ARROW, "->")                                      \
  /* Keywords (alphabetical order). */                 \
  DO(AS, "as")                                         \
  DO(DO, "do")                                         \
  DO(ENUM, "enum")                                     \
  DO(DATA, "data")                                     \
  DO(CLASS, "class")                                   \
  DO(PRIVATE, "private")                               \
  DO(PUBLIC, "public")                                 \
  DO(FROM, "from")                                     \
  DO(FOR, "for")                                       \
  DO(FORWARD, "forward") /* for forward-references. */ \
  DO(FUN, "fun")                                       \
  DO(IF, "if")                                         \
  DO(IMPORT, "import")                                 \
  DO(IS, "is")                                         \
  DO(MUT, "mut")                                       \
  DO(RETURN, "return")                                 \
  DO(VAL, "val")                                       \
  DO(WHEN, "when")                                     \
  DO(WHILE, "while")                                   \
  /* Mics. */                                          \
  DO(ANNOTATION, "@annotation")                        \
  DO(COMMENT, "// comment")                            \
  DO(MACRO_CALL, "#macro(arguments)")

enum class TokenType : int {
#define MAKE_ENUM(VAR, TEXT) VAR,
  TOKEN_ENUM(MAKE_ENUM)
#undef MAKE_ENUM
      __NUMBER_TOKENS
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
  assert(index >= 0 && index < static_cast<int>(TokenType::__NUMBER_TOKENS));
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
