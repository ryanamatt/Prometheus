#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "exceptions.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

// ============================================================================
// Construction
// ============================================================================

Interpreter::Interpreter(const std::vector<std::unique_ptr<ASTNode>>& nodes,
                         std::unordered_map<std::string, PrometheusValue> global_vars,
                         std::string base_dir)
    : nodes(&nodes), base_dir(std::move(base_dir))
{
    // Inject the version into the global scope
    global_vars["__version__"] = PROMETHEUS_VERSION;

    // The global scope is always present at index 0.
    scope_stack.push_back(std::move(global_vars));
}

// ============================================================================
// Scope helpers
// ============================================================================

void Interpreter::push_scope() {
    scope_stack.push_back({});
}

void Interpreter::pop_scope() {
    if (scope_stack.size() > 1)   // never pop the global scope
        scope_stack.pop_back();
}

PrometheusValue Interpreter::get_var(const std::string& name, int line) const {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end())
            return found->second;
    }
    throw UndefinedVariableException(name, line);
}

void Interpreter::set_var(const std::string& name, PrometheusValue value) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second = std::move(value);
            return;
        }
    }
    scope_stack.back()[name] = std::move(value);
}

void Interpreter::declare_var(const std::string& name, PrometheusValue value) {
    scope_stack.back()[name] = std::move(value);
}

bool Interpreter::has_var(const std::string& name) const {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
        if (it->count(name)) return true;
    return false;
}

// ============================================================================
// Stdlib registry
// ============================================================================

void Interpreter::init_stdlib_registry() {
    if (!stdlib_registry.empty()) return;

    std::filesystem::path stdlib_dir;
    
    // Check Environment Variable 
    if (const char* env = std::getenv("PROMETHEUS_STDLIB")) {
        stdlib_dir = env;
    } 
    // Check Local Directory (Development mode) 
    else if (std::filesystem::exists("stdlib")) {
        stdlib_dir = std::filesystem::path("stdlib");
    }
    // Check Global Installation Path (Release mode)
    else {
#ifdef _WIN32
        stdlib_dir = "C:/ProgramData/prometheus/stdlib";
#else
        stdlib_dir = "/usr/local/share/prometheus/stdlib";
#endif
    }

    std::error_code ec;
    if (!std::filesystem::is_directory(stdlib_dir, ec)) return;

    for (const auto& entry : std::filesystem::directory_iterator(stdlib_dir, ec)) {
    if (ec) break;
    if (!entry.is_regular_file()) continue;
    auto p = entry.path();
    if (p.extension() == ".prm") {
        // Change: Convert the path to an absolute path before storing it
        stdlib_registry[p.stem().string()] = std::filesystem::absolute(p).string();
    }
}
}

// ============================================================================
// exec_import / exec_use
// ============================================================================

void Interpreter::exec_import(ImportNode* node) {
    std::filesystem::path raw(node->path);
    if (raw.extension().empty())
        raw += ".prm";

    std::filesystem::path resolved;
    if (raw.is_absolute()) {
        resolved = raw;
    } else {
        std::string dir = node->base_dir.empty() ? base_dir : node->base_dir;
        resolved = dir.empty()
            ? std::filesystem::current_path() / raw
            : std::filesystem::path(dir) / raw;
    }

    std::error_code ec;
    resolved = std::filesystem::weakly_canonical(resolved, ec);

    std::string key = resolved.string();
    if (imported_files.count(key)) return;
    imported_files.insert(key);

    std::ifstream f(resolved);
    if (!f.is_open())
        throw RuntimeException("import: cannot open file '" + resolved.string() + "'");

    std::stringstream buf;
    buf << f.rdbuf();

    Lexer lex(buf.str());
    Parser parser(lex.tokenize());
    std::vector<std::unique_ptr<ASTNode>> tree = parser.parse();

    std::string child_base = resolved.parent_path().string();
    std::string saved_base = base_dir;
    base_dir = child_base;

    owned_trees.push_back(std::move(tree));
    for (auto& stmt : owned_trees.back())
        visit(stmt.get());

    base_dir = saved_base;
}

