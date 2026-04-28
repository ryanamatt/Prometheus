#include <cmath>
#include "exceptions.h"
#include "interpreter.h"

void Interpreter::register_math_functions() {
    // Single-arg double → double helpers
    auto wrap1 = [](std::string name, double(*fn)(double)) -> NativeFunction {
        return [name, fn](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
            if (args.size() != 1)
                throw ArgumentCountException(name, 1, (int)args.size(), line);
            if (auto* i = std::get_if<int>(&args[0]))
                return fn(static_cast<double>(*i));
            if (auto* d = std::get_if<double>(&args[0]))
                return fn(*d);
            throw TypeException("'" + name + "' requires a numeric argument", line);
        };
    };

    native_functions["sin"]   = wrap1("sin",   std::sin);
    native_functions["cos"]   = wrap1("cos",   std::cos);
    native_functions["tan"]   = wrap1("tan",   std::tan);
    native_functions["asin"]  = wrap1("asin",  std::asin);
    native_functions["acos"]  = wrap1("acos",  std::acos);
    native_functions["atan"]  = wrap1("atan",  std::atan);
    native_functions["sqrt"]  = wrap1("sqrt",  std::sqrt);
    native_functions["log"]   = wrap1("log",   std::log);
    native_functions["log10"] = wrap1("log10", std::log10);
    native_functions["exp"]   = wrap1("exp",   std::exp);
    native_functions["floor"] = wrap1("floor", [](double x) { return std::floor(x); });
    native_functions["ceil"]  = wrap1("ceil",  [](double x) { return std::ceil(x); });

    // atan2(y, x) — two args
    native_functions["atan2"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 2)
            throw ArgumentCountException("atan2", 2, (int)args.size(), line);
        auto to_d = [&](const PrometheusValue& v) -> double {
            if (auto* i = std::get_if<int>(&v))    return *i;
            if (auto* d = std::get_if<double>(&v)) return *d;
            throw TypeException("'atan2' requires numeric arguments", line);
        };
        return std::atan2(to_d(args[0]), to_d(args[1]));
    };
}