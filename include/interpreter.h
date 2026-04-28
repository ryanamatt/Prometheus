#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include "ast_nodes.h"

class Interpreter {
private:
    using NativeFunction = std::function<PrometheusValue(std::vector<PrometheusValue>, int)>;

    std::unordered_map<std::string, NativeFunction> native_functions;

    /**
     * @brief Private helper function to register C++ math functions in Promethus
     */
    void register_math_functions();

    const std::vector<std::unique_ptr<ASTNode>>* nodes;

    /**
     * @brief A stack of variable scopes. back() is always the innermost
     *        (most local) scope. front() is the global scope.
     *
     * Layout:
     *   scope_stack[0]  — global scope  (pushed once at construction)
     *   scope_stack[1]  — e.g. a function body
     *   scope_stack[2]  — e.g. an if-block inside that function
     *   ...
     */
    std::vector<std::unordered_map<std::string, PrometheusValue>> scope_stack;

    /** Function table — functions are always global. */
    std::unordered_map<std::string, FunctionDeclNode*> functions;

    // ------------------------------------------------------------------
    // Import / use support
    // ------------------------------------------------------------------

    /**
     * @brief Directory of the currently-executing source file.
     *        Used to resolve relative `import` paths.
     *        Empty string means CWD (REPL mode or no file given).
     */
    std::string base_dir;

    /**
     * @brief Set of canonical absolute paths that have already been imported
     *        in this interpreter session.  Prevents circular / duplicate imports.
     */
    std::unordered_set<std::string> imported_files;

    /**
     * @brief Set of stdlib module names that have already been loaded.
     *        Ensures each module is injected into scope at most once.
     */
    std::unordered_set<std::string> loaded_modules;

    /**
     * @brief Registry of known stdlib modules.
     *        Maps module_name → path to the .prm file inside the stdlib directory.
     *
     * Populated lazily on first access to keep startup cost near zero.
     */
    std::unordered_map<std::string, std::string> stdlib_registry;

    /**
     * @brief Owns the AST trees of all imported files.
     *        FunctionDeclNode* pointers in the `functions` map point into
     *        these trees, so the trees must outlive the interpreter.
     */
    std::vector<std::vector<std::unique_ptr<ASTNode>>> owned_trees;

    /**
     * @brief Resolve and initialise stdlib_registry if it hasn't been yet.
     *        Searches for the stdlib directory in these locations in order:
     *          1. $PROMETHEUS_STDLIB  (environment variable)
     *          2. <base_dir>/stdlib
     *          3. <executable_dir>/stdlib   (not easily available; skipped)
     *          4. ./stdlib
     */
    void init_stdlib_registry();

    /**
     * @brief Execute an `import path;` node.
     *        Resolves the path, guards against re-import, then lexes, parses,
     *        and interprets the file in the current scope.
     */
    void exec_import(ImportNode* node);

    /**
     * @brief Execute a `use module_name;` node.
     *        Looks up the module in stdlib_registry, then delegates to exec_import.
     *        No-ops if the module is already loaded.
     */
    void exec_use(UseNode* node);

    // ------------------------------------------------------------------
    // Scope helpers
    // ------------------------------------------------------------------

    /** Push a fresh, empty scope onto the stack. */
    void push_scope();

    /** Pop the innermost scope, discarding all locals declared in it. */
    void pop_scope();

    /**
     * @brief Walk the scope stack from innermost to outermost and return
     *        the value bound to @p name.
     * @throws UndefinedVariableException if the name is not in any scope.
     */
    PrometheusValue get_var(const std::string& name, int line = 0) const;

    /**
     * @brief Assign @p value to @p name in the innermost scope that already
     *        has a binding for it.  If no existing binding is found, create
     *        a new one in the current (innermost) scope.
     *
     * Use this for bare re-assignments: `x = 5;`
     */
    void set_var(const std::string& name, PrometheusValue value);

    /**
     * @brief Always create a new binding in the current (innermost) scope.
     *        Use this for typed declarations: `int x = 5;`
     *        This correctly shadows an outer variable of the same name.
     */
    void declare_var(const std::string& name, PrometheusValue value);

    /** Returns true if @p name is bound in any visible scope. */
    bool has_var(const std::string& name) const;

public:
    explicit Interpreter(const std::vector<std::unique_ptr<ASTNode>>& nodes,
                         std::unordered_map<std::string, PrometheusValue> global_vars = {},
                         std::string base_dir = "");

    /**
     * @brief Iterates through the top-level AST nodes and executes them.
     * @return The final global scope (used by main for DEBUG output).
     */
    std::unordered_map<std::string, PrometheusValue> interpret();

    /**
     * @brief Recursively visits an AST node, evaluating or executing it.
     */
    PrometheusValue visit(ASTNode* node);
};

#endif // INTERPRETER_H