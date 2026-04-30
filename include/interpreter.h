#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include "ast_nodes.h"
#include "exceptions.h"
#include "visitor.h"

/**
 * @brief Tree-walking interpreter for Prometheus.
 *
 * Derives from Visitor so that dispatch from ASTNode::accept() resolves
 * to the correct typed overload at compile time — no dynamic_cast anywhere
 * in the evaluation hot-path.
 *
 * The public interface is unchanged: construct with a node list and call
 * interpret().  Internally, visit(ASTNode*) is now a one-liner that
 * delegates to node->accept(*this).
 */
class Interpreter : public Visitor {
private:
    using NativeFunction =
        std::function<PrometheusValue(std::vector<PrometheusValue>, int)>;

    // ------------------------------------------------------------------
    // State
    // ------------------------------------------------------------------

    const std::vector<std::unique_ptr<ASTNode>>* nodes;

    /**
     * Scope stack — back() is always the innermost (most local) scope.
     *   [0] global scope   (pushed once at construction)
     *   [1] function body
     *   [2] if-block inside that function
     *   …
     */
    std::vector<std::unordered_map<std::string, PrometheusValue>> scope_stack;

    /** Function table — functions are always global. */
    std::unordered_map<std::string, FunctionDeclNode*> functions;

    /** Native (C++) function table populated by register_math_functions(). */
    std::unordered_map<std::string, NativeFunction> native_functions;

    /** */
    std::mt19937 generator; 

    //State Maintanece
    
    int last_seed = 0;

    // ------------------------------------------------------------------
    // Import / use support
    // ------------------------------------------------------------------

    /** Directory of the currently-executing source file (empty = CWD / REPL). */
    std::string base_dir;

    /** Canonical paths already imported — prevents circular / duplicate imports. */
    std::unordered_set<std::string> imported_files;

    /** Stdlib module names already loaded — ensures at-most-once injection. */
    std::unordered_set<std::string> loaded_modules;

    /** Maps module_name → absolute path to the .prm file. Populated lazily. */
    std::unordered_map<std::string, std::string> stdlib_registry;

    /**
     * Owns the AST trees of all imported files.  FunctionDeclNode* entries
     * in `functions` point into these trees, so the trees must outlive the
     * interpreter.
     */
    std::vector<std::vector<std::unique_ptr<ASTNode>>> owned_trees;

    // ------------------------------------------------------------------
    // Private helpers
    // ------------------------------------------------------------------

    /** Register C++ math functions as native callables. */
    void register_math_functions();

    /** Register C++ random functions as native callables. */
    void register_random_functions();

    /**
     * @brief Resolve and initialise stdlib_registry if it hasn't been yet.
     *        Searches for the stdlib directory in these locations in order:
     *          1. $PROMETHEUS_STDLIB  (environment variable)
     *          2. <base_dir>/stdlib
     *          3. <executable_dir>/stdlib   (not easily available; skipped)
     *          4. ./stdlib
     */
    void init_stdlib_registry();

    /** Execute an `import path;` node. */
    void exec_import(ImportNode* node);

    /** Execute a `use module_name;` node. */
    void exec_use(UseNode* node);

    // Scope management
    void push_scope();
    void pop_scope();

    PrometheusValue get_var(const std::string& name, int line = 0) const;
    void            set_var(const std::string& name, PrometheusValue value);
    void            declare_var(const std::string& name, PrometheusValue value);
    bool            has_var(const std::string& name) const;

public:
    explicit Interpreter(
        const std::vector<std::unique_ptr<ASTNode>>& nodes,
        std::unordered_map<std::string, PrometheusValue> global_vars = {},
        std::string base_dir = "");

    /**
     * @brief Execute all top-level nodes and return the final global scope.
     */
    std::unordered_map<std::string, PrometheusValue> interpret();

    /**
     * @brief Dispatch a node through the visitor.
     *
     * This is now a one-liner: `return node->accept(*this);`
     * It replaces the old chain of dynamic_cast checks.
     */
    PrometheusValue visit(ASTNode* node);

    // ------------------------------------------------------------------
    // Visitor overloads — one per concrete node type.
    // Implemented in src/interpreter.cc.
    // ------------------------------------------------------------------

    PrometheusValue visit(NumberNode* node) override;
    PrometheusValue visit(StringNode* node) override;
    PrometheusValue visit(BooleanNode* node) override;

    PrometheusValue visit(VarNode* node) override;
    PrometheusValue visit(BinOpNode* node) override;
    PrometheusValue visit(UnaryOpNode* node) override;
    PrometheusValue visit(VarDeclNode* node) override;
    PrometheusValue visit(IncrementDecrementNode* node) override;

    PrometheusValue visit(PrintNode* node) override;
    PrometheusValue visit(InputNode* node) override;
    PrometheusValue visit(RangeNode* node) override;

    PrometheusValue visit(IfNode* node) override;
    PrometheusValue visit(WhileNode* node) override;
    PrometheusValue visit(ForNode* node) override;
    PrometheusValue visit(ForInNode* node) override;

    PrometheusValue visit(FunctionDeclNode* node) override;
    PrometheusValue visit(ReturnNode* node) override;
    PrometheusValue visit(CallNode* node) override;

    PrometheusValue visit(ListLiteralNode* node) override;
    PrometheusValue visit(ListDeclNode* node) override;
    PrometheusValue visit(ListIndexNode* node) override;
    PrometheusValue visit(ListAssignNode* node) override;
    PrometheusValue visit(ListAppendNode* node) override;
    PrometheusValue visit(ListLengthNode* node) override;

    PrometheusValue visit(ImportNode* node) override;
    PrometheusValue visit(UseNode* node) override;

    PrometheusValue visit(EOFNode* node) override;
};

#endif // INTERPRETER_H