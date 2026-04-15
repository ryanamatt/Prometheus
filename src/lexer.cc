
#include <unordered_map>
#include "lexer.h"

std::vector<Token> Lexer::tokenize() {
    std::unordered_map<char, TokenType> symbol_map = {
            {'=', TokenType.ASSIGN},
            '!': TokenType.NOT,
            '+': TokenType.PLUS,
            '-': TokenType.MINUS,
            '*': TokenType.MULTIPLY,
            '/': TokenType.DIVIDE,
            '%': TokenType.MODULO,
            '>': TokenType.GREATER,
            '<': TokenType.LESSER,
            '&': TokenType.AND,
            '|': TokenType.OR,
            '(': TokenType.LPAREN,
            ')': TokenType.RPAREN,
            '{': TokenType.LBRACE,
            '}': TokenType.RBRACE,
            ';': TokenType.SEMICOLON,
            ',': TokenType.COMMA
        }
    }
}