#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <unordered_map>
#include <vector>
#include <string>
#include "ast_nodes.h"

class Interpreter {
private:
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
                         std::unordered_map<std::string, PrometheusValue> global_vars = {});

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