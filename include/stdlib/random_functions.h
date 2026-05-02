#ifndef RANDOM_FUNCTIONS_H
#define RANDOM_FUNCTIONS_H

#include <random>
#include <unordered_map>
#include <string>
#include "builtins.h"  // NativeFunction, PrometheusValue

/**
 * @brief Registers native random functions into the supplied table.
 * Functions are keyed with the "__native_" prefix so they are only
 * reachable through the Prometheus stdlib wrapper (random.prm), not
 * directly by user code.
 *
 * The caller owns the generator and last_seed state and passes them in
 * by reference so random.cc has no dependency on Interpreter at all.
 *
 * @param native_functions  The interpreter's native-function registry.
 * @param generator         The mt19937 engine (owned by Interpreter).
 * @param last_seed         Tracks the most recently set seed value.
 */
void register_random_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions,
    std::mt19937& generator,
    int& last_seed);

#endif // RANDOM_FUNCTIONS_H