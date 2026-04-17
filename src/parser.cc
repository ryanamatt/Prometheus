#include <sstream>
#include "parser.h"

// ============================================================================
// Helpers
// ============================================================================

Token Parser::current_token() {
    if (pos < (int)tokens.size()) {
        return tokens[pos];
    }
    return Token(TokenType::EOF_TOKEN, "EOF");
}

Token Parser::peek() {
    if (pos + 1 < (int)tokens.size()) {
        return tokens[pos + 1];
    }
    return Token(TokenType::EOF_TOKEN, "EOF");
}

Token Parser::eat(TokenType expected_type) {
    Token token = current_token();
    if (token.get_token() == expected_type) {
        pos++;
        return token;
    }

    std::ostringstream oss;
    oss << "Syntax Error: Expected " << to_string(expected_type)
        << ", got " << to_string(token.get_token())
        << " ('" << token.get_value() << "')";
    throw ParseException(oss.str());
}

// ============================================================================
// Top-level parse
// ============================================================================

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> nodes;

    while (pos < (int)tokens.size()) {
        auto node = parse_statement();
        if (node) {
            nodes.push_back(std::move(node));
        }
    }

    return nodes;
}

// ============================================================================
// Statements
// ============================================================================

std::unique_ptr<ASTNode> Parser::parse_statement() {
    Token token = current_token();
    TokenType tt = token.get_token();

    if (tt == TokenType::INT || tt == TokenType::STR || tt == TokenType::DOUBLE || tt == TokenType::BOOL) {
        return parse_declaration();
    }

    if (tt == TokenType::IDENTIFIER) {
        if (peek().get_token() == TokenType::INCREMENT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::INCREMENT);
            eat(TokenType::SEMICOLON);
            return std::make_unique<IncrementDecrementNode>(id.get_value(), 1.0);
        }

        else if (peek().get_token() == TokenType::DECREMENT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::DECREMENT);
            eat(TokenType::SEMICOLON);
            return std::make_unique<IncrementDecrementNode>(id.get_value(), -1.0);
        }

        else if (peek().get_token() == TokenType::LPAREN) {
            return parse_call();
        }
        return parse_identifier();
    }

    if (tt == TokenType::PRINT)  return parse_print();
    if (tt == TokenType::INPUT)  return parse_input();
    if (tt == TokenType::IF)     return parse_if();
    if (tt == TokenType::WHILE)  return parse_while();
    if (tt == TokenType::FOR)    return parse_for();
    if (tt == TokenType::FUNC)   return parse_func();
    if (tt == TokenType::RETURN) return parse_return();

    if (tt == TokenType::EOF_TOKEN) {
        pos++;
        return std::make_unique<EOFNode>();
    }

    std::ostringstream oss;
    oss << "Unexpected token: " << to_string(tt) << " ('" << token.get_value() << "')";
    throw ParseException(oss.str());
}

std::unique_ptr<ASTNode> Parser::parse_declaration() {
    Token type_token = eat(current_token().get_token());   // INT / STR / DOUBLE
    Token name_token = eat(TokenType::IDENTIFIER);
    eat(TokenType::ASSIGN);
    auto value_node = parse_expression();
    eat(TokenType::SEMICOLON);
    return std::make_unique<VarDeclNode>(type_token.get_value(), name_token.get_value(), std::move(value_node));
}

std::unique_ptr<ASTNode> Parser::parse_identifier() {
    Token id_token = eat(TokenType::IDENTIFIER);
    eat(TokenType::ASSIGN);
    auto value_node = parse_expression();
    eat(TokenType::SEMICOLON);
    return std::make_unique<VarDeclNode>(id_token.get_value(), id_token.get_value(), std::move(value_node));
}

std::unique_ptr<PrintNode> Parser::parse_print() {
    eat(TokenType::PRINT);
    eat(TokenType::LPAREN);

    std::vector<std::unique_ptr<ASTNode>> expressions;
    expressions.push_back(parse_expression());

    while (current_token().get_token() == TokenType::COMMA) {
        eat(TokenType::COMMA);
        expressions.push_back(parse_expression());
    }

    eat(TokenType::RPAREN);
    eat(TokenType::SEMICOLON);
    return std::make_unique<PrintNode>(std::move(expressions));
}

