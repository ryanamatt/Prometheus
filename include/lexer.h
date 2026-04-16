#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "prometheus_types.h"
#include "exceptions.h"

/**
 * @brief Performs lexical analysis on a source file to produce a token stream.
 *
 * Error handling: all lexical errors throw a subclass of LexerException.
 * The exceptions carry a 1-based line number pointing to where in the source
 * the problem was detected.
 *
 * @throws UnknownCharException        if an unrecognised character is encountered.
 * @throws UnterminatedStringException if a string literal has no closing '"'.
 * @throws InvalidNumberException      if a numeric literal is malformed (e.g. "3.14.15").
 */
class Lexer {

private:
    std::string source;
    std::vector<Token> tokens;
    int current_pos = 0;
    int current_line = 1; //< 1-based line counter, incremented on every '\n'.

public:
    explicit Lexer(std::string source) : source(std::move(source)), current_pos(0), current_line(1) {}

    /**
     * @brief Processes the source string and returns a list of Token objects.
     *
     * @throws LexerException (or a subclass) on any lexical error.
     */
    std::vector<Token> tokenize();

private:
    /**
     * @brief Extracts and returns an alphanumeric identifier or keyword token.
     */
    Token make_identifier();

    /**
     * @brief Extracts and returns a numeric (integer or float) token.
     *
     * @throws InvalidNumberException if the number contains more than one
     *         decimal point, or ends with a bare decimal point.
     */
    Token make_number();

    /**
     * @brief Extracts and returns a string literal token, consuming the
     *        surrounding double-quote characters.
     *
     * @throws UnterminatedStringException if EOF is reached before the
     *         closing '"' is found.
     */
    Token make_string();
};

#endif // LEXER_H