#include <cmath>
#include <sstream>
#include "exceptions.h"
#include "interpreter.h"

Interpreter::Interpreter(const std::vector<std::unique_ptr<ASTNode>>& nodes,
                         std::unordered_map<std::string, PrometheusValue> global_vars)
    : nodes(&nodes)
{
    // The global scope is always present at index 0.
    scope_stack.push_back(std::move(global_vars));
}


void Interpreter::push_scope() {
    scope_stack.push_back({});
}

void Interpreter::pop_scope() {
    if (scope_stack.size() > 1)   // never pop the global scope
        scope_stack.pop_back();
}

PrometheusValue Interpreter::get_var(const std::string& name, int line) const {
    // Walk from innermost scope outward
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end())
            return found->second;
    }
    throw UndefinedVariableException(name, line);
}

void Interpreter::set_var(const std::string& name, PrometheusValue value) {
    // Update the innermost scope that already has a binding for this name
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second = std::move(value);
            return;
        }
    }
    // No existing binding — create one in the current (innermost) scope
    scope_stack.back()[name] = std::move(value);
}

void Interpreter::declare_var(const std::string& name, PrometheusValue value) {
    // Always create a fresh binding in the current scope, even if an outer
    // scope already has one (shadowing).
    scope_stack.back()[name] = std::move(value);
}

bool Interpreter::has_var(const std::string& name) const {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        if (it->count(name)) return true;
    }
    return false;
}

// ============================================================================
// interpret
// ============================================================================

std::unordered_map<std::string, PrometheusValue> Interpreter::interpret() {
    for (auto& node : *nodes) {
        visit(node.get());
    }
    return scope_stack.front();   // return the global scope
}

// ============================================================================
// Helpers — extract typed values from a PrometheusValue
// ============================================================================

/**
 * @brief Returns the name of the active alternative in a PrometheusValue.
 *        Used for user-facing type mismatch messages.
 */
static std::string type_name(const PrometheusValue& v) {
    if (std::holds_alternative<int>(v))               return "int";
    if (std::holds_alternative<double>(v))            return "double";
    if (std::holds_alternative<bool>(v))              return "bool";
    if (std::holds_alternative<std::string>(v))       return "str";
    if (std::holds_alternative<PrometheusListPtr>(v)) {
        const auto& lst = std::get<PrometheusListPtr>(v);
        return "list[" + (lst ? lst->element_type : "?") + "]";
    }
    return "None";
}
 
static double get_double(const PrometheusValue& value, int line = 0) {
    if (auto* i = std::get_if<int>(&value))    return static_cast<double>(*i);
    if (auto* d = std::get_if<double>(&value)) return *d;
    throw TypeException(
        "Expected a numeric value but got '" + type_name(value) + "'", line);
}
 
static int get_int(const PrometheusValue& value, int line = 0) {
    if (auto* i = std::get_if<int>(&value))    return *i;
    if (auto* d = std::get_if<double>(&value)) return static_cast<int>(*d);
    throw TypeException(
        "Expected an integer value but got '" + type_name(value) + "'", line);
}
 
static const std::string& get_string(const PrometheusValue& value, int line = 0) {
    if (auto* s = std::get_if<std::string>(&value)) return *s;
    throw TypeException(
        "Expected a string value but got '" + type_name(value) + "'", line);
}
 
static bool get_bool(const PrometheusValue& value, int /*line*/ = 0) {
    if (auto* b = std::get_if<bool>(&value))        return *b;
    if (auto* i = std::get_if<int>(&value))         return *i != 0;
    if (auto* d = std::get_if<double>(&value))      return *d != 0.0;
    if (auto* s = std::get_if<std::string>(&value)) return !s->empty() && *s != "false";
    return false; // monostate / None
}
 
