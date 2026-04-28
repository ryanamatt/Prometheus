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
 * @brief Represents a boolean literal in the AST.
 */
class BooleanNode : public ASTNode {
public:
    Token token;
    std::string value;

    BooleanNode(Token token) : token(token), value(token.get_value()) {}
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
 * @brief Represents a unary operation (e.g., ! flag)
 */
class UnaryOpNode : public ASTNode {
public:
    Token op;
    std::unique_ptr<ASTNode> right;

    UnaryOpNode(Token op, std::unique_ptr<ASTNode> right)
        : op(op), right(std::move(right)) {}
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
 * @brief Represents an Input statement
 */
class InputNode : public ASTNode {
public:
    std::string msg;

    InputNode(std::string msg) : msg(msg) {}
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

struct Parameter {
    std::string type;
    std::string name;
    std::unique_ptr<ASTNode> default_val;

    // Add this constructor to allow moving the unique_ptr
    Parameter(std::string t, std::string n, std::unique_ptr<ASTNode> dv)
        : type(std::move(t)), name(std::move(n)), default_val(std::move(dv)) {}

    // Since it contains a unique_ptr, the compiler will correctly 
    // allow moves but forbid copies automatically.
};

/**
 * Represents a function definition
 */
class FunctionDeclNode : public ASTNode {
public:
    std::string name;
    std::string return_type;
    std::vector<Parameter> params;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDeclNode(std::string name, std::string return_type, 
                     std::vector<Parameter> params, // Pass by value
                     std::vector<std::unique_ptr<ASTNode>> body)
        : name(std::move(name)), 
          return_type(std::move(return_type)), 
          params(std::move(params)),
          body(std::move(body)) {}
};

/**
 * Represents a return statement. value_node may be nullptr for void returns.
 */
class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value_node; // nullptr for bare `return;`

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
 * Represents a list literal, e.g. [1, 2, 3]
 */
class ListLiteralNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;

    ListLiteralNode(std::vector<std::unique_ptr<ASTNode>> elements)
        : elements(std::move(elements)) {}
};

/**
 * Represents a typed list declaration, e.g. list[int] nums = [1, 2, 3];
 */
class ListDeclNode : public ASTNode {
public:
    std::string element_type;   // "int", "double", "str", "bool"
    std::string name;
    std::unique_ptr<ASTNode> value_node; // ListLiteralNode or VarNode

    ListDeclNode(std::string element_type, std::string name,
                 std::unique_ptr<ASTNode> value_node)
        : element_type(std::move(element_type)), name(std::move(name)),
          value_node(std::move(value_node)) {}
};

/**
 * Represents an index read: name[expr]
 */
class ListIndexNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> index;

    ListIndexNode(std::string name, std::unique_ptr<ASTNode> index)
        : name(std::move(name)), index(std::move(index)) {}
};

/**
 * Represents an index assignment: name[expr] = expr;
 */
class ListAssignNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> index;
    std::unique_ptr<ASTNode> value;

    ListAssignNode(std::string name, std::unique_ptr<ASTNode> index,
                   std::unique_ptr<ASTNode> value)
        : name(std::move(name)), index(std::move(index)), value(std::move(value)) {}
};

/**
 * Represents name.append(expr);
 */
class ListAppendNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> value;

    ListAppendNode(std::string name, std::unique_ptr<ASTNode> value)
        : name(std::move(name)), value(std::move(value)) {}
};

/**
 * Represents name.len() — evaluates to int
 */
class ListLengthNode : public ASTNode {
public:
    std::string name;

    ListLengthNode(std::string name) : name(std::move(name)) {}
};

/**
 * Represents `import filename;`
 * Resolves relative to the importing file's directory (or CWD in the REPL).
 * The resolved path is stored after the initial parse so the interpreter
 * can read, lex, parse, and execute the target file inline.
 */
class ImportNode : public ASTNode {
public:
    std::string path;      // exactly as written by the user (no quotes)
    std::string base_dir;  // set by the interpreter/main before execution
 
    ImportNode(std::string path, std::string base_dir = "")
        : path(std::move(path)), base_dir(std::move(base_dir)) {}
};
 
/**
 * @brief Represents `use module_name;`
 * Names a standard-library module.  The interpreter looks the name up in its
 * stdlib registry and injects the module's exported functions/variables into
 * the current global scope — but only the first time the module is referenced
 * (lazy loading).
 */
class UseNode : public ASTNode {
public:
    std::string module_name;
 
    UseNode(std::string module_name) : module_name(std::move(module_name)) {}
};

/**
 * Sentinel node representing the end of a statement stream
 */
class EOFNode : public ASTNode {};

/**
 * @brief Increments a variable.
 */
class IncrementDecrementNode : public ASTNode {
public:
    std::string name;
    double inc_val;
    IncrementDecrementNode(std::string name, double inc_val) : name(name), inc_val(inc_val) {}
};

#endif // AST_NODES_H