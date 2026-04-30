#ifndef AST_NODES_H
#define AST_NODES_H

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "prometheus_types.h"
#include "visitor.h"

/**
 * @brief Base class for all nodes in the Abstract Syntax Tree.
 *
 * Every concrete subclass must implement accept() by calling the
 * correctly-typed visitor overload for itself.  This gives O(1)
 * dispatch without any dynamic_cast in the interpreter hot-path.
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;

    /** Double-dispatch entry point. */
    virtual PrometheusValue accept(Visitor& v) = 0;
};

// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------

class NumberNode : public ASTNode {
public:
    Token token;
    std::string value;

    explicit NumberNode(Token token) : token(token), value(token.get_value()) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

class StringNode : public ASTNode {
public:
    Token token;
    std::string value;

    explicit StringNode(Token token) : token(token), value(token.get_value()) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

class BooleanNode : public ASTNode {
public:
    Token token;
    std::string value;

    explicit BooleanNode(Token token) : token(token), value(token.get_value()) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Variables
// ---------------------------------------------------------------------------

/** Variable identifier read (e.g. the `x` in `x + 1`). */
class VarNode : public ASTNode {
public:
    Token token;
    std::string value;

    explicit VarNode(Token token) : token(token), value(token.get_value()) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Operators
// ---------------------------------------------------------------------------

/** Binary operation: left OP right */
class BinOpNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    Token op;
    std::unique_ptr<ASTNode> right;

