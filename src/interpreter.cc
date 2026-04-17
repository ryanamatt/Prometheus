#include <cmath>
#include <stdbool.h>
#include <sstream>
#include <any>
#include "exceptions.h"
#include "interpreter.h"

std::unordered_map<std::string, std::any> Interpreter::interpret() {
    for (auto& node : *nodes) {
        visit(node.get());
    }
    return variables;
}

double get_double(std::any value) {
    if (value.type() == typeid(int)) return static_cast<double>(std::any_cast<int>(value));
    if (value.type() == typeid(double)) return std::any_cast<double>(value);
    throw std::runtime_error("Runtime Error: Numeric value expected.");
}

int get_int(std::any value) {
    if (value.type() == typeid(int)) return std::any_cast<int>(value);
    if (value.type() == typeid(double)) return static_cast<int>(std::any_cast<double>(value));
    throw std::runtime_error("Runtime Error: Numeric value expected.");
}

std::string get_string(std::any value) {
    return std::any_cast<std::string>(value);
}

bool get_bool(std::any value) {
    if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value);
    }
    if (value.type() == typeid(int)) {
        return std::any_cast<int>(value) != 0;
    }
    if (value.type() == typeid(double)) {
        return std::any_cast<double>(value) != 0.0;
    }
    return false; // Or throw a specific runtime error
}

std::string any_to_string(const std::any& value) {
    if (value.type() == typeid(int)) return std::to_string(std::any_cast<int>(value));
    if (value.type() == typeid(double)) {
        std::string s = std::to_string(std::any_cast<double>(value));
        // Optional: Remove trailing zeros for a cleaner look like Python
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }
    if (value.type() == typeid(std::string)) return std::any_cast<std::string>(value);
    if (value.type() == typeid(bool)) return std::any_cast<bool>(value) ? "True" : "False";
    
    return "None";
}

