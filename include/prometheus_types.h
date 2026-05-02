#ifndef PROMETHEUS_TYPES_H
#define PROMETHEUS_TYPES_H

#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

/**
 * @brief The canonical runtime value type used throughout the interpreter.
 *
 * Alternatives:
 *   int          – integer literals and integer arithmetic results
 *   double       – floating-point literals and float arithmetic results
 *   bool         – results of comparison / logical operators
 *   std::string  – string literals and string variables
 *   PrometheusListPtr – a heap-allocated, type-tagged list of values
 *   std::monostate – represents the absence of a value ("None" / void return)
 *
 * Lists are stored behind a shared_ptr so PrometheusValue can remain a
 * value type even though its variant must not directly reference itself.
 */

// Forward-declare so PrometheusValue and PrometheusListPtr can reference each other.
struct PrometheusList;
using PrometheusListPtr = std::shared_ptr<PrometheusList>;

using PrometheusValue = std::variant<int, double, bool, std::string,
                                     PrometheusListPtr, std::monostate>;

/**
 * @brief A runtime list: an element-type tag plus a vector of PrometheusValues.
 * The element type is enforced at assignment / append time.
 */
struct PrometheusList {
    std::string element_type;                // "int", "double", "str", "bool"
    std::vector<PrometheusValue> elements;

    PrometheusList() = default;
    PrometheusList(std::string elem_type, std::vector<PrometheusValue> elems)
        : element_type(std::move(elem_type)), elements(std::move(elems)) {}
};

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
    BOOL,
    VOID,
    LIST,

    // Identifiers and Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // Keywords
    IF,
    ELIF,
    ELSE,

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

    // Modules
    IMPORT,         // import (import filename)
    USE,            // use (use stlib_module)

    // Delimiters
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]

    DOT,            // .
    COLON,          // :

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
        case TokenType::BOOL:       return "BOOL";
        case TokenType::VOID:       return "VOID";
        case TokenType::LIST:       return "LIST";

        // Identifiers and Literals
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::STRING:     return "STRING";

        // Keywords
        case TokenType::IF:         return "IF";
        case TokenType::ELIF:       return "ELIF";
        case TokenType::ELSE:       return "ELSE";

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
        case TokenType::DECREMENT:   return "DECREMENT";

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

        // Modules
        case TokenType::IMPORT:     return "IMPORT";
        case TokenType::USE:        return "USE";

        // Delimiters
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::LBRACE:     return "LBRACE";
        case TokenType::RBRACE:     return "RBRACE";
        case TokenType::LBRACKET:   return "LBRACKET";
        case TokenType::RBRACKET:   return "RBRACKET";

        case TokenType::DOT:        return "DOT";
        case TokenType::COLON:      return "IN";

        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::COMMA:      return "COMMA";
        case TokenType::EOF_TOKEN:  return "EOF";

        default:                    return "UNKNOWN";
    }
}

/**
 * @brief A small container representing a single meaningful unit of code (lexemes).
 *
 * Carries the 1-based source line on which the token appeared so that
 * both the parser and interpreter can include it in error messages.
 */
class Token {

private:
    TokenType   token;
    std::string value;
    int         line_;   // 1-based line number; 0 = unknown

public:
    Token(TokenType token, std::string value, int line = 0)
        : token(token), value(std::move(value)), line_(line) {}

    TokenType   get_token() const { return token; }
    std::string get_value() const { return value; }
    int         get_line()  const { return line_; }

    void print() const {
        std::cout << "Token(" << to_string(token) << ", '" << value << "'"
                  << ", line=" << line_ << ")" << std::endl;
    }
};

#endif // PROMETHEUS_TYPES_H