void Interpreter::exec_use(UseNode* node) {
    if (node->module_name == "math" && !loaded_modules.count("math"))
        register_math_functions();

    else if (node->module_name == "random" && !loaded_modules.count("random"))
        register_random_functions();

    if (loaded_modules.count(node->module_name)) return;

    init_stdlib_registry();

    auto it = stdlib_registry.find(node->module_name);
    if (it == stdlib_registry.end()) {
        std::string list;
        for (auto& [k, v] : stdlib_registry) {
            if (!list.empty()) list += ", ";
            list += k;
        }
        throw RuntimeException(
            "use: unknown standard library module '" + node->module_name + "'. "
            "Available modules: " + (list.empty() ? "(none found)" : list));
    }

    loaded_modules.insert(node->module_name);
    ImportNode import_node(it->second);
    import_node.base_dir = "";
    exec_import(&import_node);
}

// ============================================================================
// interpret — top-level entry point
// ============================================================================

std::unordered_map<std::string, PrometheusValue> Interpreter::interpret() {
    for (auto& node : *nodes)
        visit(node.get());
    return scope_stack.front();
}

// ============================================================================
// visit(ASTNode*) — the single dispatch bridge
// ============================================================================

PrometheusValue Interpreter::visit(ASTNode* node) {
    return node->accept(*this);
}

// ============================================================================
// Static helpers — value extraction and coercion
// ============================================================================

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

static double get_double(const PrometheusValue& v, int line = 0) {
    if (auto* i = std::get_if<int>(&v))    return static_cast<double>(*i);
    if (auto* d = std::get_if<double>(&v)) return *d;
    throw TypeException(
        "Expected a numeric value but got '" + type_name(v) + "'", line);
}

static int get_int(const PrometheusValue& v, int line = 0) {
    if (auto* i = std::get_if<int>(&v))    return *i;
    if (auto* d = std::get_if<double>(&v)) return static_cast<int>(*d);
    throw TypeException(
        "Expected an integer value but got '" + type_name(v) + "'", line);
}

static const std::string& get_string(const PrometheusValue& v, int line = 0) {
    if (auto* s = std::get_if<std::string>(&v)) return *s;
    throw TypeException(
        "Expected a string value but got '" + type_name(v) + "'", line);
}

static bool get_bool(const PrometheusValue& v, int /*line*/ = 0) {
    if (auto* b = std::get_if<bool>(&v))        return *b;
    if (auto* i = std::get_if<int>(&v))         return *i != 0;
    if (auto* d = std::get_if<double>(&v))      return *d != 0.0;
    if (auto* s = std::get_if<std::string>(&v)) return !s->empty() && *s != "false";
    return false;
}

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
            if (std::holds_alternative<std::string>(elems[i]))
                out += "\"" + std::get<std::string>(elems[i]) + "\"";
            else
                out += value_to_string(elems[i]);
        }
        return out + "]";
    }
    return "None";
}

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
    return value;   // unknown / user-defined type — pass through
}

static PrometheusValue coerce_to_element(const std::string& elem_type,
                                          const PrometheusValue& value,
                                          int line = 0) {
    return coerce_to_declared(elem_type, "<list element>", value, line);
}

// ============================================================================
// Visitor overloads
// ============================================================================

// ----------------------------------------------------------------------------
// Literals
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(NumberNode* n) {
    if (n->value.find('.') != std::string::npos) {
        try { return std::stod(n->value); }
        catch (...) {
            throw RuntimeException(
                "Malformed numeric literal '" + n->value + "'", n->token.get_line());
        }
    }
    try { return std::stoi(n->value); }
    catch (...) {
        throw RuntimeException(
            "Integer literal out of range '" + n->value + "'", n->token.get_line());
    }
}

PrometheusValue Interpreter::visit(StringNode* n) {
    return n->value;
}

PrometheusValue Interpreter::visit(BooleanNode* n) {
    return get_bool(n->value);
}

