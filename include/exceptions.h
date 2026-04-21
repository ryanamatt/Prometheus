#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <sstream>
#include "prometheus_types.h"

// ============================================================================
// Shared formatting helper
// ============================================================================

namespace detail {
    inline std::string with_line(const std::string& msg, int line) {
        if (line > 0) {
            std::ostringstream oss;
            oss << "[Line " << line << "] " << msg;
            return oss.str();
        }
        return msg;
    }
} // namespace detail

// ============================================================================
// Lexer Exceptions
// ============================================================================

/**
 * @brief Base class for all Prometheus lexer exceptions.
 *
 * Carries an optional line number so error messages can point the user
 * to the exact location in their source file. Derive all lexer-specific
 * exceptions from this class.
 */
class LexerException : public std::runtime_error {
protected:
    int line_;

    static std::string format(const std::string& msg, int line) {
        return detail::with_line(msg, line);
    }

public:
    explicit LexerException(const std::string& msg, int line = 0)
        : std::runtime_error(format(msg, line)), line_(line) {}

    int line() const noexcept { return line_; }
};

/**
 * @brief Thrown when the lexer encounters a character it does not recognise.
 * Example trigger: `@`, `#`, `$` in source input.
 */
class UnknownCharException : public LexerException {
public:
    explicit UnknownCharException(char ch, int line = 0)
        : LexerException(build_msg(ch), line) {}

private:
    static std::string build_msg(char ch) {
        std::ostringstream oss;
        oss << "Unknown character: '" << ch
            << "' (0x" << std::hex << static_cast<int>(static_cast<unsigned char>(ch)) << ")";
        return oss.str();
    }
};

/**
 * @brief Thrown when a string literal is opened but never closed.
 */
class UnterminatedStringException : public LexerException {
public:
    explicit UnterminatedStringException(int line = 0)
        : LexerException("Unterminated string literal: missing closing '\"'", line) {}
};

/**
 * @brief Thrown when a numeric literal is malformed (e.g. multiple dots).
 */
class InvalidNumberException : public LexerException {
public:
    explicit InvalidNumberException(const std::string& literal, int line = 0)
        : LexerException("Invalid number literal: '" + literal + "'", line) {}
};

/**
 * @brief Thrown when a block comment is opened but never closed.
 */
class UnterminatedCommentException : public LexerException {
public:
    explicit UnterminatedCommentException(int line = 0)
        : LexerException("Unterminated comment: missing closing marker", line) {}
};

// ============================================================================
// Parse Exceptions
// ============================================================================

/**
 * @brief Thrown by the Parser when a syntactic rule is violated.
 *
 * Now carries a line number so the user can find the bad token instantly.
 *
 * @code
 *   throw ParseException("Expected SEMICOLON, got RBRACE", 12);
 *   // what() → "[Line 12] Expected SEMICOLON, got RBRACE"
 * @endcode
 */
class ParseException : public std::runtime_error {
    int line_;
public:
    explicit ParseException(const std::string& msg, int line = 0)
        : std::runtime_error(detail::with_line(msg, line)), line_(line) {}

    int line() const noexcept { return line_; }
};

/**
 * @brief Thrown when a closing brace/bracket/paren is missing.
 *
 * @code
 *   throw MissingBraceException('{', 5);
 *   // what() → "[Line 5] Missing closing brace for '{' opened here"
 * @endcode
 */
class MissingBraceException : public ParseException {
public:
    explicit MissingBraceException(char opener, int line = 0)
        : ParseException(build_msg(opener), line) {}

private:
    static std::string build_msg(char opener) {
        std::ostringstream oss;
        char closer = (opener == '(') ? ')' : (opener == '{') ? '}' : ']';
        oss << "Missing closing '" << closer
            << "' for '" << opener << "' opened here";
        return oss.str();
    }
};

/**
 * @brief Thrown when a statement is missing its terminating semicolon.
 */
class MissingSemicolonException : public ParseException {
public:
    explicit MissingSemicolonException(const std::string& context, int line = 0)
        : ParseException("Missing ';' after " + context, line) {}
};

/**
 * @brief Thrown when a function declaration has a duplicate parameter name.
 */
class DuplicateParamException : public ParseException {
public:
    explicit DuplicateParamException(const std::string& func_name,
                                     const std::string& param_name,
                                     int line = 0)
        : ParseException("Duplicate parameter name '" + param_name +
                         "' in function '" + func_name + "'", line) {}
};