    BinOpNode(std::unique_ptr<ASTNode> left, Token op, std::unique_ptr<ASTNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** Unary operation: OP right  (e.g. `!flag`, `-x`) */
class UnaryOpNode : public ASTNode {
public:
    Token op;
    std::unique_ptr<ASTNode> right;

    UnaryOpNode(Token op, std::unique_ptr<ASTNode> right)
        : op(op), right(std::move(right)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Declarations & assignment
// ---------------------------------------------------------------------------

/** Typed variable declaration or bare re-assignment: `int x = 5;` / `x = 5;` */
class VarDeclNode : public ASTNode {
public:
    std::string var_type;
    std::string name;
    std::unique_ptr<ASTNode> value_node;

    VarDeclNode(std::string var_type, std::string name, std::unique_ptr<ASTNode> value_node)
        : var_type(std::move(var_type)), name(std::move(name)),
          value_node(std::move(value_node)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `x++` / `x--` / compound increment by arbitrary delta */
class IncrementDecrementNode : public ASTNode {
public:
    std::string name;
    double inc_val;   // +1.0 for ++, -1.0 for --

    IncrementDecrementNode(std::string name, double inc_val)
        : name(std::move(name)), inc_val(inc_val) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Built-in statements
// ---------------------------------------------------------------------------

/** `print(expr, ...);` */
class PrintNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> expressions;

    explicit PrintNode(std::vector<std::unique_ptr<ASTNode>> expressions)
        : expressions(std::move(expressions)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `input("prompt")` */
class InputNode : public ASTNode {
public:
    std::string msg;

    explicit InputNode(std::string msg) : msg(std::move(msg)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/**
 * @brief `range(stop)` / `range(start, stop)` / `range(start, stop, step)`
 *
 * Evaluates to a list[int].
 */
class RangeNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> start;
    std::unique_ptr<ASTNode> stop;
    std::unique_ptr<ASTNode> step;

    RangeNode(std::unique_ptr<ASTNode> start,
              std::unique_ptr<ASTNode> stop,
              std::unique_ptr<ASTNode> step)
        : start(std::move(start)), stop(std::move(stop)), step(std::move(step)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Control flow
// ---------------------------------------------------------------------------

/** `if (cond) { } elif (cond) { } else { }` */
class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    std::vector<std::pair<std::unique_ptr<ASTNode>,
                          std::vector<std::unique_ptr<ASTNode>>>> elif_branches;
    std::vector<std::unique_ptr<ASTNode>> else_branch;

    IfNode(std::unique_ptr<ASTNode> condition,
           std::vector<std::unique_ptr<ASTNode>> then_branch,
           std::vector<std::pair<std::unique_ptr<ASTNode>,
                                 std::vector<std::unique_ptr<ASTNode>>>> elif_branches,
           std::vector<std::unique_ptr<ASTNode>> else_branch)
        : condition(std::move(condition)), then_branch(std::move(then_branch)),
          elif_branches(std::move(elif_branches)), else_branch(std::move(else_branch)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `while (cond) { }` */
class WhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> do_branch;

    WhileNode(std::unique_ptr<ASTNode> condition,
              std::vector<std::unique_ptr<ASTNode>> do_branch)
        : condition(std::move(condition)), do_branch(std::move(do_branch)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** C-style `for (init; cond; step;) { }` */
class ForNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> variable;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> change_var;
    std::vector<std::unique_ptr<ASTNode>> do_branch;

    ForNode(std::unique_ptr<ASTNode> variable,
            std::unique_ptr<ASTNode> condition,
            std::unique_ptr<ASTNode> change_var,
            std::vector<std::unique_ptr<ASTNode>> do_branch)
        : variable(std::move(variable)), condition(std::move(condition)),
          change_var(std::move(change_var)), do_branch(std::move(do_branch)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** Range-based `for (type name : list_expr) { }` */
class ForInNode : public ASTNode {
public:
    std::string var_type;
    std::string var_name;
    std::unique_ptr<ASTNode> list_expr;
    std::vector<std::unique_ptr<ASTNode>> body;

    ForInNode(std::string type, std::string name,
              std::unique_ptr<ASTNode> list,
              std::vector<std::unique_ptr<ASTNode>> body)
        : var_type(std::move(type)), var_name(std::move(name)),
          list_expr(std::move(list)), body(std::move(body)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Functions
// ---------------------------------------------------------------------------

struct Parameter {
    std::string type;
    std::string name;
    std::unique_ptr<ASTNode> default_val;   // nullptr → no default

    Parameter(std::string t, std::string n, std::unique_ptr<ASTNode> dv)
        : type(std::move(t)), name(std::move(n)), default_val(std::move(dv)) {}
};

/** `func <return_type> <name>(<params>) { }` */
class FunctionDeclNode : public ASTNode {
public:
    std::string name;
    std::string return_type;
    std::vector<Parameter> params;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDeclNode(std::string name, std::string return_type,
                     std::vector<Parameter> params,
                     std::vector<std::unique_ptr<ASTNode>> body)
        : name(std::move(name)), return_type(std::move(return_type)),
          params(std::move(params)), body(std::move(body)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `return <expr>;`  (value_node is nullptr for bare `return;`) */
class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value_node;

    explicit ReturnNode(std::unique_ptr<ASTNode> value_node)
        : value_node(std::move(value_node)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `name(arg, ...)` function call expression */
class CallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    CallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : name(std::move(name)), args(std::move(args)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Lists
// ---------------------------------------------------------------------------

/** `[expr, expr, …]` list literal */
class ListLiteralNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;

    explicit ListLiteralNode(std::vector<std::unique_ptr<ASTNode>> elements)
        : elements(std::move(elements)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `list[type] name = expr;` */
class ListDeclNode : public ASTNode {
public:
    std::string element_type; // "int", "double", "str", "bool"
    std::string name;
    std::unique_ptr<ASTNode> value_node;

    ListDeclNode(std::string element_type, std::string name,
                 std::unique_ptr<ASTNode> value_node)
        : element_type(std::move(element_type)), name(std::move(name)),
          value_node(std::move(value_node)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `name[index]` index read */
class ListIndexNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> index;

    ListIndexNode(std::string name, std::unique_ptr<ASTNode> index)
        : name(std::move(name)), index(std::move(index)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `name[index] = value;` index assignment */
class ListAssignNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> index;
    std::unique_ptr<ASTNode> value;

    ListAssignNode(std::string name, std::unique_ptr<ASTNode> index,
                   std::unique_ptr<ASTNode> value)
        : name(std::move(name)), index(std::move(index)), value(std::move(value)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `name.append(expr)` */
class ListAppendNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> value;

    ListAppendNode(std::string name, std::unique_ptr<ASTNode> value)
        : name(std::move(name)), value(std::move(value)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/** `name.len()` — evaluates to int */
class ListLengthNode : public ASTNode {
public:
    std::string name;

    explicit ListLengthNode(std::string name) : name(std::move(name)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Modules
// ---------------------------------------------------------------------------

/**
 * @brief `import path/to/file;`
 *
 * Resolves relative to the importing file's directory (or CWD in the REPL).
 * base_dir is stamped in by the interpreter before execution.
 */
class ImportNode : public ASTNode {
public:
    std::string path;
    std::string base_dir;

    explicit ImportNode(std::string path, std::string base_dir = "")
        : path(std::move(path)), base_dir(std::move(base_dir)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

/**
 * @brief `use module_name;`
 *
 * Names a standard-library module.  The interpreter looks up the name in its
 * stdlib registry and injects the module's exports into the current global
 * scope (lazy, at most once per session).
 */
class UseNode : public ASTNode {
public:
    std::string module_name;

    explicit UseNode(std::string module_name) : module_name(std::move(module_name)) {}

    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

// ---------------------------------------------------------------------------
// Sentinel
// ---------------------------------------------------------------------------

/** Sentinel node representing the end of a statement stream. */
class EOFNode : public ASTNode {
public:
    PrometheusValue accept(Visitor& v) override { return v.visit(this); }
};

#endif // AST_NODES_H