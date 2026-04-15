
#include <unordered_map>
#include <cctype>
#include <iostream>
#include "lexer.h"

std::vector<Token> Lexer::tokenize() {
    std::unordered_map<char, TokenType> symbol_map = {
            {'=', TokenType::ASSIGN},
            {';', TokenType::SEMICOLON}
        };

        while (current_pos < (int)source.size()) {
            char ch = source[current_pos];

            if (std::isspace(ch)) { current_pos++; }

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
                tokens.push_back(Token(symbol_map[ch], std::string(1, ch)));
                current_pos++;
            }

            else {
                std::cout << "Exit Failure from Unknown Char: " << ch << std::endl;
                exit(EXIT_FAILURE);
            }

        }

    tokens.push_back(Token(TokenType::EOF_TOKEN, "EOF"));
    return tokens;
}

Token Lexer::make_identifier() {
    std::string word;
    while (current_pos < (int)source.size() && (std::isalnum(source[current_pos]) || source[current_pos] == '_')) {
        word += source[current_pos];
        current_pos++;
    }

    // Keyword Check
    if (word == "int")    return Token(TokenType::INT, word);
    if (word == "str")    return Token(TokenType::STR, word);
    if (word == "double") return Token(TokenType::DOUBLE, word);

    return Token(TokenType::IDENTIFIER, word);
}

Token Lexer::make_number() {
    std::string num_str;
    while (current_pos < (int)source.size() && (std::isdigit(source[current_pos]) || source[current_pos] == '.')) {
        num_str += source[current_pos];
        current_pos++;
    }
    return Token(TokenType::NUMBER, num_str);
}

Token Lexer::make_string() {
    std::string string_val;
    current_pos++; // skip "
    while (current_pos < (int)source.size() && source[current_pos] != '"') {
        string_val += source[current_pos];
        current_pos++;
    }
    current_pos++; // skip "
    return Token(TokenType::STRING, string_val);
}