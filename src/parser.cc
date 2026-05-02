#include <sstream>
#include <unordered_set>
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
    throw ParseException(oss.str(), token.get_line());
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

    if (tt == TokenType::LIST) {
        return parse_list_decl();
    }

    if (tt == TokenType::IDENTIFIER) {
        if (peek().get_token() == TokenType::INCREMENT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::INCREMENT);
            if (current_token().get_token() != TokenType::SEMICOLON)
                throw MissingSemicolonException("'++", current_token().get_line());
            eat(TokenType::SEMICOLON);
            return std::make_unique<IncrementDecrementNode>(id.get_value(), 1.0);
        }

        else if (peek().get_token() == TokenType::DECREMENT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::DECREMENT);
            if (current_token().get_token() != TokenType::SEMICOLON)
                throw MissingSemicolonException("'--", current_token().get_line());
            eat(TokenType::SEMICOLON);
            return std::make_unique<IncrementDecrementNode>(id.get_value(), -1.0);
        }

        // name[index] = value;
        else if (peek().get_token() == TokenType::LBRACKET) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::LBRACKET);
            auto index = parse_expression();
            if (current_token().get_token() != TokenType::RBRACKET)
                throw MissingBraceException('[', id.get_line());
            eat(TokenType::RBRACKET);
            if (current_token().get_token() != TokenType::ASSIGN) {
                throw ParseException(
                    "Expected '=' after list index expression for '" + id.get_value() + "'",
                    current_token().get_line());
            }
            eat(TokenType::ASSIGN);
            auto value = parse_expression();
            if (current_token().get_token() != TokenType::SEMICOLON)
                throw MissingSemicolonException("list index assignment", current_token().get_line());
            eat(TokenType::SEMICOLON);
            return std::make_unique<ListAssignNode>(id.get_value(), std::move(index), std::move(value));
        }

        // name.append(expr); or name.len();
        else if (peek().get_token() == TokenType::DOT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::DOT);
            if (current_token().get_token() != TokenType::IDENTIFIER) {
                throw ParseException(
                    "Expected method name after '.' on '" + id.get_value() + "'",
                    current_token().get_line());
            }
            std::string method = eat(TokenType::IDENTIFIER).get_value();

            if (method == "append") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);
                auto val = parse_expression();
                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("append()", current_token().get_line());
                eat(TokenType::SEMICOLON);
                return std::make_unique<ListAppendNode>(id.get_value(), std::move(val));
            }

            if (method == "len") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);
                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("len()", current_token().get_line());
                eat(TokenType::SEMICOLON);
                return std::make_unique<ListLengthNode>(id.get_value());
            }

            if (method == "insert") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);

                auto index = parse_expression();

                if (current_token().get_token() != TokenType::COMMA)
                    throw InvalidArgumentException("insert(idx, val)", ",", 
                        current_token().get_value(), current_token().get_line());
                eat(TokenType::COMMA);
                auto value = parse_expression();

                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("insert()", current_token().get_line());
                eat(TokenType::SEMICOLON);

                return std::make_unique<ListInsertNode>(id.get_value(), std::move(index), 
                    std::move(value));
            }

            if (method == "pop") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);
                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());

                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("pop()", current_token().get_line());
                int line = current_token().get_line();
                eat(TokenType::SEMICOLON);

                return std::make_unique<ListPopNode>(id.get_value(), line);
            }

            if (method == "remove") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);

                auto value = parse_expression();

                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("pop()", current_token().get_line());
                int line = current_token().get_line();
                eat(TokenType::SEMICOLON);

                return std::make_unique<ListRemoveNode>(id.get_value(), std::move(value), line);
            }

            if (method == "clear") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', current_token().get_line());
                eat(TokenType::LPAREN);

                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                if (current_token().get_token() != TokenType::SEMICOLON)
                    throw MissingSemicolonException("pop()", current_token().get_line());
                int line = current_token().get_line();
                eat(TokenType::SEMICOLON);

                return std::make_unique<ListClearNode>(id.get_value(), line);
            }

            throw ParseException(
                "Unknown method '" + method + "' on '" + id.get_value() + "'",
                current_token().get_line());
        }

        else if (peek().get_token() == TokenType::LPAREN) {
            auto call_node = parse_call();
            if (current_token().get_token() != TokenType::SEMICOLON) {
                throw MissingSemicolonException(
                    "call to '" +
                    static_cast<CallNode*>(call_node.get())->name + "()'",
                    current_token().get_line());
            }
            eat(TokenType::SEMICOLON);
            return call_node;
        }
        return parse_identifier();
    }

    if (tt == TokenType::PRINT)  return parse_print();
    if (tt == TokenType::INPUT)  return parse_input();
    if (tt == TokenType::RANGE)  return parse_range();
    if (tt == TokenType::IF)     return parse_if();
    if (tt == TokenType::WHILE)  return parse_while();
    if (tt == TokenType::FOR)    return parse_for();
    if (tt == TokenType::FUNC)   return parse_func();
    if (tt == TokenType::RETURN) return parse_return();
    if (tt == TokenType::IMPORT) return parse_import();
    if (tt == TokenType::USE)    return parse_use();

    if (tt == TokenType::EOF_TOKEN) {
        pos++;
        return std::make_unique<EOFNode>();
    }

    std::ostringstream oss;
    oss << "Unexpected token: " << to_string(tt) << " ('" << token.get_value() << "')";
    throw ParseException(oss.str(), token.get_line());
}

