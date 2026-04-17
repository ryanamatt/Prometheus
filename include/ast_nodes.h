#ifndef AST_NODES_H
#define AST_NODES_H

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "prometheus_types.h"

/**
 * @brief Base class for all nodes in the Abstract Syntax Tree.
 * Uses a virtual destructor to ensure derived classes are cleaned up correctly.
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

/**
 * @brief Represents a numeric literal (integer or float) in the AST.
 */
class NumberNode : public ASTNode {
public:
    Token token;
    std::string value;

    NumberNode(Token token) : token(token), value(token.get_value()) {}
};

/**
 * @brief Represents a string literal in the AST.
 */
class StringNode : public ASTNode {
public:
    Token token;
    std::string value;

    StringNode(Token token) : token(token), value(token.get_value()) {}
};

/**
 * @brief Represents a variable identifier usage
 */
class VarNode : public ASTNode {
public:
    Token token;
    std::string value;

    VarNode(Token token) : token(token), value(token.get_value()) {}
};

/**
 * @brief Represents a binary operation (e.g., +, -, *, ==)
 */
class BinOpNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    Token op;
    std::unique_ptr<ASTNode> right;

    BinOpNode(std::unique_ptr<ASTNode> left, Token op, std::unique_ptr<ASTNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

};

/**
 * @brief Represents a variable declaration (e.g., int x = 5;)
 */
class VarDeclNode : public ASTNode {
public:
    std::string var_type;
    std::string name;
    std::unique_ptr<ASTNode> value_node;

    VarDeclNode(std::string var_type, std::string name, std::unique_ptr<ASTNode> value_node)
        : var_type(var_type), name(name), value_node(std::move(value_node)) {}
};

/**
 * @brief Represents a print statement
 */
class PrintNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> expressions;

    PrintNode(std::vector<std::unique_ptr<ASTNode>> expressions)
        : expressions(std::move(expressions)) {}
};

/**
 * Represents an 'if-else' control flow structure
 */
class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    // Vector of pairs: {condition, branch_body}
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::vector<std::unique_ptr<ASTNode>>>> elif_branches;
    std::vector<std::unique_ptr<ASTNode>> else_branch;

    IfNode(std::unique_ptr<ASTNode> condition, 
           std::vector<std::unique_ptr<ASTNode>> then_branch,
           std::vector<std::pair<std::unique_ptr<ASTNode>, std::vector<std::unique_ptr<ASTNode>>>> elif_branches,
           std::vector<std::unique_ptr<ASTNode>> else_branch)
        : condition(std::move(condition)), then_branch(std::move(then_branch)), 
          elif_branches(std::move(elif_branches)), else_branch(std::move(else_branch)) {}
};

/**
 * Represents a While loop
 */
class WhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> do_branch;

    WhileNode(std::unique_ptr<ASTNode> condition, std::vector<std::unique_ptr<ASTNode>> do_branch)
        : condition(std::move(condition)), do_branch(std::move(do_branch)) {}
};

/**
 * Represents a For Loop
 */
class ForNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> variable;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> change_var;
    std::vector<std::unique_ptr<ASTNode>> do_branch;

    ForNode(std::unique_ptr<ASTNode> variable, std::unique_ptr<ASTNode> condition, 
            std::unique_ptr<ASTNode> change_var, std::vector<std::unique_ptr<ASTNode>> do_branch)
        : variable(std::move(variable)), condition(std::move(condition)), 
          change_var(std::move(change_var)), do_branch(std::move(do_branch)) {}
};

/**
 * Represents a function definition
 */
class FunctionDeclNode : public ASTNode {
public:
    std::string name;
    std::string return_type;
    std::vector<std::pair<std::string, std::string>> params; // {type, name}
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDeclNode(std::string name, std::string return_type, 
                     std::vector<std::pair<std::string, std::string>> params, 
                     std::vector<std::unique_ptr<ASTNode>> body)
        : name(name), return_type(return_type), params(params), body(std::move(body)) {}
};

/**
 * Represents a return statement
 */
class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value_node;

    ReturnNode(std::unique_ptr<ASTNode> value_node) : value_node(std::move(value_node)) {}
};

/**
 * Represents a function call
 */
class CallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    CallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : name(name), args(std::move(args)) {}
};

/**
 * Sentinel node representing the end of a statement stream
 */
class EOFNode : public ASTNode {};

#endif // AST_NODES_H