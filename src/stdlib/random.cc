#include <random>
#include "stdlib/random_functions.h"
#include "exceptions.h"

void register_random_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions,
    std::mt19937& generator,
    int& last_seed)
{
    // Seed with a real random device on first registration.
    std::random_device rd;
    generator.seed(rd());

    native_functions["__native_seed"] = [&generator, &last_seed](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 1)
            throw ArgumentCountException("seed", 1, (int)args.size(), line);
        if (!std::holds_alternative<int>(args[0]))
            throw TypeException("'seed' requires an integer argument", line);
        int s = std::get<int>(args[0]);
        last_seed = s;
        generator.seed(s);
        return 0;
    };

    native_functions["__native_get_seed"] = [&last_seed](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        (void)args; (void)line;
        return last_seed;
    };

    native_functions["__native_random"] = [&generator](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        (void)args; (void)line;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(generator);
    };

    native_functions["__native_randint"] = [&generator](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 2)
            throw ArgumentCountException("randint", 2, (int)args.size(), line);
        if (!std::holds_alternative<int>(args[0]) || !std::holds_alternative<int>(args[1]))
            throw TypeException("'randint' requires integer arguments", line);
        int min = std::get<int>(args[0]);
        int max = std::get<int>(args[1]);
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    };
}