// ============================================================================
// Runtime Exceptions
// ============================================================================

/**
 * @brief Base class for all Prometheus runtime exceptions.
 *
 * Carries a line number extracted from the AST node's originating token
 * wherever possible.
 */
class RuntimeException : public std::runtime_error {
    int line_;
public:
    explicit RuntimeException(const std::string& msg, int line = 0)
        : std::runtime_error(detail::with_line(msg, line)),
          line_(line) {}

    int line() const noexcept { return line_; }
};

/**
 * @brief Thrown when a variable is used before it has been declared.
 *
 * @code
 *   throw UndefinedVariableException("x", 7);
 *   // what() → "[Line 7] Runtime Error: Undefined variable 'x'"
 * @endcode
 */
class UndefinedVariableException : public RuntimeException {
public:
    explicit UndefinedVariableException(const std::string& name, int line = 0)
        : RuntimeException("Undefined variable '" + name + "'", line) {}
};

/**
 * @brief Thrown when a called function has not been declared.
 */
class UndefinedFunctionException : public RuntimeException {
public:
    explicit UndefinedFunctionException(const std::string& name, int line = 0)
        : RuntimeException("Undefined function '" + name + "'", line) {}
};

/**
 * @brief Thrown when the number of arguments at a call site does not match
 *        the function's parameter list.
 */
class ArgumentCountException : public RuntimeException {
public:
    explicit ArgumentCountException(const std::string& func_name,
                                    int expected, int got,
                                    int line = 0)
        : RuntimeException(build_msg(func_name, expected, got), line) {}

private:
    static std::string build_msg(const std::string& name, int expected, int got) {
        std::ostringstream oss;
        oss << "Function '" << name << "' expects " << expected
            << " argument(s) but got " << got;
        return oss.str();
    }
};

/**
 * @brief Thrown when an operation receives a value of the wrong type
 *        (e.g., a string where a number is expected).
 */
class TypeException : public RuntimeException {
public:
    explicit TypeException(const std::string& msg, int line = 0)
        : RuntimeException(msg, line) {}
};

/**
 * @brief Thrown on an attempt to divide (or modulo) by zero.
 */
class DivisionByZeroException : public RuntimeException {
public:
    explicit DivisionByZeroException(int line = 0)
        : RuntimeException("Division by zero", line) {}
};

/**
 * @brief Thrown when a variable is assigned a value incompatible with its
 *        declared type (e.g., assigning a string to an `int` variable).
 */
class TypeMismatchException : public RuntimeException {
public:
    explicit TypeMismatchException(const std::string& var_name,
                                   const std::string& declared_type,
                                   const std::string& actual_type,
                                   int line = 0)
        : RuntimeException(build_msg(var_name, declared_type, actual_type), line) {}

private:
    static std::string build_msg(const std::string& var,
                                 const std::string& decl,
                                 const std::string& actual) {
        std::ostringstream oss;
        oss << "Cannot assign value of type '" << actual
            << "' to variable '" << var << "' declared as '" << decl << "'";
        return oss.str();
    }
};

/**
 * @brief Thrown when a built-in type conversion fails
 *        (e.g., int("hello")).
 */
class ConversionException : public RuntimeException {
public:
    explicit ConversionException(const std::string& value,
                                 const std::string& target_type,
                                 int line = 0)
        : RuntimeException("Cannot convert '" + value +
                           "' to type '" + target_type + "'", line) {}
};

/**
 * @brief Thrown when a unary or binary operator is applied to an unsupported
 *        type combination.
 */
class OperatorException : public RuntimeException {
public:
    explicit OperatorException(const std::string& op,
                               const std::string& type_desc,
                               int line = 0)
        : RuntimeException("Operator '" + op + "' cannot be applied to " + type_desc, line) {}
};

// ============================================================================
// Internal control-flow exception (not a user-visible error)
// ============================================================================

/**
 * @brief Used internally to unwind the call stack when a `return` statement
 *        is executed. Never exposed to the end user as an error.
 */
class ReturnException : public std::exception {
public:
    PrometheusValue value;
    explicit ReturnException(PrometheusValue val) : value(std::move(val)) {}
    const char* what() const noexcept override { return "Return Statement"; }
};

#endif // EXCEPTIONS_H