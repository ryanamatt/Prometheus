"""
The execution engine for the Prometheus language. 
It traverses the AST and executes the logic defined in the nodes.
"""
from typing import Any
from ast_nodes import (
    ASTNode, NumberNode, StringNode, VarNode, BinOpNode, VarDeclNode, PrintNode, IfNode, WhileNode, 
    ForNode, FunctionDeclNode, ReturnNode, CallNode
)
from prometheus_types import TokenType

class ReturnValue(Exception):
    def __init__(self, value: Any):
        self.value = value

class Interpreter:
    """
    Handles the evaluation of AST nodes and maintains the runtime state (variables).
    """
    def __init__(self, global_vars: dict[str, Any] | None = None) -> None:
        """Initializes a new Interpreter with an empty variable memory."""
        self.variables: dict[str, Any] = global_vars if global_vars is not None else {}
        self.functions: dict[str, FunctionDeclNode] = {}

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
            elif node.op.token_type == TokenType.AND:
                return left_val and right_val
            elif node.op.token_type == TokenType.OR:
                return left_val or right_val
            
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

        elif isinstance(node, WhileNode):
            while self.visit(node.condition):
                for stmt in node.do_branch:
                    self.visit(stmt)
            return
        
        elif isinstance(node, ForNode):
            count = 0
            self.visit(node.variable)
            while self.visit(node.condtion):
                for stmt in node.do_branch:
                    self.visit(stmt)
                self.visit(node.change_var)
                count += 1
                if count == 1000:
                    raise RecursionError("Recursion for 1000, Likely inf Recursion Error.")
            return
        
        if isinstance(node, FunctionDeclNode):
            self.functions[node.name] = node
            return None

        elif isinstance(node, CallNode):
            func_node = self.functions.get(node.name)
            if not func_node:
                raise Exception(f"Runtime Error: Function '{node.name}' not defined.")
            
            # 1. Evaluate arguments in the CURRENT scope
            arg_values = [self.visit(arg) for arg in node.args]
            
            # 2. Create a NEW interpreter for the LOCAL scope
            # We pass existing functions so the local scope can call global functions
            local_interpreter = Interpreter(global_vars={})
            local_interpreter.functions = self.functions 
            
            # 3. Map parameters to argument values
            for (_, p_name), val in zip(func_node.params, arg_values):
                local_interpreter.variables[p_name] = val
            
            # 4. Execute the body and catch the return value
            try:
                for stmt in func_node.body:
                    local_interpreter.visit(stmt)
            except ReturnValue as e:
                return e.value
            return None

        elif isinstance(node, ReturnNode):
            value = self.visit(node.value_node)
            raise ReturnValue(value)

    def interpret(self, nodes: list[ASTNode]) -> Any:
        """
        Iterates through a list of top-level AST nodes and executes them in order.
        """
        result: Any = None
        for node in nodes:
            result: Any = self.visit(node)

        return result