from enum import Enum, auto

class TokenType(Enum):
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
    LESSEREQ = auto()     # <=

    LPAREN = auto()     # (
    RPAREN = auto()     # )
    LBRACE = auto()     # {
    RBRACE = auto()     # }

    SEMICOLON = auto()  # ;
    COMMA = auto()      # ,

    def __repr__(self) -> str:
        return f"{self.value}"