// Overload used in places without a line number (e.g. BooleanNode literal)
static bool get_bool(const std::string& value) {
    return value == "true";
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
    if (auto* lp = std::get_if<PrometheusListPtr>(&value)) {
        if (!*lp) return "[]";
        std::string out = "[";
        const auto& elems = (*lp)->elements;
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) out += ", ";
            // Wrap strings in quotes for display
            if (std::holds_alternative<std::string>(elems[i]))
                out += "\"" + std::get<std::string>(elems[i]) + "\"";
            else
                out += value_to_string(elems[i]);
        }
        out += "]";
        return out;
    }
    return "None"; // std::monostate
}
 
/**
 * @brief Validates that the runtime value is compatible with the declared type,
 *        and coerces it when the language rules allow (e.g. int → double).
 *
 * Throws TypeMismatchException on a hard mismatch.
 */
static PrometheusValue coerce_to_declared(const std::string& decl_type,
                                           const std::string& var_name,
                                           const PrometheusValue& value,
                                           int line = 0) {
    if (decl_type == "int") {
        if (std::holds_alternative<int>(value))    return value;
        if (std::holds_alternative<double>(value)) return static_cast<int>(std::get<double>(value));
        if (std::holds_alternative<bool>(value))   return static_cast<int>(std::get<bool>(value));
        throw TypeMismatchException(var_name, decl_type, type_name(value), line);
    }
    if (decl_type == "double") {
        if (std::holds_alternative<double>(value)) return value;
        if (std::holds_alternative<int>(value))    return static_cast<double>(std::get<int>(value));
        if (std::holds_alternative<bool>(value))   return static_cast<double>(std::get<bool>(value));
        throw TypeMismatchException(var_name, decl_type, type_name(value), line);
    }
    if (decl_type == "str") {
        if (std::holds_alternative<std::string>(value)) return value;
        throw TypeMismatchException(var_name, decl_type, type_name(value), line);
    }
    if (decl_type == "bool") {
        if (std::holds_alternative<bool>(value)) return value;
        if (std::holds_alternative<int>(value))  return std::get<int>(value) != 0;
        throw TypeMismatchException(var_name, decl_type, type_name(value), line);
    }
    // Unknown / user-defined type → pass through without coercion
    return value;
}

/**
 * @brief Coerces a value to fit a list's element type, or throws TypeMismatchException.
 */
static PrometheusValue coerce_to_element(const std::string& elem_type,
                                          const PrometheusValue& value,
                                          int line = 0) {
    // Re-use the existing scalar coercion with a synthetic var name for the message.
    return coerce_to_declared(elem_type, "<list element>", value, line);
}

// ============================================================================
// visit
// ============================================================================

