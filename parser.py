
from prometheus_types import TokenType
from ast_nodes import ASTNode, NumberNode, StringNode, VarNode, BinOpNode, VarDeclNode, PrintNode, IfNode

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from lexer import Token

class Parser:
    """
    
    """
    def __init__(self, tokens: list[Token]) -> None:
        """
        
        """
        self.tokens: list[Token] = tokens
        self.pos = 0

    def current_token(self) -> Token | None:
        """
        
        """
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def eat(self, expected_type: TokenType) -> Token:
        """
        
        """
        token: Token | None = self.current_token()

        if token and token.token_type == expected_type:
            self.pos += 1
            return token
        else:
            raise Exception(f"Syntax Error: Expected {expected_type}, got {token.token_type if token else 'EOF'}")
        

    def parse(self) -> list[ASTNode]:
        """
        
        """
        nodes: list[ASTNode] = []
        while self.pos < len(self.tokens):
            node: ASTNode | None = self.parse_statement()
            if node:
                nodes.append(node)

        return nodes

    def parse_statement(self) -> ASTNode | None:
        """
        
        """
        token: Token | None = self.current_token()
        if token and token.token_type in [TokenType.INT, TokenType.STR, TokenType.DOUBLE]:
            return self.parse_declaration()
        
        elif token and token.token_type == TokenType.PRINT:
            return self.parse_print()
        
        elif token and token.token_type == TokenType.IF:
            return self.parse_if()
        
        self.pos += 1
        return None

    def parse_declaration(self):
        """
        
        """
        current_token = self.current_token()
        if not current_token: return

        type_token: Token = self.eat(current_token.token_type)
        name_token: Token = self.eat(TokenType.IDENTIFIER)
        self.eat(TokenType.ASSIGN)

        value_node: ASTNode = self.parse_expression()

        self.eat(TokenType.SEMICOLON)
        # print(f"Parsed Declaration for: {name_token.value} from Type Token: {type_token}")
        return VarDeclNode(type_token.value, name_token.value, value_node)

    def parse_expression(self) -> ASTNode:
        """

        """
        # First, parse addition/subtraction
        node = self.parse_additive()
        
        # Check for all comparison operators
        while (token := self.current_token()):
            if token.token_type == TokenType.GREATER:
                op = self.eat(TokenType.GREATER)
                node = BinOpNode(left=node, op=op, right=self.parse_additive())
            elif token.token_type == TokenType.GREATEREQ:
                op = self.eat(TokenType.GREATEREQ)
                node = BinOpNode(left=node, op=op, right=self.parse_additive())
            elif token.token_type == TokenType.LESSER:
                op = self.eat(TokenType.LESSER)
                node = BinOpNode(left=node, op=op, right=self.parse_additive())
            elif token.token_type == TokenType.LESSEREQ:
                op = self.eat(TokenType.LESSEREQ)
                node = BinOpNode(left=node, op=op, right=self.parse_additive())
            else:
                break
        
        return node
    
    def parse_additive(self) -> ASTNode:
        """
        
        """
        node = self.parse_term()
        while (token := self.current_token()) and token.token_type == TokenType.PLUS:
            op = self.eat(TokenType.PLUS)
            node = BinOpNode(left=node, op=op, right=self.parse_term())
        return node

    def parse_term(self) -> ASTNode:
        """
        Handles single values or variables (highest priority).
        """
        token = self.current_token()
        if not token: return ASTNode()

        if token.token_type == TokenType.NUMBER:
            return NumberNode(self.eat(TokenType.NUMBER))
        
        elif token.token_type == TokenType.STRING:
            return StringNode(self.eat(TokenType.STRING))
        
        elif token.token_type == TokenType.IDENTIFIER:
            return VarNode(self.eat(TokenType.IDENTIFIER))
        
        raise Exception(f"Expected expression, got {token.token_type}")
    
    def parse_print(self) -> PrintNode:
        """
        
        """
        self.eat(TokenType.PRINT)
        self.eat(TokenType.LPAREN)

        expressions: list[ASTNode] = []
        expressions.append(self.parse_expression())

        while self.current_token() and self.current_token().token_type == TokenType.COMMA:
            self.eat(TokenType.COMMA)
            expressions.append(self.parse_expression())

        self.eat(TokenType.RPAREN)
        self.eat(TokenType.SEMICOLON)
        return PrintNode(expressions)
    
    def parse_if(self) -> IfNode:
        self.eat(TokenType.IF)
        self.eat(TokenType.LPAREN)
        condition = self.parse_expression()
        self.eat(TokenType.RPAREN)

        self.eat(TokenType.LBRACE)
        then_branch: list[ASTNode | None] = []
        while self.current_token() and self.current_token().token_type != TokenType.RBRACE:
            then_branch.append(self.parse_statement())
        self.eat(TokenType.RBRACE)

        else_branch: list[ASTNode | None] = []
        if self.current_token() and self.current_token().token_type == TokenType.ELSE:
            self.eat(TokenType.ELSE)
            self.eat(TokenType.LBRACE)
            while self.current_token() and self.current_token().token_type != TokenType.RBRACE:
                else_branch.append(self.parse_statement())
            self.eat(TokenType.RBRACE)

        return IfNode(condition, then_branch, else_branch)