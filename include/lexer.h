
#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "prometheus_types.h"

/**
 * @brief Performs lexical analysis on a source file to produce a token stream.

 */
class Lexer {

private:
    std::string source;
    std::vector<Token> tokens;
    int current_pos = 0;

public:
    Lexer(std::string source) : source(source) {}

    /**
     * @brief Processes the source string and returns a list of Token objects.
     */
    std::vector<Token> tokenize();

private:
    /**
     * @brief Extracts and returns an alphanumeric identifier or keyword token.
     */
    Token make_identifier();
    
    /**
     * @brief Extracts and returns a numeric (integer or float) token.
     */
    Token make_number();

    /**
     * @brief Extracts and returns a string literal token, handling double quotes.
     */
    Token make_string();
};

#endif // LEXER_H