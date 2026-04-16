
#include <unordered_map>
#include <cctype>
#include <iostream>
#include "lexer.h"

std::vector<Token> Lexer::tokenize() {
    std::unordered_map<char, TokenType> symbol_map = {
            {'=', TokenType::ASSIGN},
            {'!', TokenType::NOT},
            {'+', TokenType::PLUS},
            {'-', TokenType::MINUS},
            {'*', TokenType::MULTIPLY},
            {'/', TokenType::DIVIDE},
            {'%', TokenType::MODULO},
            {'>', TokenType::GREATER},
            {'<', TokenType::LESSER},
            {'&', TokenType::AND},
            {'|', TokenType::OR},
            {'(', TokenType::LPAREN},
            {')', TokenType::RPAREN},
            {'{', TokenType::LBRACE},
            {'}', TokenType::RBRACE},
            {';', TokenType::SEMICOLON},
            {',', TokenType::COMMA}
        };

        while (current_pos < (int)source.size()) {
            char ch = source[current_pos];

            if (std::isspace(ch)) { 
                if (ch == '\n')
                    current_line++;
                current_pos++; 
            }

            else if (std::isalpha(ch)) {
                tokens.push_back(this->make_identifier());
            }

            else if (std::isdigit(ch)) {
                tokens.push_back(this->make_number());
            }

            else if (ch == '"') {
                tokens.push_back(this->make_string());
            }

            else if (symbol_map.find(ch) != symbol_map.end()) {

                char next_char = (current_pos + 1 < (int)source.size())
                             ? source[current_pos + 1]
                             : '\0';
                
                bool matched = false;
                switch (ch) {

                    case '*':
                        if (next_char == '*') {
                            tokens.push_back(Token(TokenType::EXPONENT, "**"));
                            current_pos += 2;
                            matched = true;
                        }
                        break;

                    case '>':
                        if (next_char == '=') {
                            tokens.push_back(Token(TokenType::GREATEREQ, ">="));
                            current_pos += 2;
                            matched = true;
                        }
                        break;

                    case '<':
                        if (next_char == '=') {
                            tokens.push_back(Token(TokenType::LESSEREQ, "<="));
                            current_pos += 2;
                            matched = true;
                        }
                        break;

                    case '=':
                        if (next_char == '=') {
                            tokens.push_back(Token(TokenType::EQUAL, "=="));
                            current_pos += 2;
                            matched = true;
                        }
                        break;
                    
                    case '!':
                        if (next_char == '=') {
                            tokens.push_back(Token(TokenType::NOTEQUAL, "!="));
                            current_pos += 2;
                            matched = true;
                        }
                        break;

                    case '&':
                        if (next_char == '&') {
                            tokens.push_back(Token(TokenType::AND, "&&"));
                            current_pos += 2;
                            matched = true;
                        }
                        break;

                    case '|':
                        if (next_char == '|') {
                            tokens.push_back(Token(TokenType::OR, "||"));
                            current_pos += 2;
                            matched = true;
                        }
                        break;
                }
                
                if (!matched) {
                    tokens.push_back(Token(symbol_map[ch], std::string(1, ch)));
                    current_pos++;
                }
            }

            else {
                // The character is not whitespace, alphanumeric, a quote, or a
                // known symbol — throw instead of calling exit().
                throw UnknownCharException(ch, current_line);
            }

        }

    tokens.push_back(Token(TokenType::EOF_TOKEN, "EOF"));
    return tokens;
}

Token Lexer::make_identifier() {
    std::string word;
    // Keep reading as long as it's alphanumeric or an underscore
    while (current_pos < (int)source.size() && (std::isalnum(source[current_pos]) || source[current_pos] == '_')) {
        word += source[current_pos];
        current_pos++;
    }

    // Keyword/Type mapping to match the Python logic
    static const std::unordered_map<std::string, TokenType> keyword_map = {
        {"int",    TokenType::INT},
        {"str",    TokenType::STR},
        {"double", TokenType::DOUBLE},
        {"if",     TokenType::IF},
        {"elif",   TokenType::ELIF},
        {"else",   TokenType::ELSE},
        {"print",  TokenType::PRINT},
        {"while",  TokenType::WHILE},
        {"for",    TokenType::FOR},
        {"func",   TokenType::FUNC},
        {"return", TokenType::RETURN}
    };

    // Check if the word exists in our keyword map
    auto it = keyword_map.find(word);
    if (it != keyword_map.end()) {
        return Token(it->second, word);
    }

    // If not found in map, it's a standard identifier
    return Token(TokenType::IDENTIFIER, word);
}

Token Lexer::make_number() {
    std::string num_str;
    int dot_count = 0;
    int start_line = current_line;
 
    while (current_pos < (int)source.size() &&
           (std::isdigit(source[current_pos]) || source[current_pos] == '.')) {
 
        if (source[current_pos] == '.') {
            dot_count++;
            // More than one decimal point is always invalid.
            if (dot_count > 1) {
                // Consume the rest of the malformed literal before throwing so
                // the caller's position is left just past the bad token.
                while (current_pos < (int)source.size() &&
                       (std::isdigit(source[current_pos]) || source[current_pos] == '.')) {
                    num_str += source[current_pos];
                    current_pos++;
                }
                throw InvalidNumberException(num_str, start_line);
            }
        }
 
        num_str += source[current_pos];
        current_pos++;
    }
 
    // A trailing decimal point with no fractional digits (e.g. "42.") is invalid.
    if (!num_str.empty() && num_str.back() == '.') {
        throw InvalidNumberException(num_str, start_line);
    }
 
    return Token(TokenType::NUMBER, num_str);
}

Token Lexer::make_string() {
    int start_line = current_line;
    current_pos++; // consume opening '"'
 
    std::string string_val;
    while (current_pos < (int)source.size() && source[current_pos] != '"') {
        // Track newlines inside strings so current_line stays accurate.
        if (source[current_pos] == '\n') {
            current_line++;
        }
        string_val += source[current_pos];
        current_pos++;
    }
 
    // If we walked off the end of the source without finding a closing '"',
    // the string literal was never terminated.
    if (current_pos >= (int)source.size()) {
        throw UnterminatedStringException(start_line);
    }
 
    current_pos++; // consume closing '"'
    return Token(TokenType::STRING, string_val);
}