PrometheusValue Interpreter::visit(ASTNode* node) {

    // ------------------------------------------------------------------
    // Literals
    // ------------------------------------------------------------------

    if (NumberNode* n = dynamic_cast<NumberNode*>(node)) {
        if (n->value.find('.') != std::string::npos) {
            try { return std::stod(n->value); }
            catch (...) {
                throw RuntimeException("Malformed numeric literal '" + n->value + "'",
                                       n->token.get_line());
            }
        }
        try { return std::stoi(n->value); }
        catch (...) {
            throw RuntimeException("Integer literal out of range '" + n->value + "'",
                                   n->token.get_line());
        }
    }

    else if (StringNode* n = dynamic_cast<StringNode*>(node)) {
        return n->value;
    }

    else if (BooleanNode* n = dynamic_cast<BooleanNode*>(node)) {
        return get_bool(n->value);
    }

    // ------------------------------------------------------------------
    // Variable read
    // ------------------------------------------------------------------

    else if (VarNode* n = dynamic_cast<VarNode*>(node)) {
        return get_var(n->value, n->token.get_line());
    }

    // ------------------------------------------------------------------
    // Binary operations
    // ------------------------------------------------------------------

    else if (BinOpNode* n = dynamic_cast<BinOpNode*>(node)) {
        int op_line = n->op.get_line();
        PrometheusValue left_val  = visit(n->left.get());
        PrometheusValue right_val = visit(n->right.get());
 
        switch (n->op.get_token()) {
 
            // Arithmetic ---------------------------------------------------
            case TokenType::PLUS: {
                // Allow string concatenation
                bool l_str = std::holds_alternative<std::string>(left_val);
                bool r_str = std::holds_alternative<std::string>(right_val);
                if (l_str || r_str) {
                    if (!l_str || !r_str) {
                        throw OperatorException("+",
                            "a mix of string and non-string operands "
                            "(use str() to convert)", op_line);
                    }
                    return std::get<std::string>(left_val) + std::get<std::string>(right_val);
                }
                return get_double(left_val, op_line) + get_double(right_val, op_line);
            }
 
            case TokenType::MINUS:
                return get_double(left_val, op_line) - get_double(right_val, op_line);
 
            case TokenType::MULTIPLY:
                return get_double(left_val, op_line) * get_double(right_val, op_line);
 
            case TokenType::DIVIDE: {
                double rhs = get_double(right_val, op_line);
                if (rhs == 0.0) throw DivisionByZeroException(op_line);
                return get_double(left_val, op_line) / rhs;
            }
 
            case TokenType::MODULO: {
                int rhs = get_int(right_val, op_line);
                if (rhs == 0) throw DivisionByZeroException(op_line);
                return get_int(left_val, op_line) % rhs;
            }
 
            case TokenType::EXPONENT:
                return std::pow(get_double(left_val, op_line), get_double(right_val, op_line));
 
            // Comparison ---------------------------------------------------
            case TokenType::EQUAL:
                if (std::holds_alternative<std::string>(left_val) &&
                    std::holds_alternative<std::string>(right_val)) {
                    return get_string(left_val, op_line) == get_string(right_val, op_line);
                }
                return get_double(left_val, op_line) == get_double(right_val, op_line);
 
            case TokenType::NOTEQUAL:
                if (std::holds_alternative<std::string>(left_val) &&
                    std::holds_alternative<std::string>(right_val)) {
                    return get_string(left_val, op_line) != get_string(right_val, op_line);
                }
                return get_double(left_val, op_line) != get_double(right_val, op_line);
 
            case TokenType::GREATER:
                return get_double(left_val, op_line) > get_double(right_val, op_line);
            case TokenType::LESSER:
                return get_double(left_val, op_line) < get_double(right_val, op_line);
            case TokenType::GREATEREQ:
                return get_double(left_val, op_line) >= get_double(right_val, op_line);
            case TokenType::LESSEREQ:
                return get_double(left_val, op_line) <= get_double(right_val, op_line);
 
            // Logical ------------------------------------------------------
            case TokenType::AND: return get_bool(left_val, op_line) && get_bool(right_val, op_line);
            case TokenType::OR:  return get_bool(left_val, op_line) || get_bool(right_val, op_line);
 
            default:
                throw OperatorException(n->op.get_value(), "this expression", op_line);
        }
    }

    // ------------------------------------------------------------------
    // Unary operations
    // ------------------------------------------------------------------
 
    else if (UnaryOpNode* n = dynamic_cast<UnaryOpNode*>(node)) {
        PrometheusValue right_val = visit(n->right.get());
        int op_line = n->op.get_line();
 
        if (n->op.get_token() == TokenType::NOT) {
            return !get_bool(right_val, op_line);
        }
        throw OperatorException(n->op.get_value(), "this expression", op_line);
    }

    // ------------------------------------------------------------------
    // Variable declaration / assignment
    // ------------------------------------------------------------------
 
    else if (VarDeclNode* n = dynamic_cast<VarDeclNode*>(node)) {
        PrometheusValue value = visit(n->value_node.get());

        if (n->var_type != n->name) {
            // Typed declaration (e.g. `int x = 5;`) — coerce and create a
            // fresh binding in the current scope, possibly shadowing an outer one.
            value = coerce_to_declared(n->var_type, n->name, value);
            declare_var(n->name, value);
        } else {
            // Bare re-assignment (e.g. `x = 5;`) — update the nearest
            // existing binding, or create one in the current scope if new.
            set_var(n->name, value);
        }

        return value;
    }

    // ------------------------------------------------------------------
    // Increment / Decrement
    // ------------------------------------------------------------------
 
    else if (IncrementDecrementNode* n = dynamic_cast<IncrementDecrementNode*>(node)) {
        if (!has_var(n->name)) {
            throw UndefinedVariableException(n->name);
        }

        PrometheusValue current = get_var(n->name);

        if (!std::holds_alternative<int>(current) &&
            !std::holds_alternative<double>(current)) {
            throw TypeException(
                "Increment/decrement requires a numeric variable, but '" +
                n->name + "' is of type '" + type_name(current) + "'");
        }

        double new_val = get_double(current) + n->inc_val;
        PrometheusValue updated = std::holds_alternative<int>(current)
                                  ? PrometheusValue{static_cast<int>(new_val)}
                                  : PrometheusValue{new_val};
        set_var(n->name, updated);
        return updated;
    }

    // ------------------------------------------------------------------
    // Print
    // ------------------------------------------------------------------
 
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

    // ------------------------------------------------------------------
    // Input
    // ------------------------------------------------------------------
 
    else if (InputNode* n = dynamic_cast<InputNode*>(node)) {
        std::cout << n->msg;
        std::string user_input;
        if (!std::getline(std::cin, user_input)) {
            // EOF on stdin — return empty string rather than crashing
            return std::string{};
        }
        return user_input;
    }
 
    // ------------------------------------------------------------------
    // If / elif / else
    // ------------------------------------------------------------------
 
    else if (IfNode* n = dynamic_cast<IfNode*>(node)) {
        if (get_bool(visit(n->condition.get()))) {
            push_scope();
            for (auto& stmt : n->then_branch)
                visit(stmt.get());
            pop_scope();
            return std::monostate{};
        }

        for (auto& [condition, branch] : n->elif_branches) {
            if (get_bool(visit(condition.get()))) {
                push_scope();
                for (auto& stmt : branch)
                    visit(stmt.get());
                pop_scope();
                return std::monostate{};
            }
        }

        if (!n->else_branch.empty()) {
            push_scope();
            for (auto& stmt : n->else_branch)
                visit(stmt.get());
            pop_scope();
        }
        return std::monostate{};
    }

    // ------------------------------------------------------------------
    // While
    // ------------------------------------------------------------------
 
    else if (WhileNode* n = dynamic_cast<WhileNode*>(node)) {
        while (get_bool(visit(n->condition.get()))) {
            push_scope();
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
            pop_scope();
        }
        return std::monostate{};
    }

    // ------------------------------------------------------------------
    // For
    // ------------------------------------------------------------------
 
    else if (ForNode* n = dynamic_cast<ForNode*>(node)) {
        // The loop variable (e.g. `int i = 0`) lives in its own scope that
        // wraps the entire for loop — it is not visible after the loop ends.
        push_scope();
        visit(n->variable.get());
        while (get_bool(visit(n->condition.get()))) {
            push_scope();
            for (auto& stmt : n->do_branch)
                visit(stmt.get());
            pop_scope();
            visit(n->change_var.get());
        }
        pop_scope();   // discard the loop variable
        return std::monostate{};
    }

    // ------------------------------------------------------------------
    // Function declaration
    // ------------------------------------------------------------------
 
    else if (FunctionDeclNode* n = dynamic_cast<FunctionDeclNode*>(node)) {
        if (functions.count(n->name)) {
            throw RuntimeException("Function '" + n->name + "' is already defined");
        }
        functions[n->name] = n;
        return std::monostate{};
    }

    // ------------------------------------------------------------------
    // Return
    // ------------------------------------------------------------------
 
    else if (ReturnNode* n = dynamic_cast<ReturnNode*>(node)) {
        PrometheusValue value = n->value_node ? visit(n->value_node.get()) : std::monostate{};
        throw ReturnException{value};
    }

    // ------------------------------------------------------------------
    // Function call
    // ------------------------------------------------------------------
 
    else if (CallNode* n = dynamic_cast<CallNode*>(node)) {
 
        // ---- Built-in type conversions --------------------------------
 
        if (n->name == "int") {
            if (n->args.size() != 1)
                throw ArgumentCountException("int", 1, (int)n->args.size());
            PrometheusValue val = visit(n->args[0].get());
            // Already int: return as-is
            if (std::holds_alternative<int>(val)) return val;
            // double → int
            if (std::holds_alternative<double>(val))
                return static_cast<int>(std::get<double>(val));
            // bool → int
            if (std::holds_alternative<bool>(val))
                return static_cast<int>(std::get<bool>(val));
            // string → int
            const std::string& s = get_string(val);
            try { return std::stoi(s); }
            catch (...) {
                throw ConversionException(s, "int");
            }
        }
 
        if (n->name == "double") {
            if (n->args.size() != 1)
                throw ArgumentCountException("double", 1, (int)n->args.size());
            PrometheusValue val = visit(n->args[0].get());
            if (std::holds_alternative<double>(val)) return val;
            if (std::holds_alternative<int>(val))
                return static_cast<double>(std::get<int>(val));
            if (std::holds_alternative<bool>(val))
                return static_cast<double>(std::get<bool>(val));
            const std::string& s = get_string(val);
            try { return std::stod(s); }
            catch (...) {
                throw ConversionException(s, "double");
            }
        }
 
        if (n->name == "str") {
            if (n->args.size() != 1)
                throw ArgumentCountException("str", 1, (int)n->args.size());
            PrometheusValue val = visit(n->args[0].get());
            return value_to_string(val);
        }
 
        if (n->name == "bool") {
            if (n->args.size() != 1)
                throw ArgumentCountException("bool", 1, (int)n->args.size());
            PrometheusValue val = visit(n->args[0].get());
            return get_bool(val);
        }
 
        // --- User-defined functions ---

        if (functions.find(n->name) == functions.end()) {
            throw UndefinedFunctionException(n->name);
        }

        FunctionDeclNode* func_node = functions[n->name];

        // 1. Validate argument count against min/max allowed
        size_t total_params = func_node->params.size();
        size_t provided_args = n->args.size();

        // Calculate how many parameters have default values (from right to left)
        size_t min_args = 0;
        for (const auto& param : func_node->params) {
            if (param.default_val == nullptr) min_args++;
            else break; 
        }

        if (provided_args < min_args || provided_args > total_params) {
            throw ArgumentCountException(n->name, (int)total_params, (int)provided_args);
        }

        // 2. Evaluate provided arguments in the CALLER'S scope
        std::vector<PrometheusValue> arg_values;
        for (auto& arg : n->args) {
            arg_values.push_back(visit(arg.get()));
        }

        // 3. Prepare the function's local execution environment
        Interpreter local_interp(func_node->body);
        local_interp.functions = functions;

        // 4. Bind parameters (using defaults where necessary)
        for (size_t i = 0; i < total_params; i++) {
            const auto& param = func_node->params[i];
            PrometheusValue final_val;

            if (i < provided_args) {
                // Use the value passed by the caller
                final_val = coerce_to_declared(param.type, param.name, arg_values[i]);
            } else {
                // Use the default expression defined in the function signature
                // IMPORTANT: visit the default expression in the CALLER'S context
                // or a neutral context, depending on your language's scoping rules.
                final_val = coerce_to_declared(param.type, param.name, visit(param.default_val.get()));
            }

            local_interp.declare_var(param.name, final_val);
        }

        // 5. Execute function body
        try {
            for (auto& stmt : func_node->body)
                local_interp.visit(stmt.get());
        } catch (const ReturnException& e) {
            return e.value;
        }
    }

    // ------------------------------------------------------------------
    // List literal  [expr, expr, ...]
    // ------------------------------------------------------------------

    else if (ListLiteralNode* n = dynamic_cast<ListLiteralNode*>(node)) {
        // A bare literal has no declared element type yet — the element type
        // is stamped in by ListDeclNode. Return a typeless list here; the
        // caller (ListDeclNode) will validate and re-tag it.
        auto lst = std::make_shared<PrometheusList>();
        lst->element_type = "";  // will be set by ListDeclNode
        for (auto& elem : n->elements)
            lst->elements.push_back(visit(elem.get()));
        return lst;
    }

    // ------------------------------------------------------------------
    // List declaration   list[type] name = [...];
    // ------------------------------------------------------------------

    else if (ListDeclNode* n = dynamic_cast<ListDeclNode*>(node)) {
        PrometheusValue raw = visit(n->value_node.get());

        PrometheusListPtr lst;
        if (std::holds_alternative<PrometheusListPtr>(raw)) {
            lst = std::get<PrometheusListPtr>(raw);
        } else {
            throw TypeException(
                "Expected a list literal for declaration of '" + n->name + "'");
        }

        // Stamp the declared element type and coerce every initial element.
        lst->element_type = n->element_type;
        for (auto& elem : lst->elements)
            elem = coerce_to_element(n->element_type, elem);

        declare_var(n->name, lst);
        return lst;
    }

    // ------------------------------------------------------------------
    // List index read   name[index]
    // ------------------------------------------------------------------

    else if (ListIndexNode* n = dynamic_cast<ListIndexNode*>(node)) {
        PrometheusValue var = get_var(n->name);
        if (!std::holds_alternative<PrometheusListPtr>(var))
            throw TypeException("'" + n->name + "' is not a list");

        PrometheusListPtr lst = std::get<PrometheusListPtr>(var);
        int idx = get_int(visit(n->index.get()));

        if (idx < 0 || idx >= (int)lst->elements.size()) {
            throw RuntimeException(
                "Index " + std::to_string(idx) + " out of bounds for list '" +
                n->name + "' (size " + std::to_string(lst->elements.size()) + ")");
        }
        return lst->elements[idx];
    }

    // ------------------------------------------------------------------
    // List index assign   name[index] = value;
    // ------------------------------------------------------------------

    else if (ListAssignNode* n = dynamic_cast<ListAssignNode*>(node)) {
        PrometheusValue var = get_var(n->name);
        if (!std::holds_alternative<PrometheusListPtr>(var))
            throw TypeException("'" + n->name + "' is not a list");

        PrometheusListPtr lst = std::get<PrometheusListPtr>(var);
        int idx = get_int(visit(n->index.get()));

        if (idx < 0 || idx >= (int)lst->elements.size()) {
            throw RuntimeException(
                "Index " + std::to_string(idx) + " out of bounds for list '" +
                n->name + "' (size " + std::to_string(lst->elements.size()) + ")");
        }

        PrometheusValue new_val = visit(n->value.get());
        lst->elements[idx] = coerce_to_element(lst->element_type, new_val);
        return lst->elements[idx];
    }

    // ------------------------------------------------------------------
    // List append   name.append(value);
    // ------------------------------------------------------------------

    else if (ListAppendNode* n = dynamic_cast<ListAppendNode*>(node)) {
        PrometheusValue var = get_var(n->name);
        if (!std::holds_alternative<PrometheusListPtr>(var))
            throw TypeException("'" + n->name + "' is not a list");

        PrometheusListPtr lst = std::get<PrometheusListPtr>(var);
        PrometheusValue new_val = visit(n->value.get());
        lst->elements.push_back(coerce_to_element(lst->element_type, new_val));
        return std::monostate{};
    }

    // ------------------------------------------------------------------
    // List length   name.len()
    // ------------------------------------------------------------------

    else if (ListLengthNode* n = dynamic_cast<ListLengthNode*>(node)) {
        PrometheusValue var = get_var(n->name);
        if (!std::holds_alternative<PrometheusListPtr>(var))
            throw TypeException("'" + n->name + "' is not a list");

        PrometheusListPtr lst = std::get<PrometheusListPtr>(var);
        return static_cast<int>(lst->elements.size());
    }
 
    return std::monostate{};
}