std::unique_ptr<InputNode> Parser::parse_input() {
    eat(TokenType::INPUT);
    eat(TokenType::LPAREN);

    std::string prompt = "";
    if (current_token().get_token() == TokenType::STRING) {
        prompt = eat(TokenType::STRING).get_value();
    }

    eat(TokenType::RPAREN);
    return std::make_unique<InputNode>(prompt);
}

std::unique_ptr<IfNode> Parser::parse_if() {
    eat(TokenType::IF);
    eat(TokenType::LPAREN);
    auto condition = parse_expression();
    eat(TokenType::RPAREN);

    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        then_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);

    // Zero or more elif branches
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::vector<std::unique_ptr<ASTNode>>>> elif_branches;
    while (current_token().get_token() == TokenType::ELIF) {
        eat(TokenType::ELIF);
        eat(TokenType::LPAREN);
        auto elif_cond = parse_expression();
        eat(TokenType::RPAREN);

        eat(TokenType::LBRACE);
        std::vector<std::unique_ptr<ASTNode>> elif_stmts;
        while (current_token().get_token() != TokenType::RBRACE) {
            elif_stmts.push_back(parse_statement());
        }
        eat(TokenType::RBRACE);

        elif_branches.emplace_back(std::move(elif_cond), std::move(elif_stmts));
    }

    // Optional else branch
    std::vector<std::unique_ptr<ASTNode>> else_branch;
    if (current_token().get_token() == TokenType::ELSE) {
        eat(TokenType::ELSE);
        eat(TokenType::LBRACE);
        while (current_token().get_token() != TokenType::RBRACE) {
            else_branch.push_back(parse_statement());
        }
        eat(TokenType::RBRACE);
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(then_branch),
                                    std::move(elif_branches), std::move(else_branch));
}

std::unique_ptr<WhileNode> Parser::parse_while() {
    eat(TokenType::WHILE);
    eat(TokenType::LPAREN);
    auto condition = parse_expression();
    eat(TokenType::RPAREN);

    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> do_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        do_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);

    return std::make_unique<WhileNode>(std::move(condition), std::move(do_branch));
}

std::unique_ptr<ForNode> Parser::parse_for() {
    eat(TokenType::FOR);
    eat(TokenType::LPAREN);
    auto var       = parse_declaration();      // init statement (consumes its own semicolon)
    auto condition = parse_expression();
    eat(TokenType::SEMICOLON);
    auto change_var = parse_statement();        // update statement
    eat(TokenType::RPAREN);

    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> do_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        do_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);

    return std::make_unique<ForNode>(std::move(var), std::move(condition),
                                     std::move(change_var), std::move(do_branch));
}

std::unique_ptr<FunctionDeclNode> Parser::parse_func() {
    eat(TokenType::FUNC);

    // Return type: any type keyword or identifier is accepted
    Token return_type_tok = eat(current_token().get_token());
    std::string return_type = return_type_tok.get_value();

    std::string name = eat(TokenType::IDENTIFIER).get_value();

    eat(TokenType::LPAREN);
    std::vector<std::pair<std::string, std::string>> params;
    if (current_token().get_token() != TokenType::RPAREN) {
        while (true) {
            std::string p_type = current_token().get_value();
            eat(current_token().get_token());               // consume type token
            std::string p_name = eat(TokenType::IDENTIFIER).get_value();
            params.emplace_back(p_type, p_name);

            if (current_token().get_token() == TokenType::COMMA) {
                eat(TokenType::COMMA);
            } else {
                break;
            }
        }
    }
    eat(TokenType::RPAREN);

    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> body;
    while (current_token().get_token() != TokenType::RBRACE) {
        auto stmt = parse_statement();
        if (stmt) body.push_back(std::move(stmt));
    }
    eat(TokenType::RBRACE);

    return std::make_unique<FunctionDeclNode>(name, return_type, std::move(params), std::move(body));
}

