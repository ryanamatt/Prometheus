

#include <string>

/**
 * @brief Enumeration of all valid token types supported by the 
    Prometheus Lexer and Parser.
 */
enum class TokenType
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

    SEMICOLON,       // ;
    EOF_TOKEN,            // EOF
};

/**
 * @brief Helper function to convert enum to string for debugging
 */
inline std::string to_string(TokenType t) {
    switch (t) {
        case TokenType::INT:        return "INT";
        case TokenType::STR:        return "STR";
        case TokenType::DOUBLE:     return "DOUBLE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::STRING:     return "STRING";
        case TokenType::ASSIGN:     return "ASSIGN";
        case TokenType::SEMICOLON:  return "SEMICOLON";
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