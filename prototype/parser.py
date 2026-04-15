"""
The Syntax Analyzer for Prometheus.
Consumes tokens from the Lexer and builds an Abstract Syntax Tree (AST) 
based on the language grammar.
"""

from prometheus_types import TokenType
from ast_nodes import (
    NumberNode, StringNode, VarNode, BinOpNode, VarDeclNode, PrintNode, IfNode, WhileNode, ForNode, 
    FunctionDeclNode, ReturnNode, CallNode, EOFNode
)

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from ast_nodes import ASTNode
    from prometheus_types import Token

class Parser:
    """
    Implements a recursive descent parser to convert tokens into an AST.
    """
    def __init__(self, tokens: list[Token]) -> None:
        """Initializes the Parser with a list of tokens and a pointer."""
        self.tokens: list[Token] = tokens
        self.pos = 0

    def current_token(self) -> Token:
        """Returns the token currently pointed to by the parser."""
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        
        return Token(TokenType.EOF, "EOF")
    
    def peek(self) -> Token:
        """Looks at the next token without consuming it."""
        if self.pos + 1 < len(self.tokens):
            return self.tokens[self.pos + 1]
        return Token(TokenType.EOF, "EOF")

    def eat(self, expected_type: TokenType) -> Token:
        """
        Validates the current token type and moves the pointer forward.
        Raises a Syntax Error if the type does not match.
        """
        token: Token = self.current_token()

        if token.token_type == expected_type:
            self.pos += 1
            return token
        else:
            raise Exception(f"Syntax Error: Expected {expected_type}, got {token.token_type if token else 'EOF'}")

    def parse(self) -> list[ASTNode]:
        """Entry point for parsing the entire token stream into a list of statements."""
        nodes: list[ASTNode] = []
        while self.pos < len(self.tokens):
            node: ASTNode | None = self.parse_statement()
            if node:
                nodes.append(node)

        return nodes

    def parse_statement(self) -> ASTNode:
        """Determines the type of statement and routes to the specific parsing method."""

        token: Token = self.current_token()
        if token.token_type in [TokenType.INT, TokenType.STR, TokenType.DOUBLE]:
            return self.parse_declaration()
        
        elif token.token_type == TokenType.IDENTIFIER:
            if self.peek().token_type == TokenType.LPAREN:
                return self.parse_call()
            return self.parse_identifier()
        
        elif token.token_type == TokenType.PRINT:
            return self.parse_print()
        
        elif token.token_type == TokenType.IF:
            return self.parse_if()
        
        elif token.token_type == TokenType.WHILE:
            return self.parse_while()
        
        elif token.token_type == TokenType.FOR:
            return self.parse_for()
        
        elif token.token_type == TokenType.FUNC:
            return self.parse_function()
        
        elif token.token_type == TokenType.RETURN:
            return self.parse_return()
        
        elif token.token_type == TokenType.EOF:
            self.pos += 1
            return EOFNode()
        
        raise Exception(f"Unexpected token: {token.token_type}")

    def parse_declaration(self) -> ASTNode:
        """Parses variable declarations (e.g., 'int x = 10;')."""
        current_token: Token = self.current_token()

        type_token: Token = self.eat(current_token.token_type)
        name_token: Token = self.eat(TokenType.IDENTIFIER)
        self.eat(TokenType.ASSIGN)

        value_node: ASTNode = self.parse_expression()

        self.eat(TokenType.SEMICOLON)
        # print(f"Parsed Declaration for: {name_token.value} from Type Token: {type_token}")
        return VarDeclNode(type_token.value, name_token.value, value_node)
    
    def parse_identifier(self) -> ASTNode:
        """Parses an Identifier of x = x + 1"""
        current_token: Token = self.eat(TokenType.IDENTIFIER)
        self.eat(TokenType.ASSIGN)

        value_node: ASTNode = self.parse_expression()
        self.eat(TokenType.SEMICOLON)
        return VarDeclNode(current_token.value, current_token.value, value_node)

    def parse_expression(self) -> ASTNode:
        """Parses the lowest level of expression precedence (comparisons)."""
        # First, parse addition/subtraction
        node = self.parse_math_operations()
        
        # Check for all comparison operators
        while token := self.current_token():

            if token.token_type == TokenType.EQUAL:
                op = self.eat(TokenType.EQUAL)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.NOTEQUAL:
                op = self.eat(TokenType.NOTEQUAL)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            if token.token_type == TokenType.GREATER:
                op = self.eat(TokenType.GREATER)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.GREATEREQ:
                op = self.eat(TokenType.GREATEREQ)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.LESSER:
                op = self.eat(TokenType.LESSER)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.LESSEREQ:
                op = self.eat(TokenType.LESSEREQ)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.AND:
                op = self.eat(TokenType.AND)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            elif token.token_type == TokenType.OR:
                op = self.eat(TokenType.OR)
                node = BinOpNode(left=node, op=op, right=self.parse_math_operations())

            else:
                break
        
        return node
    
    def parse_math_operations(self) -> ASTNode:
        """Parses additive operations (+, -)."""
        node = self.parse_factor()
        while token := self.current_token():
            if token.token_type in [TokenType.PLUS, TokenType.MINUS]:
                op = self.eat(token.token_type)
                node = BinOpNode(left=node, op=op, right=self.parse_factor())

            else:
                break
        return node
    
    def parse_factor(self) -> ASTNode:
        """Parses multiplicative operations (*, /, %)."""
        node = self.parse_exponent()
        while token := self.current_token():

            if token.token_type in [TokenType.MULTIPLY, TokenType.DIVIDE, TokenType.MODULO]:
                op = self.eat(token.token_type)
                node = BinOpNode(left=node, op=op, right=self.parse_exponent())

            else:
                break
        return node
    
    def parse_exponent(self) -> ASTNode:
        """Parses exponentiation operations (**)."""
        node = self.parse_parentheses()
        while token := self.current_token():

            if token.token_type == TokenType.EXPONENT:
                op = self.eat(token.token_type)
                node = BinOpNode(left=node, op=op, right=self.parse_parentheses())

            else:
                break
        return node
    
    def parse_parentheses(self) -> ASTNode:
        """Handles grouped expressions inside parentheses."""
        token = self.current_token()

        if token and token.token_type == TokenType.LPAREN:
            self.eat(TokenType.LPAREN)
            # Restart the hierarchy from the bottom (expression)
            node = self.parse_expression() 
            self.eat(TokenType.RPAREN)
            return node

        return self.parse_term()
                

    def parse_term(self) -> ASTNode:
        """Parses the highest priority elements (numbers, strings, identifiers)."""
        token: Token = self.current_token()

        if token.token_type == TokenType.NUMBER:
            return NumberNode(self.eat(TokenType.NUMBER))
        
        elif token.token_type == TokenType.STRING:
            return StringNode(self.eat(TokenType.STRING))
        
        elif token.token_type == TokenType.IDENTIFIER:
            if self.peek().token_type == TokenType.LPAREN:
                return self.parse_call()
            return VarNode(self.eat(TokenType.IDENTIFIER))
                
        raise Exception(f"Expected expression, got {token.token_type}")
    
    def parse_print(self) -> PrintNode:
        """Parses 'print' statements and their arguments."""
        self.eat(TokenType.PRINT)
        self.eat(TokenType.LPAREN)

        expressions: list[ASTNode] = []
        expressions.append(self.parse_expression())

        while self.current_token().token_type == TokenType.COMMA:
            self.eat(TokenType.COMMA)
            expressions.append(self.parse_expression())

        self.eat(TokenType.RPAREN)
        self.eat(TokenType.SEMICOLON)
        return PrintNode(expressions)
    
    def parse_if(self) -> IfNode:
        """Parses 'if' statements, including optional 'else' blocks."""
        self.eat(TokenType.IF)
        self.eat(TokenType.LPAREN)
        condition = self.parse_expression()
        self.eat(TokenType.RPAREN)

        self.eat(TokenType.LBRACE)
        then_branch: list[ASTNode] = []
        while self.current_token().token_type != TokenType.RBRACE:
            then_branch.append(self.parse_statement())
        self.eat(TokenType.RBRACE)

        elif_branches: list[tuple[ASTNode, list[ASTNode]]] = []
        while self.current_token().token_type == TokenType.ELIF:
            self.eat(TokenType.ELIF)
            self.eat(TokenType.LPAREN)
            elif_cond: ASTNode = self.parse_expression()
            self.eat(TokenType.RPAREN)

            self.eat(TokenType.LBRACE)
            elif_stmts: list[ASTNode] = []
            while self.current_token().token_type != TokenType.RBRACE:
                elif_stmts.append(self.parse_statement())
            self.eat(TokenType.RBRACE)
            elif_branches.append((elif_cond, elif_stmts))

        else_branch: list[ASTNode] = []
        if self.current_token().token_type == TokenType.ELSE:
            self.eat(TokenType.ELSE)
            self.eat(TokenType.LBRACE)
            while self.current_token().token_type != TokenType.RBRACE:
                else_branch.append(self.parse_statement())
            self.eat(TokenType.RBRACE)

        return IfNode(condition, then_branch, elif_branches, else_branch)
    
    def parse_while(self) -> ASTNode:
        """Parses a While Loop"""
        self.eat(TokenType.WHILE)
        self.eat(TokenType.LPAREN)
        condition: ASTNode = self.parse_expression()
        self.eat(TokenType.RPAREN)

        do_branch: list[ASTNode] = []

        self.eat(TokenType.LBRACE)
        while self.current_token().token_type != TokenType.RBRACE:
            do_branch.append(self.parse_statement())
        self.eat(TokenType.RBRACE)
        
        return WhileNode(condition, do_branch)
    
    def parse_for(self) -> ASTNode:
        """Parses a For Loop."""
        self.eat(TokenType.FOR)
        self.eat(TokenType.LPAREN)
        var: ASTNode = self.parse_declaration()
        condition: ASTNode = self.parse_expression()
        self.eat(TokenType.SEMICOLON)
        change_var: ASTNode = self.parse_statement()
        self.eat(TokenType.RPAREN)
        self.eat(TokenType.LBRACE)
        do_branch: list[ASTNode] = []
        while self.current_token().token_type != TokenType.RBRACE:
            do_branch.append(self.parse_statement())
        self.eat(TokenType.RBRACE)
        return ForNode(var, condition, change_var, do_branch)
    
    def parse_function(self) -> ASTNode:
        self.eat(TokenType.FUNC)
        return_type = self.eat(TokenType.INT).value # Simplification: assuming 'int' for now
        name = self.eat(TokenType.IDENTIFIER).value
        
        self.eat(TokenType.LPAREN)
        params: list[tuple[str, str]] = []
        if self.current_token().token_type != TokenType.RPAREN:
            while True:
                p_type = self.current_token().value
                self.eat(self.current_token().token_type)
                p_name = self.eat(TokenType.IDENTIFIER).value
                params.append((p_type, p_name))
                if self.current_token().token_type == TokenType.COMMA:
                    self.eat(TokenType.COMMA)
                else:
                    break
        self.eat(TokenType.RPAREN)
        self.eat(TokenType.LBRACE)
        body: list[ASTNode] = []
        while self.current_token().token_type != TokenType.RBRACE:
            stmt = self.parse_statement()
            if stmt: body.append(stmt)
        self.eat(TokenType.RBRACE)
        return FunctionDeclNode(name, return_type, params, body)

    def parse_return(self) -> ASTNode:
        self.eat(TokenType.RETURN)
        value = self.parse_expression()
        self.eat(TokenType.SEMICOLON)
        print(value)
        return ReturnNode(value)

    def parse_call(self) -> ASTNode:
        name = self.eat(TokenType.IDENTIFIER).value
        self.eat(TokenType.LPAREN)
        args: list[ASTNode] = []
        if self.current_token().token_type != TokenType.RPAREN:
            while True:
                args.append(self.parse_expression())
                if self.current_token().token_type == TokenType.COMMA:
                    self.eat(TokenType.COMMA)
                else:
                    break
        self.eat(TokenType.RPAREN)
        return CallNode(name, args)