std::unique_ptr<ASTNode> Parser::parse_declaration() {
    Token type_token = eat(current_token().get_token());   // INT / STR / DOUBLE / BOOL
    Token name_token = eat(TokenType::IDENTIFIER);

    std::unique_ptr<ASTNode> value_node;

    if (current_token().get_token() == TokenType::ASSIGN) {
            eat(TokenType::ASSIGN);
            value_node = parse_expression();
    } else {
        // Provide a default value of 0 if no assignment exists
        if (type_token.get_token() == TokenType::STR)
            value_node = std::make_unique<StringNode>(Token(TokenType::STRING, ""));

        else if (type_token.get_token() == TokenType::BOOL)
            value_node = std::make_unique<BooleanNode>(Token(TokenType::BOOL, "false"));

        else
            value_node = std::make_unique<NumberNode>(Token(TokenType::NUMBER, "0"));
    }

    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException(
            "declaration of '" + name_token.get_value() + "'",
            current_token().get_line());
    }
    
    int line = current_token().get_line();
    eat(TokenType::SEMICOLON);
    return std::make_unique<VarDeclNode>(type_token.get_value(), name_token.get_value(), std::move(value_node), line);
}

std::unique_ptr<ListDeclNode> Parser::parse_list_decl() {
    Token list_tok = eat(TokenType::LIST);

    // list[element_type]
    if (current_token().get_token() != TokenType::LBRACKET)
        throw MissingBraceException('[', list_tok.get_line());
    eat(TokenType::LBRACKET);

    TokenType et = current_token().get_token();
    if (et != TokenType::INT && et != TokenType::STR &&
        et != TokenType::DOUBLE && et != TokenType::BOOL) {
        throw ParseException(
            "Expected element type (int, double, str, bool) inside list[...], got '" +
            current_token().get_value() + "'",
            current_token().get_line());
    }
    std::string element_type = eat(et).get_value();

    if (current_token().get_token() != TokenType::RBRACKET)
        throw MissingBraceException('[', list_tok.get_line());
    eat(TokenType::RBRACKET);

    // variable name
    if (current_token().get_token() != TokenType::IDENTIFIER) {
        throw ParseException(
            "Expected variable name after list[" + element_type + "]",
            current_token().get_line());
    }
    std::string name = eat(TokenType::IDENTIFIER).get_value();

    // optional initialiser (defaults to empty list)
    std::unique_ptr<ASTNode> value_node;
    if (current_token().get_token() == TokenType::ASSIGN) {
        eat(TokenType::ASSIGN);
        value_node = parse_expression(); // should parse a ListLiteralNode
    } else {
        // default: empty list literal
        value_node = std::make_unique<ListLiteralNode>(
            std::vector<std::unique_ptr<ASTNode>>{});
    }

    if (current_token().get_token() != TokenType::SEMICOLON)
        throw MissingSemicolonException("list declaration of '" + name + "'",
                                        current_token().get_line());
    eat(TokenType::SEMICOLON);
    return std::make_unique<ListDeclNode>(element_type, name, std::move(value_node));
}

