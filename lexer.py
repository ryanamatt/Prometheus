
from prometheus_types import TokenType

class Token:
    """
    
    """
    def __init__(self, token_type: TokenType, value: str) -> None:
        """
        
        """
        self.token_type = token_type
        self.value = value

    def __repr__(self):
        """
        
        """
        return f"Token({self.token_type}, '{self.value}')"

class Lexer:
    """
    
    """
    def __init__(self, filename: str):
        """
        
        """
        self.filename: str = filename
        self.source: str = ""
        self.tokens: list[Token] = []
        self.current_pos: int = 0

        self._read_file()

    def _read_file(self) -> None:
        """
        
        """
        with open(self.filename, 'r') as f:
            self.source = f.read()

    def tokenize(self) -> list[Token]:
        """
        
        """
        symbol_map = {
            '=': TokenType.ASSIGN,
            '+': TokenType.PLUS,
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
                match char:
                    case '>':
                        if self.current_pos + 1 < len(self.source) and self.source[self.current_pos + 1] == '=':
                            self.tokens.append(Token(TokenType.GREATEREQ, ">="))
                            self.current_pos += 2
                        else:
                            self.tokens.append(Token(TokenType.GREATER, ">"))
                            self.current_pos += 1

                    case '<':
                        if self.current_pos + 1 < len(self.source) and self.source[self.current_pos + 1] == '=':
                            self.tokens.append(Token(TokenType.LESSEREQ, "<="))
                            self.current_pos += 2
                        else:
                            self.tokens.append(Token(TokenType.LESSER, "<"))
                            self.current_pos += 1

                    case _:
                        self.tokens.append(Token(symbol_map[char], char))
                        self.current_pos += 1

            else:
                # Handle unknown characters
                # print("Unknown Character:", char)
                self.current_pos += 1

        return self.tokens

    def _make_identifier(self) -> Token:
        """
        
        """
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
        """
        
        """
        num_str = ""
        while self.current_pos < len(self.source) and (self.source[self.current_pos].isdigit() or self.source[self.current_pos] == '.'):
            num_str += self.source[self.current_pos]
            self.current_pos += 1
            
        return Token(TokenType.NUMBER, num_str)
    
    def _make_string(self) -> Token:
        """
        
        """
        string_val = ""
        self.current_pos += 1  # Skip opening "
        while self.current_pos < len(self.source) and self.source[self.current_pos] != '"':
            string_val += self.source[self.current_pos]
            self.current_pos += 1
        self.current_pos += 1  # Skip closing "
        return Token(TokenType.STRING, string_val)