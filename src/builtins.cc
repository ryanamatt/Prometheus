#include <iostream>
#include <sstream>
#include "builtins.h"
#include "exceptions.h"

// ============================================================================
// Value helpers (local copies — avoids a circular dependency on interpreter.h)
// ============================================================================

static std::string builtin_type_name(const PrometheusValue& v) {
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

static std::string value_to_string(const PrometheusValue& value) {
    if (auto* i  = std::get_if<int>(&value))    return std::to_string(*i);
    if (auto* d  = std::get_if<double>(&value)) {
        std::string s = std::to_string(*d);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
        return s;
    }
    if (auto* s  = std::get_if<std::string>(&value)) return *s;
    if (auto* b  = std::get_if<bool>(&value))        return *b ? "true" : "false";
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

static int get_int(const PrometheusValue& v, int line = 0) {
    if (auto* i = std::get_if<int>(&v))    return *i;
    if (auto* d = std::get_if<double>(&v)) return static_cast<int>(*d);
    throw TypeException(
        "Expected an integer value but got '" + builtin_type_name(v) + "'", line);
}

// ============================================================================
// Built-in registrations
// ============================================================================

void register_builtins(
    std::unordered_map<std::string, NativeFunction>& native_functions)
{
    // -----------------------------------------------------------------------
    // print(expr, ...)
    //
    // Accepts one or more arguments; prints them space-separated with a
    // trailing newline.  Returns the printed string.
    // -----------------------------------------------------------------------
    native_functions["print"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.empty())
            throw ArgumentCountException("print", 1, 0, line);

        std::stringstream ss;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) ss << " ";
            ss << value_to_string(args[i]);
        }
        std::string output = ss.str();
        std::cout << output << std::endl;
        return output;
    };

    // -----------------------------------------------------------------------
    // input(prompt?)
    //
    // Accepts zero or one string argument used as a prompt.  Reads a line
    // from stdin and returns it as a str value.
    // -----------------------------------------------------------------------
    native_functions["input"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() > 1)
            throw ArgumentCountException("input", 1, static_cast<int>(args.size()), line);

        if (!args.empty()) {
            if (!std::holds_alternative<std::string>(args[0]))
                throw TypeException(
                    "input() prompt must be a str, got '" +
                    builtin_type_name(args[0]) + "'", line);
            std::cout << std::get<std::string>(args[0]);
        }

        std::string user_input;
        if (!std::getline(std::cin, user_input))
            return std::string{};
        return user_input;
    };

    // -----------------------------------------------------------------------
    // range(stop) / range(start, stop) / range(start, stop, step)
    //
    // Returns a list[int] matching Python's range() semantics.
    // -----------------------------------------------------------------------
    native_functions["range"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.empty() || args.size() > 3)
            throw ArgumentCountException("range", 1, static_cast<int>(args.size()), line);

        int start_val = 0, stop_val = 0, step_val = 1;

        if (args.size() == 1) {
            stop_val = get_int(args[0], line);
        } else if (args.size() == 2) {
            start_val = get_int(args[0], line);
            stop_val  = get_int(args[1], line);
        } else {
            start_val = get_int(args[0], line);
            stop_val  = get_int(args[1], line);
            step_val  = get_int(args[2], line);
        }

        if (step_val == 0)
            throw RuntimeException("range() step argument must not be zero", line);

        auto lst = std::make_shared<PrometheusList>();
        lst->element_type = "int";

        if (step_val > 0)
            for (int i = start_val; i < stop_val; i += step_val)
                lst->elements.push_back(i);
        else
            for (int i = start_val; i > stop_val; i += step_val)
                lst->elements.push_back(i);

        return lst;
    };
}