std::unique_ptr<ASTNode> Parser::parse_identifier() {
    Token id_token = eat(TokenType::IDENTIFIER);
 
    if (current_token().get_token() != TokenType::ASSIGN) {
        std::ostringstream oss;
        oss << "Expected '=' after identifier '" << id_token.get_value()
            << "', got " << to_string(current_token().get_token())
            << " ('" << current_token().get_value() << "')";
        throw ParseException(oss.str(), current_token().get_line());
    }
    eat(TokenType::ASSIGN);
 
    auto value_node = parse_expression();
 
    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException(
            "assignment to '" + id_token.get_value() + "'",
            current_token().get_line());
    }
    int line = current_token().get_line();
    eat(TokenType::SEMICOLON);
    return std::make_unique<VarDeclNode>(id_token.get_value(), id_token.get_value(), std::move(value_node), line);
}

std::unique_ptr<PrintNode> Parser::parse_print() {
    Token print_tok = eat(TokenType::PRINT);
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', current_token().get_line());
    }
    eat(TokenType::LPAREN);
 
    if (current_token().get_token() == TokenType::RPAREN) {
        throw ParseException("print() requires at least one argument", print_tok.get_line());
    }
 
    std::vector<std::unique_ptr<ASTNode>> expressions;
    expressions.push_back(parse_expression());
 
    while (current_token().get_token() == TokenType::COMMA) {
        eat(TokenType::COMMA);
        if (current_token().get_token() == TokenType::RPAREN) {
            throw ParseException("Trailing comma in print() argument list", current_token().get_line());
        }
        expressions.push_back(parse_expression());
    }
 
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', print_tok.get_line());
    }
    eat(TokenType::RPAREN);
 
    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException("print() statement", current_token().get_line());
    }
    eat(TokenType::SEMICOLON);
    return std::make_unique<PrintNode>(std::move(expressions));
}

std::unique_ptr<InputNode> Parser::parse_input() {
    Token input_tok = eat(TokenType::INPUT);
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', current_token().get_line());
    }
    eat(TokenType::LPAREN);
 
    std::string prompt = "";
    if (current_token().get_token() == TokenType::STRING) {
        prompt = eat(TokenType::STRING).get_value();
    } else if (current_token().get_token() != TokenType::RPAREN) {
        throw ParseException(
            "input() prompt must be a string literal",
            current_token().get_line());
    }
 
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', input_tok.get_line());
    }
    eat(TokenType::RPAREN);
    return std::make_unique<InputNode>(prompt);
}

std::unique_ptr<RangeNode> Parser::parse_range() {
    Token range_tok = eat(TokenType::RANGE);
    eat(TokenType::LPAREN);

    std::vector<std::unique_ptr<ASTNode>> args;
    // Parse arguments separated by commas
    while (current_token().get_token() != TokenType::RPAREN) {
        args.push_back(parse_expression());
        if (current_token().get_token() == TokenType::COMMA) {
            eat(TokenType::COMMA);
        } else {
            break;
        }
    }
    eat(TokenType::RPAREN);

    std::unique_ptr<ASTNode> start, stop, step;

    // Logic similar to Python's range()
    if (args.size() == 1) {
        // range(stop) -> starts at 0, step is 1
        start = std::make_unique<NumberNode>(Token(TokenType::NUMBER, "0"));
        stop  = std::move(args[0]);
        step  = std::make_unique<NumberNode>(Token(TokenType::NUMBER, "1"));
    } else if (args.size() == 2) {
        // range(start, stop) -> step is 1
        start = std::move(args[0]);
        stop  = std::move(args[1]);
        step  = std::make_unique<NumberNode>(Token(TokenType::NUMBER, "1"));
    } else if (args.size() == 3) {
        // range(start, stop, step)
        start = std::move(args[0]);
        stop  = std::move(args[1]);
        step  = std::move(args[2]);
    } else {
        throw ParseException("range() expects 1, 2, or 3 arguments", range_tok.get_line());
    }

    return std::make_unique<RangeNode>(std::move(start), std::move(stop), std::move(step));
}

