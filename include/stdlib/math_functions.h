#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include <unordered_map>
#include <string>
#include "builtins.h"  // NativeFunction, PrometheusValue

/**
 * @brief Registers native math functions into the supplied table.
 *
 * Functions are keyed with the "__native_" prefix so they are only
 * reachable through the Prometheus stdlib wrapper (math.prm), not
 * directly by user code.
 *
 * @param native_functions  The interpreter's native-function registry.
 */
void register_math_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions);

#endif // MATH_FUNCTIONS_H