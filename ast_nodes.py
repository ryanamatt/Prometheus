"""
Defines the Abstract Syntax Tree (AST) node classes for the Prometheus language.
Each class represents a specific grammatical construct used by the parser to 
build a structured representation of the source code.
"""

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from prometheus_types import Token

class ASTNode:
    """Base class for all nodes in the Abstract Syntax Tree."""
    pass

class NumberNode(ASTNode):
    """Represents a numeric literal (integer or float) in the AST."""
    def __init__(self, token: Token):
        """Initializes a NumberNode with its corresponding token."""
        self.token: Token = token
        self.value: str = token.value

    def __repr__(self):
        return f"{self.value}"
    
class StringNode(ASTNode):
    """Represents a string literal in the AST."""
    def __init__(self, token: Token):
        """Initializes a StringNode with its corresponding token."""
        self.token = token
        self.value = token.value

    def __repr__(self):
        return f'"{self.value}"'

class VarNode(ASTNode):
    """Represents a variable identifier usage in an expression."""
    def __init__(self, token: Token):
        """Initializes a VarNode with the variable's identifier token."""
        self.token: Token = token
        self.value: str = token.value

    def __repr__(self):
        return f"{self.value}"

class BinOpNode(ASTNode):
    """Represents a binary operation (e.g., +, -, *, ==) between two nodes."""
    def __init__(self, left: ASTNode, op: Token, right: ASTNode):
        """Initializes a BinOpNode with left/right operands and an operator."""
        self.left: ASTNode = left
        self.op: Token = op
        self.right: ASTNode = right

    def __repr__(self):
        return f"({self.left} {self.op.value} {self.right})"

class VarDeclNode(ASTNode):
    """Represents a variable declaration statement (e.g., int x = 5;)."""
    def __init__(self, var_type: str, name: str, value_node: ASTNode):
        """Initializes a VarDeclNode with type, identifier name, and initial value."""
        self.var_type: str = var_type
        self.name: str = name
        self.value_node: ASTNode = value_node

    def __repr__(self):
        return f"VarDecl({self.var_type} {self.name} = {self.value_node})"
    
class PrintNode(ASTNode):
    """Represents a print statement containing one or more expressions."""
    def __init__(self, expressions: list[ASTNode]):
        """Initializes a PrintNode with a list of expressions to be printed."""
        self.expressions = expressions

    def __repr__(self):
        return f"Print({self.expressions})"
    
class IfNode(ASTNode):
    """Represents an 'if-else' control flow structure."""
    def __init__(self, condition: ASTNode, then_branch: list[ASTNode], else_branch: list[ASTNode]):
        """Initializes an IfNode with a condition and code blocks for true/false results."""
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch

    def __repr__(self):
        return f"If({self.condition}) {{ {self.then_branch} }} Else {{ {self.else_branch} }}"
    
class EOFNode(ASTNode):
    """Sentinel node representing the end of a statement stream or file."""
    def __repr__(self):
        return "EOF"