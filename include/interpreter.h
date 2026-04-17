#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <unordered_map>
#include <string>
#include <any>
#include "ast_nodes.h"

class Interpreter {
private:
    const std::vector<std::unique_ptr<ASTNode>>* nodes;
    std::unordered_map<std::string, std::any> variables;
    std::unordered_map<std::string, FunctionDeclNode*> functions;

public:
    explicit Interpreter(const std::vector<std::unique_ptr<ASTNode>>& nodes, 
        std::unordered_map<std::string, std::any> global_vars = {}) : nodes(&nodes), variables(std::move(global_vars)) {}

    /**
     * @brief Iterates through a list of top-level AST nodes and executes them in order.
     */
    std::any interpret();

    /**
     * @brief  Recursively visits an AST node and evaluates its value or executes its logic.
     */
    std::any visit(ASTNode* node);
};

#endif // INTERPRETER_H