std::unique_ptr<IfNode> Parser::parse_if() {
    Token if_tok = eat(TokenType::IF);
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', if_tok.get_line());
    }
    eat(TokenType::LPAREN);
    auto condition = parse_expression();
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', if_tok.get_line());
    }
    eat(TokenType::RPAREN);
 
    if (current_token().get_token() != TokenType::LBRACE) {
        throw MissingBraceException('{', if_tok.get_line());
    }
    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        if (current_token().get_token() == TokenType::EOF_TOKEN) {
            throw MissingBraceException('{', if_tok.get_line());
        }
        then_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);
 
    // Zero or more elif branches
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::vector<std::unique_ptr<ASTNode>>>> elif_branches;
    while (current_token().get_token() == TokenType::ELIF) {
        Token elif_tok = eat(TokenType::ELIF);
 
        if (current_token().get_token() != TokenType::LPAREN) {
            throw MissingBraceException('(', elif_tok.get_line());
        }
        eat(TokenType::LPAREN);
        auto elif_cond = parse_expression();
        if (current_token().get_token() != TokenType::RPAREN) {
            throw MissingBraceException('(', elif_tok.get_line());
        }
        eat(TokenType::RPAREN);
 
        if (current_token().get_token() != TokenType::LBRACE) {
            throw MissingBraceException('{', elif_tok.get_line());
        }
        eat(TokenType::LBRACE);
        std::vector<std::unique_ptr<ASTNode>> elif_stmts;
        while (current_token().get_token() != TokenType::RBRACE) {
            if (current_token().get_token() == TokenType::EOF_TOKEN) {
                throw MissingBraceException('{', elif_tok.get_line());
            }
            elif_stmts.push_back(parse_statement());
        }
        eat(TokenType::RBRACE);
 
        elif_branches.emplace_back(std::move(elif_cond), std::move(elif_stmts));
    }
 
    // Optional else branch
    std::vector<std::unique_ptr<ASTNode>> else_branch;
    if (current_token().get_token() == TokenType::ELSE) {
        Token else_tok = eat(TokenType::ELSE);
 
        if (current_token().get_token() != TokenType::LBRACE) {
            throw MissingBraceException('{', else_tok.get_line());
        }
        eat(TokenType::LBRACE);
        while (current_token().get_token() != TokenType::RBRACE) {
            if (current_token().get_token() == TokenType::EOF_TOKEN) {
                throw MissingBraceException('{', else_tok.get_line());
            }
            else_branch.push_back(parse_statement());
        }
        eat(TokenType::RBRACE);
    }
 
    return std::make_unique<IfNode>(std::move(condition), std::move(then_branch),
                                    std::move(elif_branches), std::move(else_branch));
}

std::unique_ptr<WhileNode> Parser::parse_while() {
    Token while_tok = eat(TokenType::WHILE);
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', while_tok.get_line());
    }
    eat(TokenType::LPAREN);
    auto condition = parse_expression();
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', while_tok.get_line());
    }
    eat(TokenType::RPAREN);
 
    if (current_token().get_token() != TokenType::LBRACE) {
        throw MissingBraceException('{', while_tok.get_line());
    }
    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> do_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        if (current_token().get_token() == TokenType::EOF_TOKEN) {
            throw MissingBraceException('{', while_tok.get_line());
        }
        do_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);
 
    return std::make_unique<WhileNode>(std::move(condition), std::move(do_branch));
}

