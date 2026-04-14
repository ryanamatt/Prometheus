"""
The execution engine for the Prometheus language. 
It traverses the AST and executes the logic defined in the nodes.
"""
from typing import Any
from ast_nodes import ASTNode, NumberNode, StringNode, VarNode, BinOpNode, VarDeclNode, PrintNode, IfNode, LoopNode
from prometheus_types import TokenType

class Interpreter:
    """
    Handles the evaluation of AST nodes and maintains the runtime state (variables).
    """
    def __init__(self) -> None:
        """Initializes a new Interpreter with an empty variable memory."""
        self.variables: dict[str, Any] = {}

    def visit(self, node: ASTNode) -> Any:
        """
        Recursively visits an AST node and evaluates its value or executes its logic.
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
            elif node.op.token_type == TokenType.MINUS:
                return left_val - right_val
            elif node.op.token_type == TokenType.MULTIPLY:
                return left_val * right_val
            elif node.op.token_type == TokenType.DIVIDE:
                return left_val / right_val
            elif node.op.token_type == TokenType.MODULO:
                return left_val % right_val
            elif node.op.token_type == TokenType.EXPONENT:
                return left_val ** right_val
            
            elif node.op.token_type == TokenType.EQUAL:
                return left_val == right_val
            elif node.op.token_type == TokenType.NOTEQUAL:
                return left_val != right_val
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
            # 1. Check main IF
            if condition_met:
                for stmt in node.then_branch:
                    self.visit(stmt)
                return

            # 2. Check ELIFs
            for condition, branch in node.elif_branches:
                if self.visit(condition):
                    for stmt in branch:
                        self.visit(stmt)
                    return # Exit once a branch is met

            # 3. Check ELSE
            if node.else_branch:
                for stmt in node.else_branch:
                    self.visit(stmt)

        elif isinstance(node, LoopNode):
            while self.visit(node.condition):
                for stmt in node.do_branch:
                    self.visit(stmt)
            return

    def interpret(self, nodes: list[ASTNode]) -> Any:
        """
        Iterates through a list of top-level AST nodes and executes them in order.
        """
        result: Any = None
        for node in nodes:
            result: Any = self.visit(node)

        return result