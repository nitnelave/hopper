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
    char normalized = c - start;
    if (0 <= normalized && normalized < base - 10) return normalized + 10;
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

Lexer::Lexer(const std::string& source, SourceTag tag,
             const std::string& filename)
    : stream_{get_stream_from_source(source, tag)},
      location_{filename, 1, 0},
      previous_location_(location_) {}

ErrorOr<Token, LexError> Lexer::read_lowercase_identifier() {
  RETURN_OR_ASSIGN(Token tok, read_identifier(TokenType::LOWER_CASE_IDENT));
  // Check for keywords.
  for (int i = static_cast<int>(TokenType::__KEYWORDS_START__) + 1;
       i < static_cast<int>(TokenType::__KEYWORDS_END__); ++i) {
    TokenType tt = static_cast<TokenType>(i);
    if (tok.text == to_symbol(tt)) return Token{tt, tok.text, tok.location};
  }
  return tok;
}

ErrorOr<Token, LexError> Lexer::read_identifier(TokenType tt) {
  Location beginning = get_location();
  std::stringstream ss;
  while (is_alpha_num(next_char_)) {
    ss << next_char_;
    get_next_char();
  }
  unget_char();
  return Token{tt, ss.str(), {beginning, get_location()}};
}

ErrorOr<Token, LexError> Lexer::get_next_token() {
  get_next_char();
  // Skip blanks.
  while (next_char_ == ' ' || next_char_ == '\n') get_next_char();

  // Save initial location.
  Location beginning = get_location();
  // Save initial lookup character.
  char current = next_char_;

  // Construct a token with the last character.
  auto make_single_token = [&, this](TokenType tt) {
    return ErrorOr<Token, LexError>{
        Token{tt, {current}, {beginning, beginning}}};
  };

  // Construct a token with the last 2 characters.
  auto make_double_token = [&, this](TokenType tt) {
    return ErrorOr<Token, LexError>{
        Token{tt, {current, this->next_char_}, {beginning, get_location()}}};
  };

  // Test the next character for each of the mappings and return the
  // corresponding token, if matching. Otherwise, return a token of type
  // default_type.
  auto with_second_char = [&, this](
      TokenType default_type,
      const std::initializer_list<std::pair<char, TokenType>>& mappings) {
    if (mappings.size() > 0) {
      get_next_char();
      for (const auto& m : mappings)
        if (m.first == next_char_) return make_double_token(m.second);
      unget_char();
    }
    return make_single_token(default_type);
  };

  // LEXER STARTS.

  switch (next_char_) {
    case '#':
      // TODO: macro
      return LexError("Macros are unimplemented", beginning);
    case '0':
      get_next_char();
      switch (next_char_) {
        case 'x':
          return read_base(beginning, TokenType::HEX, 16);
        case 'o':
          return read_base(beginning, TokenType::OCT, 8);
        case 'b':
          return read_base(beginning, TokenType::BINARY_NUMBER, 2);
      }
      do {
        get_next_char();
      } while (is_alpha_num(next_char_));
      unget_char();
      return LexError("Invalid number literal", {beginning, get_location()});
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
      // TODO: string
      return LexError("String is unimplemented", beginning);
    case 'r':
      get_next_char();
      if (next_char_ == '"')
        return LexError("String is unimplemented", beginning);
      unget_char();
      break;  // Normal identifier, defer to after switch.
    case '+':
      return with_second_char(TokenType::PLUS, {{'+', TokenType::INCREMENT},
                                                {'=', TokenType::PLUS_ASSIGN}});
    case '-':
      return with_second_char(TokenType::MINUS, {{'-', TokenType::DECREMENT},
                                                 {'=', TokenType::MINUS_ASSIGN},
                                                 {'=', TokenType::ARROW}});
    case '/':
      get_next_char();
      switch (next_char_) {
        case '/':
          return read_comment(beginning);
        case '=':
          return make_double_token(TokenType::DIVIDE_ASSIGN);
      }
      unget_char();
      return make_single_token(TokenType::DIVIDE);
    case '*':
      return with_second_char(TokenType::STAR,
                              {{'=', TokenType::TIMES_ASSIGN}});
    case '|':
      return with_second_char(TokenType::BITOR,
                              {{'|', TokenType::OR},
                               {'=', TokenType::OR_ASSIGN},
                               {'>', TokenType::BITSHIFT_RIGHT}});
    case '&':
      return with_second_char(
          TokenType::AMPERSAND,
          {{'&', TokenType::AND}, {'=', TokenType::AND_ASSIGN}});
    case '>':
      return with_second_char(TokenType::GREATER,
                              {{'=', TokenType::GREATER_OR_EQUAL}});
    case '<':
      return with_second_char(
          TokenType::LESS,
          {{'|', TokenType::BITSHIFT_LEFT}, {'=', TokenType::LESS_OR_EQUAL}});
    case '=':
      return with_second_char(TokenType::ASSIGN, {{'=', TokenType::EQUAL}});
    case '!':
      return with_second_char(TokenType::BANG, {{'=', TokenType::DIFFERENT}});
    case '^':
      return with_second_char(TokenType::BITXOR,
                              {{'=', TokenType::XOR_ASSIGN}});
    case '~':
      return make_single_token(TokenType::TILDE);
    case '(':
      return make_single_token(TokenType::OPEN_PAREN);
    case ')':
      return make_single_token(TokenType::CLOSE_PAREN);
    case '[':
      return make_single_token(TokenType::OPEN_BRACKET);
    case ']':
      return make_single_token(TokenType::CLOSE_BRACKET);
    case '{':
      return make_single_token(TokenType::OPEN_BRACE);
    case '}':
      return make_single_token(TokenType::CLOSE_BRACE);
    case ';':
      return make_single_token(TokenType::SEMICOLON);
    case ':':
      return make_single_token(TokenType::COLON);
    case ',':
      return make_single_token(TokenType::COMMA);
    case EOF:
      return make_single_token(TokenType::END_OF_FILE);
  }
  if (is_lowercase(next_char_)) return read_lowercase_identifier();
  if (is_uppercase(next_char_))
    return read_identifier(TokenType::UPPER_CASE_IDENT);
  return LexError(
      std::string("Unrecognized character: '").append(1, next_char_) + "' ",
      beginning);
}