// ----------------------------------------------------------------------------
// Variable read
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(VarNode* n) {
    return get_var(n->value, n->token.get_line());
}

// ----------------------------------------------------------------------------
// Binary operations
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(BinOpNode* n) {
    int op_line = n->op.get_line();
    PrometheusValue lv = visit(n->left.get());
    PrometheusValue rv = visit(n->right.get());

    switch (n->op.get_token()) {

        // Arithmetic -------------------------------------------------------
        case TokenType::PLUS: {
            bool l_str = std::holds_alternative<std::string>(lv);
            bool r_str = std::holds_alternative<std::string>(rv);
            if (l_str || r_str) {
                if (!l_str || !r_str)
                    throw OperatorException("+",
                        "a mix of string and non-string operands "
                        "(use str() to convert)", op_line);
                return std::get<std::string>(lv) + std::get<std::string>(rv);
            }
            return get_double(lv, op_line) + get_double(rv, op_line);
        }

        case TokenType::MINUS:
            return get_double(lv, op_line) - get_double(rv, op_line);

        case TokenType::MULTIPLY:
            return get_double(lv, op_line) * get_double(rv, op_line);

        case TokenType::DIVIDE: {
            double rhs = get_double(rv, op_line);
            if (rhs == 0.0) throw DivisionByZeroException(op_line);
            return get_double(lv, op_line) / rhs;
        }

        case TokenType::MODULO: {
            int rhs = get_int(rv, op_line);
            if (rhs == 0) throw DivisionByZeroException(op_line);
            return get_int(lv, op_line) % rhs;
        }

        case TokenType::EXPONENT:
            return std::pow(get_double(lv, op_line), get_double(rv, op_line));

        // Comparison -------------------------------------------------------
        case TokenType::EQUAL:
            if (std::holds_alternative<std::string>(lv) &&
                std::holds_alternative<std::string>(rv))
                return get_string(lv, op_line) == get_string(rv, op_line);
            return get_double(lv, op_line) == get_double(rv, op_line);

        case TokenType::NOTEQUAL:
            if (std::holds_alternative<std::string>(lv) &&
                std::holds_alternative<std::string>(rv))
                return get_string(lv, op_line) != get_string(rv, op_line);
            return get_double(lv, op_line) != get_double(rv, op_line);

        case TokenType::GREATER:   return get_double(lv, op_line) >  get_double(rv, op_line);
        case TokenType::LESSER:    return get_double(lv, op_line) <  get_double(rv, op_line);
        case TokenType::GREATEREQ: return get_double(lv, op_line) >= get_double(rv, op_line);
        case TokenType::LESSEREQ:  return get_double(lv, op_line) <= get_double(rv, op_line);

        // Logical ----------------------------------------------------------
        case TokenType::AND: return get_bool(lv, op_line) && get_bool(rv, op_line);
        case TokenType::OR:  return get_bool(lv, op_line) || get_bool(rv, op_line);

        default:
            throw OperatorException(n->op.get_value(), "this expression", op_line);
    }
}

// ----------------------------------------------------------------------------
// Unary operations
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(UnaryOpNode* n) {
    PrometheusValue rv = visit(n->right.get());
    int op_line = n->op.get_line();

    if (n->op.get_token() == TokenType::NOT)
        return !get_bool(rv, op_line);

    if (n->op.get_token() == TokenType::MINUS) {
        if (std::holds_alternative<int>(rv))    return -std::get<int>(rv);
        if (std::holds_alternative<double>(rv)) return -std::get<double>(rv);
        return -get_double(rv, op_line);
    }

    throw OperatorException(n->op.get_value(), "this expression", op_line);
}

// ----------------------------------------------------------------------------
// Variable declaration / assignment
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(VarDeclNode* n) {
    PrometheusValue value = visit(n->value_node.get());

    if (n->var_type != n->name) {
        // Typed declaration — coerce and shadow any outer binding.
        value = coerce_to_declared(n->var_type, n->name, value);
        declare_var(n->name, value);
    } else {
        // Bare assignment (e.g., x = 10)
        if (!has_var(n->name))
            throw UndefinedVariableException(n->name, n->token_line);

        // Bare re-assignment — update the nearest existing binding.
        set_var(n->name, value);
    }
    return value;
}

