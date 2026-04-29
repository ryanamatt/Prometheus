#ifndef PARSER_H
#define PARSER_H

#include "ast_nodes.h"
#include "exceptions.h"

/**
 * @brief Implements a recursive-descent parser that converts a token stream
 *        into an Abstract Syntax Tree (AST).
 */
class Parser {
private:
    std::vector<Token> tokens;
    int pos;

public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), pos(0) {}

    /**
     * @brief Entry point – consumes all tokens and returns a list of top-level
     *        statement nodes.
     */
    std::vector<std::unique_ptr<ASTNode>> parse();

private:

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    /** Returns the token currently pointed to (or a synthetic EOF token). */
    Token current_token();

    /** 
     * @brief Returns the token one ahead of the current position without consuming it. 
     */
    Token peek();

    /**
     * @brief Validates that the current token matches `expected_type`, advances
     *        the position, and returns the consumed token.
     * @throws ParseException on a type mismatch.
     */
    Token eat(TokenType expected_type);

    // -----------------------------------------------------------------------
    // Statements
    // -----------------------------------------------------------------------

    /** 
     * @brief Dispatches to the appropriate method based on the current token. 
     */
    std::unique_ptr<ASTNode> parse_statement();

    /** 
     * @brief Parses a typed variable declaration: `int x = <expr>;` 
     */
    std::unique_ptr<ASTNode> parse_declaration();

    /**
     * @brief Parses a list declaration: list[int] nums = [1, 2, 3];
     */
    std::unique_ptr<ListDeclNode> parse_list_decl();

    /** 
     * @brief Parses a bare identifier assignment: `x = <expr>;`
     */
    std::unique_ptr<ASTNode> parse_identifier();

    /** 
     * @brief Parses a `print(<expr>, ...);` statement.
     */
    std::unique_ptr<PrintNode> parse_print();

    /**
     * @brief Parsers a `str var = input("String");` statement.
     */
    std::unique_ptr<InputNode> parse_input();

    /**
     * @brief Parses the range function
     */
    std::unique_ptr<RangeNode> parse_range();

    /** 
     * @brief Parses an `if (...) { } elif (...) { } else { }` construct.
     */
    std::unique_ptr<IfNode> parse_if();

    /** 
     * @brief Parses a `while (...) { }` loop.
     */
    std::unique_ptr<WhileNode> parse_while();

    /** 
     * @brief Parses a `for (<decl> <cond>; <stmt>) { }` loop. 
     */
    std::unique_ptr<ASTNode> parse_for();

    /** @brief Parses a `func <type> <name>(...) { }` function definition. */
    std::unique_ptr<FunctionDeclNode> parse_func();

    /** @brief Parses a `return <expr>;` statement.*/
    std::unique_ptr<ReturnNode> parse_return();

    /**
     * @brief Parses `import path/to/file;` — bare path, no quotes.
     */
    std::unique_ptr<ImportNode> parse_import();

    /**
     * @brief Parses `use module_name;` — loads a stdlib module lazily.
     */
    std::unique_ptr<UseNode> parse_use();

    /** 
     * @brief Parses a `<name>(arg, ...)` function call expression. 
     */
    std::unique_ptr<CallNode> parse_call();

    // -----------------------------------------------------------------------
    // Expressions  (lowest → highest precedence)
    // -----------------------------------------------------------------------

    /** 
     * @brief Comparison / logical operators: ==, !=, >, >=, <, <=, &&, ||
     */
    std::unique_ptr<ASTNode> parse_expression();

    /** 
     * @brief Additive operators: +, - *
     */
    std::unique_ptr<ASTNode> parse_add_sub();

    /** 
     * @brief Multiplicative operators: *, /, % 
     * */
    std::unique_ptr<ASTNode> parse_factor();

    /** 
     * @brief Exponentiation operator: ** 
     * 
    */
    std::unique_ptr<ASTNode> parse_exponent();

    /**
     * @brief Parses a unary operator: !
     */
    std::unique_ptr<ASTNode> parse_unary();

    /** 
     * @brief Grouped sub-expression inside parentheses: `( <expr> )` 
     * */
    std::unique_ptr<ASTNode> parse_parentheses();

    /**
     * @brief Atomic terms: number literals, string literals, identifiers, call sites. 
     * */
    std::unique_ptr<ASTNode> parse_term();

    /**
     * @brief Handles type conversions int(), double(), str()
     */
    std::unique_ptr<ASTNode> parse_call_special(std::string name);
};

#endif // PARSER_H