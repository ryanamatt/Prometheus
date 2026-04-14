from ast_nodes import ASTNode, NumberNode, StringNode, VarNode, BinOpNode, VarDeclNode, PrintNode, IfNode
from prometheus_types import TokenType
from typing import Any

class Interpreter:
    """
    
    """
    def __init__(self) -> None:
        """
        
        """
        self.variables: dict[str, Any] = {}

    def visit(self, node: ASTNode) -> Any:
        """
        
        """
        if isinstance(node, NumberNode):
            return float(node.value) if '.' in node.value else int(node.value)
        
        elif isinstance(node, VarNode):
            if node.value in self.variables:
                return self.variables[node.value]
            raise Exception(f"Runtime Error: Variable '{node.value}' is not defined.")
        
        elif isinstance(node, StringNode):
            return node.value
        
        elif isinstance(node, BinOpNode):
            left_val = self.visit(node.left)
            right_val = self.visit(node.right)

            if node.op.token_type == TokenType.PLUS:
                return left_val + right_val
            
            elif node.op.token_type == TokenType.GREATER:
                return left_val > right_val
            elif node.op.token_type == TokenType.GREATEREQ:
                return left_val >= right_val
            elif node.op.token_type == TokenType.LESSER:
                return left_val < right_val
            elif node.op.token_type == TokenType.LESSEREQ:
                return left_val <= right_val
            
        elif isinstance(node, VarDeclNode):
            value = self.visit(node.value_node)
            self.variables[node.name] = value
            return value
        
        elif isinstance(node, PrintNode):
            results = [str(self.visit(expr)) for expr in node.expressions]
            output = " ".join(results)
            print(output)
            return output
        
        elif isinstance(node, IfNode):
            condition_met = self.visit(node.condition)
            
            if condition_met:
                for stmt in node.then_branch:
                    self.visit(stmt)
            elif node.else_branch:
                for stmt in node.else_branch:
                    self.visit(stmt)
        

    def interpret(self, nodes: list[ASTNode]):
        for node in nodes:
            result = self.visit(node)
        return result