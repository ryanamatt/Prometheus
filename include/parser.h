#ifndef PARSE_H
#define PARSE_H

#include "ast_nodes.h"

class Parser {
private:
    std::vector<Token> tokens;
    int pos;

public:
    Parser(std::vector<Token> tokens) : tokens(tokens), pos(0) {}

    /**
     * @brief Entry point for parsing the entire token stream into a list of statements.
     */
    std::vector<ASTNode> parse();

private:

    /**
     * @brief Returns the token currently pointed to by the parser.
     */
    Token current_token();

    /**
     * @brief Looks at the next token without consuming it.
     */
    Token peek();

    /**
     * @brief Validates the current token type and moves the pointer forward.
        Raises a Syntax Error if the type does not match.
     */
    ASTNode eat(TokenType token_type);

    /**
     * @brief Determines the type of statement and routes to the specific parsing method.
     */
    ASTNode parse_statement();

    /**
     * @brief Parses variable declarations (e.g., 'int x = 10;').
     */
    ASTNode parse_declaration();

    /**
     * @brief Parses an Identifier of x = x + 1
     */
    ASTNode parse_identifier();

    /**
     * @brief Parses the lowest level of expression precedence (comparisons).
     */
    ASTNode parse_expression();

    /**
     * @brief Parses additive operations (+, -).
     */
    ASTNode parse_add_sub();

    /**
     * @brief Parses multiplicative operations (*, /, %).
     */
    ASTNode parse_factor();

    /**
     * @brief Parses exponentiation operations (**).
     */
    ASTNode parse_exponenet();

    /**
     * @brief Handles grouped expressions inside parentheses.
     */
    ASTNode parse_parantheses();

    /**
     * @brief Parses the highest priority elements (numbers, strings, identifiers).
     */
    ASTNode parse_term();

    /**
     * @brief Parses 'print' statements and their arguments.
     */
    PrintNode parse_print();

    /**
     * @brief Parses 'if' statements, including optional 'else' blocks.
     */
    IfNode parse_if();

    /**
     * @brief Parses a While Loop
     */
    WhileNode parse_while();

    /**
     * @brief Parses a For Loop.
     */
    ForNode parse_for();

    /**
     * @brief Parses a function
     */
    FunctionDeclNode parse_func();

    /**
     * @brief Parses the return keyword
     */
    ReturnNode parse_return();

    /**
     * @brief Parses the call to a function
     */
    CallNode parse_call();
};

#endif // PARSER_H