// ----------------------------------------------------------------------------
// Increment / Decrement
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(IncrementDecrementNode* n) {
    if (!has_var(n->name))
        throw UndefinedVariableException(n->name);

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

// ----------------------------------------------------------------------------
// Print
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(PrintNode* n) {
    std::stringstream ss;
    for (size_t i = 0; i < n->expressions.size(); ++i) {
        if (i > 0) ss << " ";
        ss << value_to_string(visit(n->expressions[i].get()));
    }
    std::string output = ss.str();
    std::cout << output << std::endl;
    return output;
}

// ----------------------------------------------------------------------------
// Input
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(InputNode* n) {
    std::cout << n->msg;
    std::string user_input;
    if (!std::getline(std::cin, user_input))
        return std::string{};
    return user_input;
}

// ----------------------------------------------------------------------------
// Range
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(RangeNode* n) {
    int start_val = get_int(visit(n->start.get()));
    int stop_val  = get_int(visit(n->stop.get()));
    int step_val  = get_int(visit(n->step.get()));

    if (step_val == 0)
        throw RuntimeException("range() step argument must not be zero");

    auto lst = std::make_shared<PrometheusList>();
    lst->element_type = "int";

    if (step_val > 0)
        for (int i = start_val; i < stop_val; i += step_val) lst->elements.push_back(i);
    else
        for (int i = start_val; i > stop_val; i += step_val) lst->elements.push_back(i);

    return lst;
}

// ----------------------------------------------------------------------------
// If / elif / else
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(IfNode* n) {
    if (get_bool(visit(n->condition.get()))) {
        push_scope();
        for (auto& stmt : n->then_branch) visit(stmt.get());
        pop_scope();
        return std::monostate{};
    }

    for (auto& [cond, branch] : n->elif_branches) {
        if (get_bool(visit(cond.get()))) {
            push_scope();
            for (auto& stmt : branch) visit(stmt.get());
            pop_scope();
            return std::monostate{};
        }
    }

    if (!n->else_branch.empty()) {
        push_scope();
        for (auto& stmt : n->else_branch) visit(stmt.get());
        pop_scope();
    }
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// While
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(WhileNode* n) {
    while (get_bool(visit(n->condition.get()))) {
        push_scope();
        for (auto& stmt : n->do_branch) visit(stmt.get());
        pop_scope();
    }
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// For (C-style)
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ForNode* n) {
    push_scope();
    visit(n->variable.get());
    while (get_bool(visit(n->condition.get()))) {
        push_scope();
        for (auto& stmt : n->do_branch) visit(stmt.get());
        pop_scope();
        visit(n->change_var.get());
    }
    pop_scope();
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// For-in (range-based)
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ForInNode* n) {
    PrometheusValue iterable = visit(n->list_expr.get());
    if (!std::holds_alternative<PrometheusListPtr>(iterable))
        throw TypeException(
            "for-in loop requires a list, but got '" + type_name(iterable) + "'");

    auto lst = std::get<PrometheusListPtr>(iterable);
    for (const PrometheusValue& elem : lst->elements) {
        push_scope();
        declare_var(n->var_name, coerce_to_declared(n->var_type, n->var_name, elem));
        for (auto& stmt : n->body) visit(stmt.get());
        pop_scope();
    }
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// Function declaration
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(FunctionDeclNode* n) {
    if (functions.count(n->name))
        throw RuntimeException("Function '" + n->name + "' is already defined");
    functions[n->name] = n;
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// Return
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ReturnNode* n) {
    PrometheusValue value = n->value_node ? visit(n->value_node.get()) : std::monostate{};
    throw ReturnException{value};
}

// ----------------------------------------------------------------------------
// Function call
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(CallNode* n) {

    // Built-in type conversions --------------------------------------------
    if (n->name == "int") {
        if (n->args.size() != 1)
            throw ArgumentCountException("int", 1, (int)n->args.size());
        PrometheusValue val = visit(n->args[0].get());
        if (std::holds_alternative<int>(val))    return val;
        if (std::holds_alternative<double>(val)) return static_cast<int>(std::get<double>(val));
        if (std::holds_alternative<bool>(val))   return static_cast<int>(std::get<bool>(val));
        const std::string& s = get_string(val);
        try { return std::stoi(s); } catch (...) { throw ConversionException(s, "int"); }
    }

    if (n->name == "double") {
        if (n->args.size() != 1)
            throw ArgumentCountException("double", 1, (int)n->args.size());
        PrometheusValue val = visit(n->args[0].get());
        if (std::holds_alternative<double>(val)) return val;
        if (std::holds_alternative<int>(val))    return static_cast<double>(std::get<int>(val));
        if (std::holds_alternative<bool>(val))   return static_cast<double>(std::get<bool>(val));
        const std::string& s = get_string(val);
        try { return std::stod(s); } catch (...) { throw ConversionException(s, "double"); }
    }

    if (n->name == "str") {
        if (n->args.size() != 1)
            throw ArgumentCountException("str", 1, (int)n->args.size());
        return value_to_string(visit(n->args[0].get()));
    }

    if (n->name == "bool") {
        if (n->args.size() != 1)
            throw ArgumentCountException("bool", 1, (int)n->args.size());
        return get_bool(visit(n->args[0].get()));
    }

    // Native (C++) functions -----------------------------------------------
    auto native_it = native_functions.find(n->name);
    if (native_it != native_functions.end()) {
        std::vector<PrometheusValue> arg_vals;
        for (auto& arg : n->args)
            arg_vals.push_back(visit(arg.get()));
        return native_it->second(arg_vals, 1);
    }

    // User-defined functions -----------------------------------------------
    if (!functions.count(n->name))
        throw UndefinedFunctionException(n->name);

    FunctionDeclNode* func_node = functions[n->name];
    size_t total_params  = func_node->params.size();
    size_t provided_args = n->args.size();

    size_t min_args = 0;
    for (const auto& p : func_node->params) {
        if (p.default_val == nullptr) min_args++;
        else break;
    }

    if (provided_args < min_args || provided_args > total_params)
        throw ArgumentCountException(n->name, (int)total_params, (int)provided_args);

    // Evaluate arguments in caller's scope
    std::vector<PrometheusValue> arg_values;
    for (auto& arg : n->args)
        arg_values.push_back(visit(arg.get()));

    // Build a local interpreter for the function body
    Interpreter local_interp(func_node->body);
    local_interp.functions       = functions;
    local_interp.native_functions = native_functions;

    for (size_t i = 0; i < total_params; ++i) {
        const auto& param = func_node->params[i];
        PrometheusValue final_val = (i < provided_args)
            ? coerce_to_declared(param.type, param.name, arg_values[i])
            : coerce_to_declared(param.type, param.name,
                                 visit(param.default_val.get()));
        local_interp.declare_var(param.name, final_val);
    }

    PrometheusValue result = std::monostate{};
    try {
        for (auto& stmt : func_node->body)
            local_interp.visit(stmt.get());
    } catch (const ReturnException& e) {
        result = e.value;
    }

    return coerce_to_declared(func_node->return_type, "return", result);
}

// ----------------------------------------------------------------------------
// List literal
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListLiteralNode* n) {
    auto lst = std::make_shared<PrometheusList>();
    lst->element_type = "";   // stamped by ListDeclNode
    for (auto& elem : n->elements)
        lst->elements.push_back(visit(elem.get()));
    return lst;
}

// ----------------------------------------------------------------------------
// List declaration
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListDeclNode* n) {
    PrometheusValue raw = visit(n->value_node.get());
    if (!std::holds_alternative<PrometheusListPtr>(raw))
        throw TypeException(
            "Expected a list literal for declaration of '" + n->name + "'");

    auto lst = std::get<PrometheusListPtr>(raw);
    lst->element_type = n->element_type;
    for (auto& elem : lst->elements)
        elem = coerce_to_element(n->element_type, elem);

    declare_var(n->name, lst);
    return lst;
}

// ----------------------------------------------------------------------------
// List index read
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListIndexNode* n) {
    PrometheusValue var = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var))
        throw TypeException("'" + n->name + "' is not a list");

    auto lst = std::get<PrometheusListPtr>(var);
    int idx  = get_int(visit(n->index.get()));

    if (idx < 0 || idx >= (int)lst->elements.size())
        throw RuntimeException(
            "Index " + std::to_string(idx) + " out of bounds for list '" +
            n->name + "' (size " + std::to_string(lst->elements.size()) + ")");

    return lst->elements[idx];
}

