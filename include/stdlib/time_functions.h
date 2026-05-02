#ifndef TIME_FUNCTIONS_H
#define TIME_FUNCTIONS_H

#include <random>
#include <unordered_map>
#include <string>
#include "builtins.h"  // NativeFunction, PrometheusValue

/**
 * @brief Registers native time functions into the supplied table.
 * Functions are keyed with the "__native_" prefix so they are only
 * reachable through the Prometheus stdlib wrapper (time.prm), not
 * directly by user code.
 *
 * @param native_functions  The interpreter's native-function registry.
 */
void register_time_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions);

#endif // RANDOM_FUNCTIONS_H