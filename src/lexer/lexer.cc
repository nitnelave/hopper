#include "lexer/lexer.h"

#include <fstream>
#include <initializer_list>
#include <sstream>
#include <stdexcept>

namespace lexer {
namespace {
std::unique_ptr<std::istream> get_stream_from_source(const std::string& source,
                                                     Lexer::SourceTag tag) {
  switch (tag) {
    case Lexer::SourceTag::FILE:
      return std::make_unique<std::ifstream>(source);
    case Lexer::SourceTag::STRING:
      return std::make_unique<std::stringstream>(source);
    default:
      // unreachable
      throw std::domain_error("Invalid tag when building the lexer");
  }
}

int value_of_char(char c, int base) {
  char normalized = c - '0';
  if (0 <= normalized && normalized < base && normalized < 10)
    return normalized;
  if (base > 10 && normalized >= 10) {
    char start;
    if (c >= 'A' && c <= 'F')
      start = 'A';
    else if (c >= 'a' && c <= 'f')
      start = 'a';
    else
      return -1;
    return c - start + 10;
  }
  return -1;
}

bool is_lowercase(char c) { return (c >= 'a' && c <= 'z'); }

bool is_uppercase(char c) { return (c >= 'A' && c <= 'Z'); }

bool is_alpha_num(char c, bool underscore = true) {
  return (c >= '0' && c <= '9') || is_uppercase(c) || is_lowercase(c) ||
         (underscore && c == '_');
}
}  // namespace

namespace internals {
class LexerHelper {
 public:
  explicit LexerHelper(Lexer* lexer)
      : lexer_(lexer),
        beginning_(lexer_->location()),
        first_char_(lexer_->current_char()) {}

  // Construct a token with the last character.
  ErrorOr<Token, LexError> make_single_token(TokenType tt) const {
    return Token(tt, std::string(1, lexer_->current_char()),
                 {beginning_, beginning_});
  };

  // Construct a token with the last 2 characters.
  ErrorOr<Token, LexError> make_double_token(TokenType tt) const {
    return Token{tt,
                 {first_char_, lexer_->current_char()},
                 {beginning_, lexer_->location()}};
  };

  // Test the next character for each of the mappings and return the
  // corresponding token, if matching. Otherwise, return a token of type
  // default_type.
  ErrorOr<Token, LexError> with_second_char(
      TokenType default_type,
      const std::initializer_list<std::pair<char, TokenType>>& mappings) const {
    if (mappings.size() > 0) {
      RETURN_IF_ERROR(lexer_->get_next_char());
      for (const auto& m : mappings)
        if (m.first == lexer_->current_char())
          return make_double_token(m.second);
      lexer_->unget_char();
    }
    return make_single_token(default_type);
  };

