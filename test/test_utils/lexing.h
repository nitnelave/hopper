#pragma once

#include <vector>

#include "lexer/lexer.h"

lexer::Range make_range(int line1, int col1, int line2, int col2,
                        const std::string& file = "<string>");

/// Read all the tokens from the lexer until EOF into a vector.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<lexer::Token>> consume_tokens(lexer::Lexer& lexer);

/// Lex a string and return the tokens.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<lexer::Token>> string_to_tokens(const std::string& input);

/// Lex a file and return the tokens.
/// Returns an error if one was generated at any time.
ErrorOr<std::vector<lexer::Token>> file_to_tokens(const std::string& filename);
