#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include "stdlib/time_functions.h"
#include "exceptions.h"

void register_time_functions(
    std::unordered_map<std::string, NativeFunction>& native_functions)
{
    // Returns current Unix timestamp in seconds
    native_functions["__native_now"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 0)
            throw ArgumentCountException("now()", 0, 1, line);

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return static_cast<int>(seconds);
    };

    // Pauses execution for X milliseconds
    native_functions["__native_sleep"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 1)
            throw ArgumentCountException("sleep", 1, (int)args.size(), line);

        int ms = 0;
        if (auto* i = std::get_if<int>(&args[0])) {
            ms = *i;
        } else if (auto* d = std::get_if<double>(&args[0])) {
            ms = static_cast<int>(*d);
        } else {
            throw TypeException("'sleep' requires a numeric argument (milliseconds)", line);
        }

        if (ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return std::monostate{}; // Return void
    };

    // High-resolution steady clock for profiling (returns double seconds)
    native_functions["__native_clock"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (!args.empty())
            throw ArgumentCountException("clock", 0, (int)args.size(), line);

        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<double>(duration).count();
    };

    // format_time(timestamp, format_string) -> str
    native_functions["__native_format_time"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (args.size() != 2)
            throw ArgumentCountException("format_time", 2, (int)args.size(), line);

        int timestamp = 0;
        if (auto* i = std::get_if<int>(&args[0])) timestamp = *i;
        else throw TypeException("'format_time' argument 1 must be an integer timestamp", line);

        std::string format;
        if (auto* s = std::get_if<std::string>(&args[1])) format = *s;
        else throw TypeException("'format_time' argument 2 must be a string", line);

        std::time_t tt = static_cast<std::time_t>(timestamp);
        std::tm tm = *std::localtime(&tt);
        std::stringstream ss;
        ss << std::put_time(&tm, format.c_str());
        return ss.str();
    };

    // iso8601() -> str (current time in YYYY-MM-DDTHH:MM:SSZ)
    native_functions["__native_iso8601"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (!args.empty())
            throw ArgumentCountException("iso8601", 0, (int)args.size(), line);

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    // ticks() -> double (High-resolution monotonic clock for start/end measurements)
    native_functions["__native_ticks"] = [](std::vector<PrometheusValue> args, int line) -> PrometheusValue {
        if (!args.empty())
            throw ArgumentCountException("ticks", 0, (int)args.size(), line);

        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        // Returns total seconds as a double for high precision arithmetic
        return std::chrono::duration<double>(duration).count();
    };
}