std::any Interpreter::visit(ASTNode* node) {

    if (NumberNode* n = dynamic_cast<NumberNode*>(node)) {
        if (n->value.find('.') != std::string::npos) {
            return std::stod(n->value);
        }
        return std::stoi(n->value);
    }

    else if (VarNode* n = dynamic_cast<VarNode*>(node)) {
        auto it = variables.find(n->value);
        if (it != variables.end()) {
            return variables[n->value];
        }
    }

    else if (StringNode* n = dynamic_cast<StringNode*>(node))  {
        return n->value;
    }

    else if (BinOpNode* n = dynamic_cast<BinOpNode*>(node)) {
        std::any left_val = visit(n->left.get());
        std::any right_val = visit(n->right.get());

        switch (n->op.get_token()) {

            // Math Operators

            case TokenType::PLUS:
                return get_double(left_val) + get_double(right_val);

            case TokenType::MINUS:
                return get_double(left_val) - get_double(right_val);

            case TokenType::MULTIPLY:
                return get_double(left_val) * get_double(right_val);

            case TokenType::DIVIDE:
                return get_double(left_val) / get_double(right_val);

            case TokenType::MODULO:
                return get_int(left_val) % get_int(right_val);

            case TokenType::EXPONENT:
                return std::pow(get_double(left_val), get_double(right_val));

            // Comparison Operators
            
            case TokenType::EQUAL:
                if (left_val.type() == typeid(std::string) && right_val.type() == typeid(std::string)) {
                    return get_string(left_val) == get_string(right_val);
                }
                return get_double(left_val) == get_double(right_val);

            case TokenType::NOTEQUAL:
                if (left_val.type() == typeid(std::string) && right_val.type() == typeid(std::string)) {
                    return get_string(left_val) != get_string(right_val);
                }
                return get_double(left_val) != get_double(right_val);

            case TokenType::GREATER:
                return get_double(left_val) > get_double(right_val);

            case TokenType::LESSER:
                return get_double(left_val) < get_double(right_val);

            case TokenType::GREATEREQ:
                return get_double(left_val) >= get_double(right_val);

            case TokenType::LESSEREQ:
                return get_double(left_val) <= get_double(right_val);

            case TokenType::AND:
                return get_bool(left_val) && get_bool(right_val);

            case TokenType::OR:
                return get_bool(left_val) || get_bool(right_val);

            default:
                throw std::runtime_error("Runtime Error: Operator not supported in binary expression");
        }
    }

    else if (VarDeclNode* n = dynamic_cast<VarDeclNode*>(node)) {
        std::any value = visit(n->value_node.get());
        variables[n->name] = value;
        return value;
    }

    else if (IncrementDecrementNode* n = dynamic_cast<IncrementDecrementNode*>(node)) {
        auto it = variables.find(n->name);
        if (it == variables.end()) {
            throw std::runtime_error("Runtime Error: Undefined variable '" + n->name + "'");
        }

        double current_val = get_double(it->second);
        double new_val = current_val + n->inc_val;
        if (it->second.type() == typeid(int)) {
            variables[n->name] = static_cast<int>(new_val);
        } else {
            variables[n->name] = new_val;
        }
        return variables[n->name];
    }

    else if (PrintNode* n = dynamic_cast<PrintNode*>(node)) {
        std::vector<std::string> results;
        for (const auto& expr : n->expressions) {
            std::any val = visit(expr.get());
            results.push_back(any_to_string(val)); 
        }

        std::stringstream ss;
        for (size_t i = 0; i < results.size(); ++i) {
            ss << results[i];
            if (i < (results.size() - 1)) {
                ss << " ";
            }
        }

        std::string output = ss.str();
        std::cout << output << std::endl;
        return output;
    }

    else if (InputNode* n = dynamic_cast<InputNode*>(node)) {
        std::cout << n->msg;
        std::string user_input;
        std::getline(std::cin, user_input);
        return user_input;
    }

    else if (IfNode* n = dynamic_cast<IfNode*>(node)) {
        // Check IF
        bool condition_met = get_bool(visit(n->condition.get()));
        if (condition_met) {
            for (auto& stmt : n->then_branch)
                visit(stmt.get());
            return 0;
        }

        // Check ELIF(s)
        for (auto& [condition, branch] : n->elif_branches) {
            if (get_bool(visit(condition.get()))) {
                for (auto& stmt : branch)
                    visit(stmt.get());
                return 0;
            }
        }

        // Check ELSE
        for (auto& stmt : n->else_branch)
            visit(stmt.get());
    }

    else if (WhileNode* n = dynamic_cast<WhileNode*>(node)) {
        while (get_bool(visit(n->condition.get()))) {
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
        }
        return 0;
    }

    else if (ForNode* n = dynamic_cast<ForNode*>(node)) {
        visit(n->variable.get());
        while (get_bool(visit(n->condition.get()))) {
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
            visit(n->change_var.get());
        }
        return 0;
    }

    else if (FunctionDeclNode* n = dynamic_cast<FunctionDeclNode*>(node)) {
        functions[n->name] = n;
        return 0;
    }

    else if (ReturnNode* n = dynamic_cast<ReturnNode*>(node)) {
        std::any value = visit(n->value_node.get());
        throw ReturnException{value};
    }

    else if (CallNode* n = dynamic_cast<CallNode*>(node)) {
        // Handle Built-in Conversions first
        if (n->name == "int") {
            std::any val = visit(n->args[0].get());
            return std::stoi(get_string(val)); 
        }

        else if (n->name == "double") {
            std::any val = visit(n->args[0].get());
            return std::stod(get_string(val));
        }

        else if  (n->name == "str") {
            std::any val = visit(n->args[0].get());
            return std::to_string(get_double(val));
        }


        if (functions.find(n->name) == functions.end()) {
            throw std::runtime_error("Runtime Error: Function '" + n->name + "' not defined.");
        }

        FunctionDeclNode* func_node = functions[n->name];
        std::vector<std::any> arg_values;
        for (auto& arg : n->args)
            arg_values.push_back(visit(arg.get()));

        Interpreter local_interpreter(func_node->body, variables);
        local_interpreter.functions = functions;

        for (size_t i = 0; i < arg_values.size() && i < func_node->params.size(); i++) {
            auto& p_name = func_node->params[i].second;
            local_interpreter.variables[p_name] = arg_values[i];
        }

        try {
            for (auto& stmt : func_node->body) {
                local_interpreter.visit(stmt.get());
            }
        } catch (const ReturnException& e) {
            return e.value; // Return the caught value
        }


    }

    return {};
}