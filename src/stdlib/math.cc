#include <cmath>
#include "stdlib/math_functions.h"
#include "exceptions.h"

void register_math_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions)
{
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

    native_functions["__native_sin"]   = wrap1("sin",   std::sin);
    native_functions["__native_cos"]   = wrap1("cos",   std::cos);
    native_functions["__native_tan"]   = wrap1("tan",   std::tan);
    native_functions["__native_asin"]  = wrap1("asin",  std::asin);
    native_functions["__native_acos"]  = wrap1("acos",  std::acos);
    native_functions["__native_atan"]  = wrap1("atan",  std::atan);
    native_functions["__native_sqrt"]  = wrap1("sqrt",  std::sqrt);
    native_functions["__native_log"]   = wrap1("log",   std::log);
    native_functions["__native_log10"] = wrap1("log10", std::log10);
    native_functions["__native_exp"]   = wrap1("exp",   std::exp);
    native_functions["__native_floor"] = wrap1("floor", [](double x) { return std::floor(x); });
    native_functions["__native_ceil"]  = wrap1("ceil",  [](double x) { return std::ceil(x); });

    // atan2(y, x) — two args
    native_functions["__native_atan2"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
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