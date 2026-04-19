#include <cmath>
#include <sstream>
#include "exceptions.h"
#include "interpreter.h"

std::unordered_map<std::string, PrometheusValue> Interpreter::interpret() {
    for (auto& node : *nodes) {
        visit(node.get());
    }
    return variables;
}

// ============================================================================
// Helpers — extract typed values from a PrometheusValue
// ============================================================================

static double get_double(const PrometheusValue& value) {
    if (auto* i = std::get_if<int>(&value))    return static_cast<double>(*i);
    if (auto* d = std::get_if<double>(&value)) return *d;
    throw std::runtime_error("Runtime Error: Numeric value expected.");
}

static int get_int(const PrometheusValue& value) {
    if (auto* i = std::get_if<int>(&value))    return *i;
    if (auto* d = std::get_if<double>(&value)) return static_cast<int>(*d);
    throw std::runtime_error("Runtime Error: Numeric value expected.");
}

static const std::string& get_string(const PrometheusValue& value) {
    if (auto* s = std::get_if<std::string>(&value)) return *s;
    throw std::runtime_error("Runtime Error: String value expected.");
}

static bool get_bool(const PrometheusValue& value) {
    if (auto* b = std::get_if<bool>(&value))   return *b;
    if (auto* i = std::get_if<int>(&value))    return *i != 0;
    if (auto* d = std::get_if<double>(&value)) return *d != 0.0;
    if (auto* s = std::get_if<std::string>(&value)) return !s->empty() && *s != "false";
    return false;
}

static std::string value_to_string(const PrometheusValue& value) {
    if (auto* i = std::get_if<int>(&value))    return std::to_string(*i);
    if (auto* d = std::get_if<double>(&value)) {
        std::string s = std::to_string(*d);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }
    if (auto* s = std::get_if<std::string>(&value)) return *s;
    if (auto* b = std::get_if<bool>(&value))        return *b ? "true" : "false";
    return "None"; // std::monostate
}

// ============================================================================
// visit
// ============================================================================

