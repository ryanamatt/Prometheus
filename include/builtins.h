#ifndef BUILTINS_H
#define BUILTINS_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include "prometheus_types.h"

/**
 * @brief Registers the built-in functions (print, input, range) into the
 *        supplied native-function table.
 *
 * This is called once from the Interpreter constructor so that the three
 * former keyword-builtins are indistinguishable from any other native
 * callable at runtime.
 *
 * Signature of each entry:
 *   PrometheusValue fn(std::vector<PrometheusValue> args, int call_line)
 *
 * @param native_functions  The interpreter's native-function registry.
 *                          Entries are added (never replaced) by this call.
 */
using NativeFunction =
    std::function<PrometheusValue(std::vector<PrometheusValue>, int)>;

void register_builtins(
    std::unordered_map<std::string, NativeFunction>& native_functions);

#endif // BUILTINS_H