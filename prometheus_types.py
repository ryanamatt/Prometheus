"""
Defines the core data types and token definitions used across the 
Prometheus interpreter ecosystem.
"""

from enum import Enum, auto

class TokenType(Enum):
    """
    Enumeration of all valid token types supported by the 
    Prometheus Lexer and Parser.
    """
    # Data Types
    INT = auto()
    STR = auto()
    DOUBLE = auto()
    
    # Identifiers & Literals
    IDENTIFIER = auto()
    NUMBER = auto()
    STRING = auto()
    
    # Keywords
    IF = auto()
    ELIF = auto()
    ELSE = auto()
    PRINT = auto()
    
    # Symbols
    ASSIGN = auto()     # =
    NOT = auto()        # !

    # Math Operators
    PLUS = auto()       # +
    MINUS = auto()      # -
    MULTIPLY = auto()   # *
    DIVIDE = auto()     # /
    MODULO = auto()     # %
    EXPONENT = auto()   # ** 

    # Comparison Operators
    EQUAL = auto()      # ==
    NOTEQUAL = auto()   # !=
    GREATER = auto()    # >
    LESSER = auto()     # <
    GREATEREQ = auto()  # >=
    LESSEREQ = auto()   # <=

    # Loops
    WHILE = auto()

    LPAREN = auto()     # (
    RPAREN = auto()     # )
    LBRACE = auto()     # {
    RBRACE = auto()     # }

    SEMICOLON = auto()  # ;
    COMMA = auto()      # ,
    EOF = auto()        # EOF

    def __repr__(self) -> str:
        """Returns the string representation of the Enum value."""
        return f"{self.value}"
    
class Token:
    """
    A small container representing a single meaningful unit of code (lexemes).
    """
    def __init__(self, token_type: TokenType, value: str) -> None:
        """Initializes a Token with a type and its literal string value."""
        self.token_type = token_type
        self.value = value

    def __repr__(self):
        return f"Token({self.token_type}, '{self.value}')"