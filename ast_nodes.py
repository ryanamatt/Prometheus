from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from lexer import Token

class ASTNode:
    """
        
    """
    pass

class NumberNode(ASTNode):
    """
    
    """
    def __init__(self, token: Token):
        """
    
        """
        self.token: Token = token
        self.value: str = token.value

    def __repr__(self):
        return f"{self.value}"
    
class StringNode(ASTNode):
    """
    
    """
    def __init__(self, token: Token):
        """
        
        """
        self.token = token
        self.value = token.value

    def __repr__(self):
        return f'"{self.value}"'

class VarNode(ASTNode):
    """
    
    """
    def __init__(self, token: Token):
        """
        
        """
        self.token: Token = token
        self.value: str = token.value

    def __repr__(self):
        return f"{self.value}"

class BinOpNode(ASTNode):
    """
    
    """
    def __init__(self, left: ASTNode, op: Token, right: ASTNode):
        """
        
        """
        self.left: ASTNode = left
        self.op: Token = op
        self.right: ASTNode = right

    def __repr__(self):
        return f"({self.left} {self.op.value} {self.right})"

class VarDeclNode(ASTNode):
    """
    
    """
    def __init__(self, var_type: str, name: str, value_node: ASTNode):
        """
        
        """
        self.var_type: str = var_type
        self.name: str = name
        self.value_node: ASTNode = value_node

    def __repr__(self):
        return f"VarDecl({self.var_type} {self.name} = {self.value_node})"
    
class PrintNode(ASTNode):
    """
    
    """
    def __init__(self, expressions: list[ASTNode]):
        """
        
        """
        self.expressions = expressions

    def __repr__(self):
        return f"Print({self.expressions})"
    
class IfNode(ASTNode):
    def __init__(self, condition: ASTNode, then_branch: list[ASTNode], else_branch: list[ASTNode] | None = None):
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch

    def __repr__(self):
        return f"If({self.condition}) {{ {self.then_branch} }} Else {{ {self.else_branch} }}"