std::unique_ptr<ASTNode> Parser::parse_for() {
    Token for_tok = eat(TokenType::FOR);
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', for_tok.get_line());
    }
    eat(TokenType::LPAREN);


    // ----------------------------------------------------------------
    // Peek-ahead: is this a range-based for-in loop?
    //   for (int i : nums) { ... }
    // Pattern: TYPE  IDENTIFIER  COLON  ...
    // We look two tokens ahead (from current pos):
    //   tokens[pos]   = type keyword (INT / STR / DOUBLE / BOOL)
    //   tokens[pos+1] = identifier
    //   tokens[pos+2] = COLON
    // ----------------------------------------------------------------
    bool is_for_in = false;
    if (pos + 2 < (int)tokens.size()) {
        TokenType t0 = tokens[pos].get_token();
        TokenType t2 = tokens[pos + 2].get_token();
        bool type_kw = (t0 == TokenType::INT   || t0 == TokenType::STR ||
                        t0 == TokenType::DOUBLE || t0 == TokenType::BOOL);
        is_for_in = type_kw
                    && tokens[pos + 1].get_token() == TokenType::IDENTIFIER
                    && t2 == TokenType::COLON;
    }

    if (is_for_in) {
        // --- for (type name : list_expr) { body } ---
        std::string var_type = eat(current_token().get_token()).get_value(); // consume type
        std::string var_name = eat(TokenType::IDENTIFIER).get_value();
        eat(TokenType::COLON);

        auto list_expr = parse_expression();

        if (current_token().get_token() != TokenType::RPAREN)
            throw MissingBraceException('(', for_tok.get_line());
        eat(TokenType::RPAREN);

        if (current_token().get_token() != TokenType::LBRACE)
            throw MissingBraceException('{', for_tok.get_line());
        eat(TokenType::LBRACE);

        std::vector<std::unique_ptr<ASTNode>> body;
        while (current_token().get_token() != TokenType::RBRACE) {
            if (current_token().get_token() == TokenType::EOF_TOKEN)
                throw MissingBraceException('{', for_tok.get_line());
            body.push_back(parse_statement());
        }
        eat(TokenType::RBRACE);

        return std::make_unique<ForInNode>(var_type, var_name,
                                           std::move(list_expr), std::move(body));
    }

    // --- Classic for (init; cond; update;) { body } ---

    auto var       = parse_declaration();      // init statement (consumes its own semicolon)
    auto condition = parse_expression();
    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException("for-loop condition", current_token().get_line());
    }
    eat(TokenType::SEMICOLON);

    // Update clause: i++, i--, or identifier assignment (e.g. j = j + 2).
    // No trailing semicolon is consumed — the closing ')' follows immediately.
    std::unique_ptr<ASTNode> change_var;
    if (current_token().get_token() == TokenType::IDENTIFIER &&
        peek().get_token() == TokenType::INCREMENT) {
        Token id = eat(TokenType::IDENTIFIER);
        eat(TokenType::INCREMENT);
        change_var = std::make_unique<IncrementDecrementNode>(id.get_value(), 1.0);
    } 
    
    else if (current_token().get_token() == TokenType::IDENTIFIER &&
               peek().get_token() == TokenType::DECREMENT) {
        Token id = eat(TokenType::IDENTIFIER);
        eat(TokenType::DECREMENT);
        change_var = std::make_unique<IncrementDecrementNode>(id.get_value(), -1.0);
    } 
    
    else if (current_token().get_token() == TokenType::IDENTIFIER &&
               peek().get_token() == TokenType::ASSIGN) {
        Token id = eat(TokenType::IDENTIFIER);
        int line = id.get_line();
        eat(TokenType::ASSIGN);
        auto value_node = parse_expression();
        change_var = std::make_unique<VarDeclNode>(id.get_value(), id.get_value(), std::move(value_node), line);
    } 
    
    else
        throw ParseException("Invalid for-loop update clause", current_token().get_line());

    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', for_tok.get_line());
    }
    eat(TokenType::RPAREN);
 
    if (current_token().get_token() != TokenType::LBRACE) {
        throw MissingBraceException('{', for_tok.get_line());
    }
    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> do_branch;
    while (current_token().get_token() != TokenType::RBRACE) {
        if (current_token().get_token() == TokenType::EOF_TOKEN) {
            throw MissingBraceException('{', for_tok.get_line());
        }
        do_branch.push_back(parse_statement());
    }
    eat(TokenType::RBRACE);
 
    return std::make_unique<ForNode>(std::move(var), std::move(condition),
                                     std::move(change_var), std::move(do_branch));
}