PrometheusValue Interpreter::visit(ASTNode* node) {

    if (NumberNode* n = dynamic_cast<NumberNode*>(node)) {
        if (n->value.find('.') != std::string::npos) {
            return std::stod(n->value);
        }
        return std::stoi(n->value);
    }

    else if (VarNode* n = dynamic_cast<VarNode*>(node)) {
        auto it = variables.find(n->value);
        if (it != variables.end()) {
            return it->second;
        }
        return std::monostate{};
    }

    else if (StringNode* n = dynamic_cast<StringNode*>(node)) {
        return n->value;
    }

    else if (BooleanNode* n = dynamic_cast<BooleanNode*>(node)) {
        return get_bool(n->value);
    }

    else if (BinOpNode* n = dynamic_cast<BinOpNode*>(node)) {
        PrometheusValue left_val  = visit(n->left.get());
        PrometheusValue right_val = visit(n->right.get());

        switch (n->op.get_token()) {

            // Arithmetic
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

            // Comparison
            case TokenType::EQUAL:
                if (std::holds_alternative<std::string>(left_val) &&
                    std::holds_alternative<std::string>(right_val)) {
                    return get_string(left_val) == get_string(right_val);
                }
                return get_double(left_val) == get_double(right_val);

            case TokenType::NOTEQUAL:
                if (std::holds_alternative<std::string>(left_val) &&
                    std::holds_alternative<std::string>(right_val)) {
                    return get_string(left_val) != get_string(right_val);
                }
                return get_double(left_val) != get_double(right_val);

            case TokenType::GREATER:    return get_double(left_val) >  get_double(right_val);
            case TokenType::LESSER:     return get_double(left_val) <  get_double(right_val);
            case TokenType::GREATEREQ:  return get_double(left_val) >= get_double(right_val);
            case TokenType::LESSEREQ:   return get_double(left_val) <= get_double(right_val);

            // Logical
            case TokenType::AND: return get_bool(left_val) && get_bool(right_val);
            case TokenType::OR:  return get_bool(left_val) || get_bool(right_val);

            default:
                throw std::runtime_error("Runtime Error: Operator not supported in binary expression");
        }
    }

    else if (VarDeclNode* n = dynamic_cast<VarDeclNode*>(node)) {
        PrometheusValue value = visit(n->value_node.get());
        variables[n->name] = value;
        return value;
    }

    else if (UnaryOpNode* n = dynamic_cast<UnaryOpNode*>(node)) {
        PrometheusValue right_val = visit(n->right.get());

        if (n->op.get_token() == TokenType::NOT) {
            return !get_bool(right_val);
        }
        throw std::runtime_error("Runtime Error: Unsupported unary operator");
    }

    else if (IncrementDecrementNode* n = dynamic_cast<IncrementDecrementNode*>(node)) {
        auto it = variables.find(n->name);
        if (it == variables.end()) {
            throw std::runtime_error("Runtime Error: Undefined variable '" + n->name + "'");
        }

        double current_val = get_double(it->second);
        double new_val = current_val + n->inc_val;

        // Preserve the original type (int stays int, double stays double)
        if (std::holds_alternative<int>(it->second)) {
            variables[n->name] = static_cast<int>(new_val);
        } else {
            variables[n->name] = new_val;
        }
        return variables[n->name];
    }

    else if (PrintNode* n = dynamic_cast<PrintNode*>(node)) {
        std::vector<std::string> results;
        for (const auto& expr : n->expressions) {
            results.push_back(value_to_string(visit(expr.get())));
        }

        std::stringstream ss;
        for (size_t i = 0; i < results.size(); ++i) {
            ss << results[i];
            if (i < results.size() - 1) ss << " ";
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
        if (get_bool(visit(n->condition.get()))) {
            for (auto& stmt : n->then_branch)
                visit(stmt.get());
            return std::monostate{};
        }

        for (auto& [condition, branch] : n->elif_branches) {
            if (get_bool(visit(condition.get()))) {
                for (auto& stmt : branch)
                    visit(stmt.get());
                return std::monostate{};
            }
        }

        for (auto& stmt : n->else_branch)
            visit(stmt.get());
        return std::monostate{};
    }

    else if (WhileNode* n = dynamic_cast<WhileNode*>(node)) {
        while (get_bool(visit(n->condition.get()))) {
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
        }
        return std::monostate{};
    }

    else if (ForNode* n = dynamic_cast<ForNode*>(node)) {
        visit(n->variable.get());
        while (get_bool(visit(n->condition.get()))) {
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
            visit(n->change_var.get());
        }
        return std::monostate{};
    }

    else if (FunctionDeclNode* n = dynamic_cast<FunctionDeclNode*>(node)) {
        functions[n->name] = n;
        return std::monostate{};
    }

    else if (ReturnNode* n = dynamic_cast<ReturnNode*>(node)) {
        PrometheusValue value = visit(n->value_node.get());
        throw ReturnException{value};
    }

    else if (CallNode* n = dynamic_cast<CallNode*>(node)) {
        // Built-in type conversions
        if (n->name == "int") {
            PrometheusValue val = visit(n->args[0].get());
            return std::stoi(get_string(val));
        }
        if (n->name == "double") {
            PrometheusValue val = visit(n->args[0].get());
            return std::stod(get_string(val));
        }
        if (n->name == "str") {
            PrometheusValue val = visit(n->args[0].get());
            return std::to_string(get_double(val));
        }

        if (n->name == "bool") {
            PrometheusValue val = visit(n->args[0].get());
            return get_bool(val);
        }

        if (functions.find(n->name) == functions.end()) {
            throw std::runtime_error("Runtime Error: Function '" + n->name + "' not defined.");
        }

        FunctionDeclNode* func_node = functions[n->name];

        std::vector<PrometheusValue> arg_values;
        for (auto& arg : n->args)
            arg_values.push_back(visit(arg.get()));

        Interpreter local_interpreter(func_node->body, variables);
        local_interpreter.functions = functions;

        for (size_t i = 0; i < arg_values.size() && i < func_node->params.size(); i++) {
            local_interpreter.variables[func_node->params[i].second] = arg_values[i];
        }

        try {
            for (auto& stmt : func_node->body) {
                local_interpreter.visit(stmt.get());
            }
        } catch (const ReturnException& e) {
            return e.value;
        }
    }

    return std::monostate{};
}