ErrorOr<Token, LexError> Lexer::read_comment(const Location& beginning) {
  std::stringstream ss;
  ss << '/';
  while (next_char_ != '\n' && next_char_ != EOF) {
    ss << next_char_;
    get_next_char();
  }
  unget_char();
  return Token{TokenType::COMMENT, ss.str(), {beginning, get_location()}};
}

ErrorOr<Token, LexError> Lexer::read_base(const Location& beginning,
                                          TokenType tt, int base) {
  int64_t result = 0;
  bool saw_digit = false;
  while (true) {
    get_next_char();
    int value = value_of_char(next_char_, base);
    if (value == -1) {
      break;
    }
    saw_digit = true;
    result *= base;
    result += value;
  }
  if (!saw_digit || is_alpha_num(next_char_))
    return LexError("Invalid number literal", {beginning, get_location()});
  unget_char();
  auto result_token =
      Token{tt, std::to_string(result), {beginning, get_location()}};
  return std::move(result_token);
}

void Lexer::get_next_char() {
  if (was_not_consumed_) {
    std::swap(next_char_, previous_char_);
    was_not_consumed_ = false;
    return;
  }
  // Keep returning EOF at the end of the file.
  if (next_char_ == EOF) return;
  previous_location_.line = location_.line;
  previous_location_.column = location_.column;
  if (next_char_ == '\n') {
    ++location_.line;
    location_.column = 0;
  }
  ++location_.column;
  previous_char_ = next_char_;
  if (!stream_->get(next_char_)) next_char_ = EOF;
}

void Lexer::unget_char() {
  if (was_not_consumed_)
    throw std::domain_error(
        "Next char was already unget_char(), "
        "cannot unget two chars");
  std::swap(next_char_, previous_char_);
  was_not_consumed_ = true;
}

const Location& Lexer::get_location() const {
  if (was_not_consumed_) return previous_location_;
  return location_;
}

Lexer from_file(const std::string& file) {
  return Lexer(file, Lexer::SourceTag::FILE, file);
}

Lexer from_string(const std::string& text) {
  return Lexer(text, Lexer::SourceTag::STRING, "<string>");
}

}  // namespace lexer
