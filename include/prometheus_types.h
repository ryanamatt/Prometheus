#ifndef PROMETHEUS_TYPES_H
#define PROMETHEUS_TYPES_H

#include <iostream>
#include <string>
#include <variant>

/**
 * @brief The canonical runtime value type used throughout the interpreter.
 *
 * Alternatives:
 *   int          – integer literals and integer arithmetic results
 *   double       – floating-point literals and float arithmetic results
 *   bool         – results of comparison / logical operators
 *   std::string  – string literals and string variables
 *   std::monostate – represents the absence of a value ("None" / void return)
 */
using PrometheusValue = std::variant<int, double, bool, std::string, std::monostate>;
/**
 * @brief Enumeration of all valid token types supported by the 
    Prometheus Lexer and Parser.
 */
enum class TokenType
{
    // Types
    INT,
    STR,
    DOUBLE,

    // Identifiers and Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // Keywords
    IF,
    ELIF,
    ELSE,
    PRINT,
    INPUT,

    // Symbols
    ASSIGN,         // =
    NOT,            // !

    // Math Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    MODULO,         // %
    EXPONENT,       // **

    INCREMENT,      // ++
    DECREMENT,      // --

    // Comparison Operators
    EQUAL,          // ==
    NOTEQUAL,       // !=
    GREATER,        // >
    LESSER,         // <
    GREATEREQ,      // >=
    LESSEREQ,       // <=
    AND,            // &&
    OR,             // ||

    // Loops
    WHILE,          // WHILE
    FOR,            // FOR

    // Funcs
    FUNC,           // FUNC
    RETURN,         // RETURN

    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }

    SEMICOLON,       // ;
    COMMA,           // ,
    EOF_TOKEN,       // EOF
};

/**
 * @brief Helper function to convert enum to string for debugging
 */
inline std::string to_string(TokenType t) {
    switch (t) {
        // Types
        case TokenType::INT:        return "INT";
        case TokenType::STR:        return "STR";
        case TokenType::DOUBLE:     return "DOUBLE";

        // Identifiers and Literals
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::STRING:     return "STRING";

        // Keywords
        case TokenType::IF:         return "IF";
        case TokenType::ELIF:       return "ELIF";
        case TokenType::ELSE:       return "ELSE";
        case TokenType::PRINT:      return "PRINT";
        case TokenType::INPUT:      return "INPUT";

        // Symbols
        case TokenType::ASSIGN:     return "ASSIGN";
        case TokenType::NOT:        return "NOT";

        // Math Operators
        case TokenType::PLUS:       return "PLUS";
        case TokenType::MINUS:      return "MINUS";
        case TokenType::MULTIPLY:   return "MULTIPLY";
        case TokenType::DIVIDE:     return "DIVIDE";
        case TokenType::MODULO:     return "MODULO";
        case TokenType::EXPONENT:   return "EXPONENT";

        case TokenType::INCREMENT:   return "INCREMENT";

        // Comparison Operators
        case TokenType::EQUAL:      return "EQUAL";
        case TokenType::NOTEQUAL:   return "NOTEQUAL";
        case TokenType::GREATER:    return "GREATER";
        case TokenType::LESSER:     return "LESSER";
        case TokenType::GREATEREQ:  return "GREATEREQ";
        case TokenType::LESSEREQ:   return "LESSEREQ";
        case TokenType::AND:        return "AND";
        case TokenType::OR:         return "OR";

        // Loops
        case TokenType::WHILE:      return "WHILE";
        case TokenType::FOR:        return "FOR";

        // Funcs
        case TokenType::FUNC:       return "FUNC";
        case TokenType::RETURN:     return "RETURN";

        // Delimiters
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::LBRACE:     return "LBRACE";
        case TokenType::RBRACE:     return "RBRACE";
        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::COMMA:      return "COMMA";
        case TokenType::EOF_TOKEN:  return "EOF";

        default:                    return "UNKNOWN";
    }
}

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

    void print() const {
        std::cout << "Token(" << to_string(token) << ", '" << value << "')" << std::endl;
    }
};

#endif // PROMETHEUS_TYPES_H