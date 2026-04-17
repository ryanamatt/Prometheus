#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <unordered_map>
#include <string>
#include "ast_nodes.h"

class Interpreter {
private:
    const std::vector<std::unique_ptr<ASTNode>>* nodes;
    std::unordered_map<std::string, PrometheusValue> variables;
    std::unordered_map<std::string, FunctionDeclNode*> functions;

public:
    explicit Interpreter(const std::vector<std::unique_ptr<ASTNode>>& nodes,
        std::unordered_map<std::string, PrometheusValue> global_vars = {})
        : nodes(&nodes), variables(std::move(global_vars)) {}

    /**
     * @brief Iterates through a list of top-level AST nodes and executes them in order.
     */
    std::unordered_map<std::string, PrometheusValue> interpret();

    /**
     * @brief  Recursively visits an AST node and evaluates its value or executes its logic.
     */
    PrometheusValue visit(ASTNode* node);
};

#endif // INTERPRETER_H