 private:
  Lexer* lexer_;
  Location beginning_;
  char first_char_;
};
}  // namespace internals

Lexer::Lexer(const std::string& source, SourceTag tag,
             const std::string& filename)
    : reader_{get_stream_from_source(source, tag), filename} {}

ErrorOr<Token, LexError> Lexer::read_lowercase_identifier() {
  RETURN_OR_MOVE(Token tok, read_identifier(TokenType::LOWER_CASE_IDENT));
  // Check for keywords.
  for (int i = static_cast<int>(TokenType::__KEYWORDS_START__) + 1;
       i < static_cast<int>(TokenType::__KEYWORDS_END__); ++i) {
    TokenType tt = static_cast<TokenType>(i);
    if (tok.text() == to_symbol(tt))
      return Token{tt, tok.text(), tok.location()};
  }
  return std::move(tok);
}

ErrorOr<Token, LexError> Lexer::read_identifier(TokenType tt) {
  Location beginning = location();
  std::stringstream ss;
  while (is_alpha_num(current_char())) {
    ss << current_char();
    RETURN_IF_ERROR(get_next_char());
  }
  unget_char();
  return Token{tt, ss.str(), {beginning, location()}};
}

ErrorOr<Token, LexError> Lexer::get_next_token() {
  RETURN_IF_ERROR(get_next_char());
  // Skip blanks.
  while (current_char() == ' ' || current_char() == '\n')
    RETURN_IF_ERROR(get_next_char());

  // Save initial location.
  Location beginning = location();
  internals::LexerHelper helper(this);

  // LEXER STARTS.

  switch (current_char()) {
    case '#':
      // TODO: macro
      return LexError("Macros are unimplemented", {beginning, beginning});
    case '0':
      RETURN_IF_ERROR(get_next_char());
      switch (current_char()) {
        case 'x':
          return read_base(beginning, TokenType::HEX, 16);
        case 'o':
          return read_base(beginning, TokenType::OCT, 8);
        case 'b':
          return read_base(beginning, TokenType::BINARY_NUMBER, 2);
      }
      if (!is_alpha_num(current_char())) {
        unget_char();
        // Found single '0'.
        return helper.make_single_token(TokenType::INT);
      }
      do {
        RETURN_IF_ERROR(get_next_char());
      } while (is_alpha_num(current_char()));
      unget_char();
      return LexError("Invalid number literal", {beginning, location()});
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      // TODO: float
      unget_char();
      return read_base(beginning, TokenType::INT, 10);
    case '"':
      return read_string(beginning);
    case 'r':
      RETURN_IF_ERROR(get_next_char());
      if (current_char() == '"')
        return LexError("String is unimplemented", {beginning, location()});
      unget_char();
      break;  // Normal identifier, defer to after switch.
    case '+':
      return helper.with_second_char(
          TokenType::PLUS,
          {{'+', TokenType::INCREMENT}, {'=', TokenType::PLUS_ASSIGN}});
    case '-':
      return helper.with_second_char(TokenType::MINUS,
                                     {{'-', TokenType::DECREMENT},
                                      {'=', TokenType::MINUS_ASSIGN},
                                      {'>', TokenType::ARROW}});
    case '/':
      RETURN_IF_ERROR(get_next_char());
      switch (current_char()) {
        case '/':
          return read_comment(beginning);
        case '=':
          return helper.make_double_token(TokenType::DIVIDE_ASSIGN);
      }
      unget_char();
      return helper.make_single_token(TokenType::DIVIDE);
    case '.':
      RETURN_IF_ERROR(get_next_char());
      if (current_char() == '.') {
        RETURN_IF_ERROR(get_next_char());
        if (current_char() == '.')
          return Token{TokenType::DOTDOTDOT, "...", {beginning, location()}};
        unget_char();
        return helper.make_double_token(TokenType::DOTDOT);
      }
      unget_char();
      return helper.make_single_token(TokenType::DOT);
    case '*':
      return helper.with_second_char(TokenType::STAR,
                                     {{'=', TokenType::TIMES_ASSIGN}});
    case '|':
      return helper.with_second_char(TokenType::BITOR,
                                     {{'|', TokenType::OR},
                                      {'=', TokenType::OR_ASSIGN},
                                      {'>', TokenType::BITSHIFT_RIGHT}});
    case '&':
      return helper.with_second_char(
          TokenType::AMPERSAND,
          {{'&', TokenType::AND}, {'=', TokenType::AND_ASSIGN}});
    case '>':
      return helper.with_second_char(TokenType::GREATER,
                                     {{'=', TokenType::GREATER_OR_EQUAL}});
    case '<':
      return helper.with_second_char(
          TokenType::LESS,
          {{'|', TokenType::BITSHIFT_LEFT}, {'=', TokenType::LESS_OR_EQUAL}});
    case '=':
      return helper.with_second_char(TokenType::ASSIGN,
                                     {{'=', TokenType::EQUAL}});
    case '!':
      return helper.with_second_char(TokenType::BANG,
                                     {{'=', TokenType::DIFFERENT}});
    case '^':
      return helper.with_second_char(TokenType::BITXOR,
                                     {{'=', TokenType::XOR_ASSIGN}});
    case '~':
      return helper.make_single_token(TokenType::TILDE);
    case '(':
      return helper.make_single_token(TokenType::OPEN_PAREN);
    case ')':
      return helper.make_single_token(TokenType::CLOSE_PAREN);
    case '[':
      return helper.make_single_token(TokenType::OPEN_BRACKET);
    case ']':
      return helper.make_single_token(TokenType::CLOSE_BRACKET);
    case '{':
      return helper.make_single_token(TokenType::OPEN_BRACE);
    case '}':
      return helper.make_single_token(TokenType::CLOSE_BRACE);
    case ';':
      return helper.make_single_token(TokenType::SEMICOLON);
    case ':':
      return helper.with_second_char(TokenType::COLON,
                                     {{':', TokenType::COLON_COLON}});
    case '?':
      RETURN_IF_ERROR(get_next_char());
      if (current_char() == '.')
        return helper.make_double_token(TokenType::QUESTION_MARK_DOT);
      if (current_char() == ':')
        return helper.make_double_token(TokenType::QUESTION_MARK_COLON);
      if (current_char() == '-') {
        RETURN_IF_ERROR(get_next_char());
        if (current_char() == '>')
          return Token{
              TokenType::QUESTION_MARK_ARROW, "?->", {beginning, location()}};
        unget_char();
      }
      unget_char();
      return helper.make_single_token(TokenType::QUESTION_MARK);
    case '_':
      return helper.make_single_token(TokenType::UNDERSCORE);
    case ',':
      return helper.make_single_token(TokenType::COMMA);
    case EOF:
      return helper.make_single_token(TokenType::END_OF_FILE);
  }
  if (is_lowercase(current_char())) return read_lowercase_identifier();
  if (is_uppercase(current_char()))
    return read_identifier(TokenType::UPPER_CASE_IDENT);
  return LexError(
      std::string("Unrecognized character: '").append(1, current_char()) + '\'',
      {beginning, beginning});
}

ErrorOr<Token, LexError> Lexer::read_comment(const Location& beginning) {
  std::stringstream ss;
  ss << '/';
  while (current_char() != '\n' && current_char() != EOF) {
    ss << current_char();
    RETURN_IF_ERROR(get_next_char());
  }
  unget_char();
  return Token{TokenType::COMMENT, ss.str(), {beginning, location()}};
}

ErrorOr<Token, LexError> Lexer::read_base(const Location& beginning,
                                          TokenType tt, int base) {
  int64_t result = 0;
  bool saw_digit = false;
  while (true) {
    RETURN_IF_ERROR(get_next_char());
    int value = value_of_char(current_char(), base);
    if (value == -1) {
      break;
    }
    saw_digit = true;
    result *= base;
    result += value;
  }
  // Eat the extra characters at the end of the literal.
  if (!saw_digit || is_alpha_num(current_char())) {
    while (is_alpha_num(current_char())) RETURN_IF_ERROR(get_next_char());
    unget_char();
    return LexError("Invalid number literal", {beginning, location()});
  }
  unget_char();
  return Token{tt, result, {beginning, location()}};
}

/// Reads until valid double quote.
ErrorOr<Token, LexError> Lexer::read_string(const Location& beginning) {
  std::string result = "";
  bool escaped = false;

  // Skip first char.
  while (true) {
    RETURN_IF_ERROR(get_next_char());
    char c = current_char();
    if (c == '"' && !escaped) {
      break;
    } else if (c == '\\') {
      escaped = true;
    } else {
      if (escaped) {
        escaped = false;
        result.append(1, '\\');
      }
      result.append(1, c);
    }
  }

  return Token(TokenType::STRING, result, {beginning, location()});
}

MaybeError<LexError> Lexer::get_next_char() { return char_stack_.get_next(); }

void Lexer::unget_char() { char_stack_.unget(); }

char Lexer::current_char() const { return char_stack_.current().first; }

const Location& Lexer::location() const { return char_stack_.current().second; }

Lexer from_file(const std::string& file) {
  return Lexer(file, Lexer::SourceTag::FILE, file);
}

Lexer from_string(const std::string& text) {
  return Lexer(text, Lexer::SourceTag::STRING, "<string>");
}

}  // namespace lexer