std::unique_ptr<FunctionDeclNode> Parser::parse_func() {
    Token func_tok = eat(TokenType::FUNC);
 
    // Parse Return type
    TokenType rtt = current_token().get_token();
    if (rtt != TokenType::INT && rtt != TokenType::STR && rtt != TokenType::DOUBLE &&
        rtt != TokenType::BOOL && rtt != TokenType::VOID && rtt != TokenType::IDENTIFIER &&
        rtt != TokenType::LIST) {
        throw ParseException("Expected return type after 'func'", current_token().get_line());
    }
    std::string return_type = eat(rtt).get_value();

    if (rtt == TokenType::LIST) {
        if (current_token().get_token() != TokenType::LBRACKET)
            throw MissingBraceException('[', current_token().get_line());
        eat(TokenType::LBRACKET);
        
        // Get the inner type (e.g., int)
        return_type += "[" + current_token().get_value() + "]";
        eat(current_token().get_token()); 
        
        if (current_token().get_token() != TokenType::RBRACKET)
            throw MissingBraceException(']', current_token().get_line());
        eat(TokenType::RBRACKET);
    }
    
 
    // Parse Function Name
    if (current_token().get_token() != TokenType::IDENTIFIER) {
        throw ParseException("Expected function name", current_token().get_line());
    }
    std::string name = eat(TokenType::IDENTIFIER).get_value();
 
    eat(TokenType::LPAREN);
 
    // Parse Parameters
    std::vector<Parameter> params;
    std::unordered_set<std::string> seen_params;
 
    if (current_token().get_token() != TokenType::RPAREN) {
        while (true) {
            TokenType pt = current_token().get_token();
            if (pt != TokenType::INT && pt != TokenType::STR &&
                pt != TokenType::DOUBLE && pt != TokenType::BOOL &&
                pt != TokenType::LIST && pt != TokenType::IDENTIFIER) {
                throw ParseException("Invalid parameter type", current_token().get_line());
            }
 
            std::string p_full_type = eat(pt).get_value();

            // Parse List Type for Parameters
            if (pt == TokenType::LIST) {
                if (current_token().get_token() != TokenType::LBRACKET)
                    throw MissingBraceException('[', current_token().get_line());
                eat(TokenType::LBRACKET);
                
                // Get the inner type (e.g., int)
                p_full_type += "[" + current_token().get_value() + "]";
                eat(current_token().get_token()); 
                
                if (current_token().get_token() != TokenType::RBRACKET)
                    throw MissingBraceException(']', current_token().get_line());
                eat(TokenType::RBRACKET);
            }
 
            if (current_token().get_token() != TokenType::IDENTIFIER) {
                throw ParseException(
                    "Expected parameter name after type '" + p_full_type + "'",
                    current_token().get_line());
            }
            Token p_name_tok = eat(TokenType::IDENTIFIER);
            std::string p_name = p_name_tok.get_value();

            std::unique_ptr<ASTNode> default_value = nullptr;
            if (current_token().get_token() == TokenType::ASSIGN) {
                eat(TokenType::ASSIGN);
                default_value = parse_expression();
            }
 
            if (!seen_params.insert(p_name).second) {
                throw DuplicateParamException(name, p_name, p_name_tok.get_line());
            }
            params.emplace_back(p_full_type, p_name, std::move(default_value));
 
            if (current_token().get_token() == TokenType::COMMA) {
                eat(TokenType::COMMA);
            } else {
                break;
            }
        }
    }
 
    eat(TokenType::RPAREN);
    
    // Parse Function Body
    eat(TokenType::LBRACE);
    std::vector<std::unique_ptr<ASTNode>> body;
    while (current_token().get_token() != TokenType::RBRACE) {
        if (current_token().get_token() == TokenType::EOF_TOKEN) throw MissingBraceException('{', func_tok.get_line());
        auto stmt = parse_statement();
        if (stmt) body.push_back(std::move(stmt));
    }
    eat(TokenType::RBRACE);
 
    return std::make_unique<FunctionDeclNode>(name, return_type, std::move(params), std::move(body));
}

