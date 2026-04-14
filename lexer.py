"""
Scanner/Lexer module for Prometheus.
Converts raw source code text into a sequence of structured Tokens.
"""

from prometheus_types import TokenType, Token

class Lexer:
    """
    Performs lexical analysis on a source file to produce a token stream.
    """
    def __init__(self, source: str):
        """Initializes the Lexer and reads the content of the specified file."""
        self.source: str = source
        self.tokens: list[Token] = []
        self.current_pos: int = 0

    def tokenize(self) -> list[Token]:
        """
        Processes the source string and returns a list of Token objects.
        """
        symbol_map = {
            '=': TokenType.ASSIGN,
            '!': TokenType.NOT,
            '+': TokenType.PLUS,
            '-': TokenType.MINUS,
            '*': TokenType.MULTIPLY,
            '/': TokenType.DIVIDE,
            '%': TokenType.MODULO,
            '>': TokenType.GREATER,
            '<': TokenType.LESSER,
            '(': TokenType.LPAREN,
            ')': TokenType.RPAREN,
            '{': TokenType.LBRACE,
            '}': TokenType.RBRACE,
            ';': TokenType.SEMICOLON,
            ',': TokenType.COMMA
        }

        while (self.current_pos < len(self.source)):
            char: str = self.source[self.current_pos]

            if char.isspace():
                self.current_pos += 1
            
            elif char.isalpha():
                self.tokens.append(self._make_identifier())

            elif char.isdigit():
                self.tokens.append(self._make_number())

            elif char == '"':
                self.tokens.append(self._make_string())

            elif char in symbol_map: # Add symbols here
                if self.current_pos + 1 < len(self.source) and char == '*' and self.source[self.current_pos + 1] == '*':
                    self.tokens.append(Token(TokenType.EXPONENT, '**'))
                    self.current_pos += 2

                elif self.current_pos + 1 < len(self.source) and char == '>' and self.source[self.current_pos + 1] == '=':
                    self.tokens.append(Token(TokenType.GREATEREQ, ">="))
                    self.current_pos += 2

                elif self.current_pos + 1 < len(self.source) and char == '<' and self.source[self.current_pos + 1] == '=':
                    self.tokens.append(Token(TokenType.LESSEREQ, "<="))
                    self.current_pos += 2

                elif self.current_pos + 1 < len(self.source) and char == '=' and self.source[self.current_pos + 1] == '=':
                    self.tokens.append(Token(TokenType.EQUAL, "=="))
                    self.current_pos += 2

                elif self.current_pos + 1 < len(self.source) and char == '!' and self.source[self.current_pos + 1] == '=':
                        self.tokens.append(Token(TokenType.NOTEQUAL, "!="))
                        self.current_pos += 2

                else:
                    self.tokens.append(Token(symbol_map[char], char))
                    self.current_pos += 1

            else:
                # Handle unknown characters
                # print("Unknown Character:", char)
                self.current_pos += 1

        self.tokens.append(Token(TokenType.EOF, "EOF"))
        return self.tokens

    def _make_identifier(self) -> Token:
        """Extracts and returns an alphanumeric identifier or keyword token."""
        word: str = ""
        # Keep Reading as as it's alphanumberic
        while self.current_pos < len(self.source) and (self.source[self.current_pos].isalnum() 
                                                       or self.source[self.current_pos] == '_'):
            word += self.source[self.current_pos]
            self.current_pos += 1

        # Check keywords/types first
        try:
            # Matches 'int', 'str', 'if', 'else', 'print' if they are in the TokenType enum
            target_type = TokenType[word.upper()]
            return Token(target_type, word)
        except KeyError:
            return Token(TokenType.IDENTIFIER, word)

    def _make_number(self) -> Token:
        """Extracts and returns a numeric (integer or float) token."""
        num_str = ""
        while self.current_pos < len(self.source) and (self.source[self.current_pos].isdigit() or self.source[self.current_pos] == '.'):
            num_str += self.source[self.current_pos]
            self.current_pos += 1
            
        return Token(TokenType.NUMBER, num_str)
    
    def _make_string(self) -> Token:
        """Extracts and returns a string literal token, handling double quotes."""
        string_val = ""
        self.current_pos += 1  # Skip opening "
        while self.current_pos < len(self.source) and self.source[self.current_pos] != '"':
            string_val += self.source[self.current_pos]
            self.current_pos += 1
        self.current_pos += 1  # Skip closing "
        return Token(TokenType.STRING, string_val)