// ----------------------------------------------------------------------------
// List index assign
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListAssignNode* n) {
    PrometheusValue var = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var))
        throw TypeException("'" + n->name + "' is not a list");

    auto lst = std::get<PrometheusListPtr>(var);
    int idx  = get_int(visit(n->index.get()));

    if (idx < 0 || idx >= (int)lst->elements.size())
        throw RuntimeException(
            "Index " + std::to_string(idx) + " out of bounds for list '" +
            n->name + "' (size " + std::to_string(lst->elements.size()) + ")");

    lst->elements[idx] = coerce_to_element(lst->element_type, visit(n->value.get()));
    return lst->elements[idx];
}

// ----------------------------------------------------------------------------
// List append
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListAppendNode* n) {
    PrometheusValue var = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var))
        throw TypeException("'" + n->name + "' is not a list");

    auto lst = std::get<PrometheusListPtr>(var);
    lst->elements.push_back(coerce_to_element(lst->element_type, visit(n->value.get())));
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// List length
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListLengthNode* n) {
    PrometheusValue var = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var))
        throw TypeException("'" + n->name + "' is not a list");

    return static_cast<int>(std::get<PrometheusListPtr>(var)->elements.size());
}

// ----------------------------------------------------------------------------
// List insert
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListInsertNode* n) {
    PrometheusValue var_name = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var_name))
        throw TypeException("'" + n->name + "' is not a list");
    auto lst_name = std::get<PrometheusListPtr>(var_name);

    int index  = get_int(visit(n->index.get()));
    PrometheusValue value = visit(n->value.get()); 
    lst_name->elements.insert(lst_name->elements.begin() + index, 
        coerce_to_element(lst_name->element_type, visit(n->value.get())));
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// List Pop
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListPopNode* n) {
    PrometheusValue var_name = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var_name))
        throw TypeException("'" + n->name + "' is not a list");
    auto lst = std::get<PrometheusListPtr>(var_name);

    if (lst->elements.empty())
        return std::monostate{};

    PrometheusValue lastValue = lst->elements.back();
    lst->elements.pop_back();
    return lastValue;
}

// ----------------------------------------------------------------------------
// List Remove
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListRemoveNode* n) {
    PrometheusValue var_name = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var_name))
        throw TypeException("'" + n->name + "' is not a list");
    auto lst = std::get<PrometheusListPtr>(var_name);

    PrometheusValue value = visit(n->value.get());
    auto it = std::find(lst->elements.begin(), lst->elements.end(), value);
    if (it != lst->elements.end())
        lst->elements.erase(it);
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// List Clear
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ListClearNode* n) {
    PrometheusValue var_name = get_var(n->name);
    if (!std::holds_alternative<PrometheusListPtr>(var_name))
        throw TypeException("'" + n->name + "' is not a list");
    auto lst = std::get<PrometheusListPtr>(var_name);

    lst->elements.clear();
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// Import
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(ImportNode* n) {
    if (n->base_dir.empty()) n->base_dir = base_dir;
    exec_import(n);
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// Use
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(UseNode* n) {
    exec_use(n);
    return std::monostate{};
}

// ----------------------------------------------------------------------------
// EOF sentinel
// ----------------------------------------------------------------------------

PrometheusValue Interpreter::visit(EOFNode* /*n*/) {
    return std::monostate{};
}