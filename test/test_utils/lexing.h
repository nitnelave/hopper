#pragma once

#include <vector>

#include "lexer/lexer.h"

namespace lexer {
/// Read all the tokens from the lexer until EOF into a vector.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<Token>> consume_tokens(Lexer& lexer);

/// Lex a string and return the tokens.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<Token>> string_to_tokens(const std::string& input);

/// Lex a file and return the tokens.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<Token>> file_to_tokens(const std::string& filename);
}  // namespace lexer