std::unique_ptr<ReturnNode> Parser::parse_return() {
    Token ret_tok = eat(TokenType::RETURN);

    // Bare `return;` — valid in void functions
    if (current_token().get_token() == TokenType::SEMICOLON) {
        eat(TokenType::SEMICOLON);
        return std::make_unique<ReturnNode>(nullptr);
    }

    auto value = parse_expression();

    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException("return statement", current_token().get_line());
    }
    eat(TokenType::SEMICOLON);
    return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<CallNode> Parser::parse_call() {
    Token name_tok = eat(TokenType::IDENTIFIER);
    std::string name = name_tok.get_value();
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', current_token().get_line());
    }
    eat(TokenType::LPAREN);
 
    std::vector<std::unique_ptr<ASTNode>> args;
    if (current_token().get_token() != TokenType::RPAREN) {
        while (true) {
            args.push_back(parse_expression());
            if (current_token().get_token() == TokenType::COMMA) {
                eat(TokenType::COMMA);
                if (current_token().get_token() == TokenType::RPAREN) {
                    throw ParseException(
                        "Trailing comma in argument list of call to '" + name + "'",
                        current_token().get_line());
                }
            } else {
                break;
            }
        }
    }
 
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', name_tok.get_line());
    }
    eat(TokenType::RPAREN);
 
    // Function calls used as statements need a semicolon; calls inside
    // expressions do not. The statement-level call site (parse_statement)
    // will eat the semicolon after parse_call() returns — nothing to do here.
 
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
    auto node = parse_unary();

    while (current_token().get_token() == TokenType::EXPONENT) {
        Token op = eat(TokenType::EXPONENT);
        node = std::make_unique<BinOpNode>(std::move(node), op, parse_parentheses());
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parse_unary() {
    TokenType tt = current_token().get_token();

    if (tt == TokenType::NOT || tt == TokenType::MINUS) {
        Token op = eat(tt);
        // call parse_parentheses so it can handle !(a > b)
        return std::make_unique<UnaryOpNode>(op, parse_unary());
    }
    return parse_parentheses(); 
}

std::unique_ptr<ASTNode> Parser::parse_parentheses() {
    if (current_token().get_token() == TokenType::LPAREN) {
        Token lparen = eat(TokenType::LPAREN);
        auto node = parse_expression();
        if (current_token().get_token() != TokenType::RPAREN) {
            throw MissingBraceException('(', lparen.get_line());
        }
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

    // List literal: [expr, expr, ...]
    if (tt == TokenType::LBRACKET) {
        Token lb = eat(TokenType::LBRACKET);
        std::vector<std::unique_ptr<ASTNode>> elems;
        if (current_token().get_token() != TokenType::RBRACKET) {
            elems.push_back(parse_expression());
            while (current_token().get_token() == TokenType::COMMA) {
                eat(TokenType::COMMA);
                if (current_token().get_token() == TokenType::RBRACKET)
                    throw ParseException("Trailing comma in list literal", current_token().get_line());
                elems.push_back(parse_expression());
            }
        }
        if (current_token().get_token() != TokenType::RBRACKET)
            throw MissingBraceException('[', lb.get_line());
        eat(TokenType::RBRACKET);
        return std::make_unique<ListLiteralNode>(std::move(elems));
    }

    if (tt == TokenType::INT || tt == TokenType::STR || tt == TokenType::DOUBLE) {
        if (peek().get_token() == TokenType::LPAREN) {
            // Treat the keyword as an identifier so parse_call can handle it
            return parse_call_special(token.get_value()); 
        }
    }

    if (tt == TokenType::BOOL) {
        Token bool_tok = eat(TokenType::BOOL);
        if (bool_tok.get_value() != "true" && bool_tok.get_value() != "false") {
            throw ParseException(
                "Invalid boolean literal '" + bool_tok.get_value() + "'",
                bool_tok.get_line());
        }
        return std::make_unique<BooleanNode>(bool_tok);
    }

    if (tt == TokenType::INPUT) return parse_input();
    if (tt == TokenType::RANGE) return parse_range();

    if (tt == TokenType::IDENTIFIER) {
        if (peek().get_token() == TokenType::LPAREN) {
            return parse_call();
        }
        // name[index] — index read as an expression
        if (peek().get_token() == TokenType::LBRACKET) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::LBRACKET);
            auto index = parse_expression();
            if (current_token().get_token() != TokenType::RBRACKET)
                throw MissingBraceException('[', id.get_line());
            eat(TokenType::RBRACKET);
            return std::make_unique<ListIndexNode>(id.get_value(), std::move(index));
        }
        // name.len() as an expression (e.g. used in conditions)
        if (peek().get_token() == TokenType::DOT) {
            Token id = eat(TokenType::IDENTIFIER);
            eat(TokenType::DOT);
            if (current_token().get_token() != TokenType::IDENTIFIER)
                throw ParseException("Expected method name after '.'", current_token().get_line());
            std::string method = eat(TokenType::IDENTIFIER).get_value();

            if (method == "len") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::LPAREN);
                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                return std::make_unique<ListLengthNode>(id.get_value());
            }

            if (method == "pop") {
                if (current_token().get_token() != TokenType::LPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::LPAREN);
                if (current_token().get_token() != TokenType::RPAREN)
                    throw MissingBraceException('(', id.get_line());
                eat(TokenType::RPAREN);
                return std::make_unique<ListPopNode>(id.get_value(), id.get_line());
            }

            throw ParseException(
                "Unknown expression method '" + method + "' on '" + id.get_value() + "'",
                current_token().get_line());
        }
        return std::make_unique<VarNode>(eat(TokenType::IDENTIFIER));
    }

    std::ostringstream oss;
    oss << "Expected expression, got " << to_string(tt) << " ('" << token.get_value() << "')";
    throw ParseException(oss.str(), token.get_line());
}

