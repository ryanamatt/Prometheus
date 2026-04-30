#include <random>
#include "interpreter.h"

// Assuming you add std::mt19937 generator to your Interpreter header
void Interpreter::register_random_functions() {
    // Initialize with a real random device seed
    std::random_device rd;
    generator.seed(rd());

    // __native_random() -> returns double between [0.0, 1.0)
    native_functions["__native_random"] = [this](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        (void)args;
        (void)line;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(generator);
    };

    // __native_randint(min, max) -> returns int between [min, max]
    native_functions["__native_randint"] = [this](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 2) throw ArgumentCountException("randint", 2, (int)args.size(), line);
        
        // Helper to extract ints (similar to your to_d helper in math.cc)
        int min = std::get<int>(args[0]); 
        int max = std::get<int>(args[1]);
        
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    };
}