std::unique_ptr<ReturnNode> Parser::parse_return() {
    eat(TokenType::RETURN);
    auto value = parse_expression();
    eat(TokenType::SEMICOLON);
    return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<CallNode> Parser::parse_call() {
    std::string name = eat(TokenType::IDENTIFIER).get_value();
    eat(TokenType::LPAREN);

    std::vector<std::unique_ptr<ASTNode>> args;
    if (current_token().get_token() != TokenType::RPAREN) {
        while (true) {
            args.push_back(parse_expression());
            if (current_token().get_token() == TokenType::COMMA) {
                eat(TokenType::COMMA);
            } else {
                break;
            }
        }
    }
    eat(TokenType::RPAREN);

    return std::make_unique<CallNode>(name, std::move(args));
}

// ============================================================================
// Expressions  (lowest → highest precedence)
// ============================================================================

std::unique_ptr<ASTNode> Parser::parse_expression() {
    auto node = parse_add_sub();

    while (true) {
        TokenType tt = current_token().get_token();

        if (tt == TokenType::EQUAL) {
            Token op = eat(TokenType::EQUAL);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::NOTEQUAL) {
            Token op = eat(TokenType::NOTEQUAL);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::GREATER) {
            Token op = eat(TokenType::GREATER);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::GREATEREQ) {
            Token op = eat(TokenType::GREATEREQ);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::LESSER) {
            Token op = eat(TokenType::LESSER);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::LESSEREQ) {
            Token op = eat(TokenType::LESSEREQ);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::AND) {
            Token op = eat(TokenType::AND);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else if (tt == TokenType::OR) {
            Token op = eat(TokenType::OR);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_add_sub());
        } 
        
        else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parse_add_sub() {
    auto node = parse_factor();

    while (true) {
        TokenType tt = current_token().get_token();
        if (tt == TokenType::PLUS || tt == TokenType::MINUS) {
            Token op = eat(tt);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_factor());
        } 
        else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parse_factor() {
    auto node = parse_exponent();

    while (true) {
        TokenType tt = current_token().get_token();
        if (tt == TokenType::MULTIPLY || tt == TokenType::DIVIDE || tt == TokenType::MODULO) {
            Token op = eat(tt);
            node = std::make_unique<BinOpNode>(std::move(node), op, parse_exponent());
        } else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parse_exponent() {
    auto node = parse_parentheses();

    while (current_token().get_token() == TokenType::EXPONENT) {
        Token op = eat(TokenType::EXPONENT);
        node = std::make_unique<BinOpNode>(std::move(node), op, parse_parentheses());
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parse_parentheses() {
    if (current_token().get_token() == TokenType::LPAREN) {
        eat(TokenType::LPAREN);
        auto node = parse_expression();
        eat(TokenType::RPAREN);
        return node;
    }
    return parse_term();
}

std::unique_ptr<ASTNode> Parser::parse_term() {
    Token token = current_token();
    TokenType tt = token.get_token();

    if (tt == TokenType::NUMBER) {
        return std::make_unique<NumberNode>(eat(TokenType::NUMBER));
    }

    if (tt == TokenType::STRING) {
        return std::make_unique<StringNode>(eat(TokenType::STRING));
    }

    if (tt == TokenType::INT || tt == TokenType::STR || tt == TokenType::DOUBLE) {
        if (peek().get_token() == TokenType::LPAREN) {
            // We treat the keyword as an identifier so parse_call can handle it
            return parse_call_special(token.get_value()); 
        }
    }

    if (tt == TokenType::BOOL) {
        return std::make_unique<BooleanNode>(eat(TokenType::BOOL));
    }

    if (tt == TokenType::INPUT) {
        return parse_input();
    }

    if (tt == TokenType::IDENTIFIER) {
        if (peek().get_token() == TokenType::LPAREN) {
            return parse_call();
        }
        return std::make_unique<VarNode>(eat(TokenType::IDENTIFIER));
    }

    std::ostringstream oss;
    oss << "Expected expression, got " << to_string(tt) << " ('" << token.get_value() << "')";
    throw ParseException(oss.str());
}

std::unique_ptr<ASTNode> Parser::parse_call_special(std::string name) {
    // already have the 'name' (e.g., "int"), so just skip the token
    pos++; 
    
    eat(TokenType::LPAREN);
    std::vector<std::unique_ptr<ASTNode>> args;
    if (current_token().get_token() != TokenType::RPAREN) {
        args.push_back(parse_expression());
        while (current_token().get_token() == TokenType::COMMA) {
            eat(TokenType::COMMA);
            args.push_back(parse_expression());
        }
    }
    eat(TokenType::RPAREN);

    return std::make_unique<CallNode>(name, std::move(args));
}