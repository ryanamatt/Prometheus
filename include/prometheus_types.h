

#include <string>

/**
 * @brief Enumeration of all valid token types supported by the 
    Prometheus Lexer and Parser.
 */
enum TokenType
{
    INT,
    STR,
    DOUBLE,

    // Identifiers and Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // Keywords

    // Symbols
    ASSIGN,         // =

    // Math Operators

    // Comparison Operators

    // Loops

    // Funcs

    SEMICOLN,       // ;
    EOF_TOKEN,            // EOF
};

/**
 * @brief A small container representing a single meaningful unit of code (lexemes).
 */
class Token {

private:
    TokenType token;
    std::string value;

public:
    Token(TokenType token, std::string value) : token(token), value(value) {}
    
    TokenType get_token() { return token; }
    std::string get_value() { return value; }
};