std::unique_ptr<ASTNode> Parser::parse_call_special(std::string name) {
    int tok_line = current_token().get_line();
    pos++; // skip the type-keyword token
 
    if (current_token().get_token() != TokenType::LPAREN) {
        throw MissingBraceException('(', current_token().get_line());
    }
    eat(TokenType::LPAREN);
 
    std::vector<std::unique_ptr<ASTNode>> args;
    if (current_token().get_token() != TokenType::RPAREN) {
        args.push_back(parse_expression());
        while (current_token().get_token() == TokenType::COMMA) {
            eat(TokenType::COMMA);
            if (current_token().get_token() == TokenType::RPAREN) {
                throw ParseException(
                    "Trailing comma in argument list of '" + name + "()'",
                    current_token().get_line());
            }
            args.push_back(parse_expression());
        }
    } else {
        throw ParseException(
            "Type conversion '" + name + "()' requires exactly one argument",
            tok_line);
    }
 
    if (args.size() != 1) {
        throw ParseException(
            "Type conversion '" + name + "()' requires exactly one argument, got " +
            std::to_string(args.size()),
            tok_line);
    }
 
    if (current_token().get_token() != TokenType::RPAREN) {
        throw MissingBraceException('(', tok_line);
    }
    eat(TokenType::RPAREN);
 
    return std::make_unique<CallNode>(name, std::move(args));
}
// ============================================================================
// Import / Use
// ============================================================================

/**
 * import path/to/file.prm;
 *
 * The path is assembled by consuming alternating tokens that can form a
 * file-system path: IDENTIFIER, DOT, DIVIDE (/), MINUS (-).
 * This covers common forms such as:
 *   import utils;
 *   import utils.prm;
 *   import helpers/utils;
 *   import helpers/utils.prm;
 *   import ../shared/utils.prm;
 */
std::unique_ptr<ImportNode> Parser::parse_import() {
    Token import_tok = eat(TokenType::IMPORT);

    std::string path;

    // Consume leading "../" sequences (two DOTs followed by optional DIVIDE)
    while (pos + 1 < (int)tokens.size() &&
           tokens[pos].get_token()     == TokenType::DOT &&
           tokens[pos + 1].get_token() == TokenType::DOT) {
        path += "..";
        pos += 2;
        if (current_token().get_token() == TokenType::DIVIDE) {
            path += '/';
            pos++;
        }
    }

    // The first segment must be an identifier
    if (current_token().get_token() != TokenType::IDENTIFIER) {
        throw ParseException(
            "Expected a file path after 'import'",
            import_tok.get_line());
    }

    // Assemble path from tokens until semicolon / EOF.
    // Allowed inter-segment tokens: IDENTIFIER, DOT, DIVIDE, MINUS.
    while (true) {
        TokenType tt = current_token().get_token();
        if (tt == TokenType::IDENTIFIER) {
            path += eat(TokenType::IDENTIFIER).get_value();
        } else if (tt == TokenType::DOT) {
            path += '.';
            pos++;
        } else if (tt == TokenType::DIVIDE) {
            path += '/';
            pos++;
        } else if (tt == TokenType::MINUS) {
            path += '-';
            pos++;
        } else {
            break;
        }
    }

    if (path.empty()) {
        throw ParseException("Empty path in import statement", import_tok.get_line());
    }

    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException("import statement", current_token().get_line());
    }
    eat(TokenType::SEMICOLON);

    return std::make_unique<ImportNode>(path);
}

/**
 * use module_name;
 *
 * Module names follow identifier rules: letters, digits, underscores.
 */
std::unique_ptr<UseNode> Parser::parse_use() {
    Token use_tok = eat(TokenType::USE);

    if (current_token().get_token() != TokenType::IDENTIFIER) {
        throw ParseException(
            "Expected a module name after 'use'",
            use_tok.get_line());
    }

    std::string module_name = eat(TokenType::IDENTIFIER).get_value();

    if (current_token().get_token() != TokenType::SEMICOLON) {
        throw MissingSemicolonException("use statement", current_token().get_line());
    }
    eat(TokenType::SEMICOLON);

    return std::make_unique<UseNode>(module_name);
}