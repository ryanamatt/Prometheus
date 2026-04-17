#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <sstream>
#include <any>

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

    /**
     * @brief Helper that prepends "[Line N] " to a message when a valid
     *        line number (> 0) is supplied.
     */
    static std::string format(const std::string& msg, int line) {
        if (line > 0) {
            std::ostringstream oss;
            oss << "[Line " << line << "] " << msg;
            return oss.str();
        }
        return msg;
    }

public:
    explicit LexerException(const std::string& msg, int line = 0)
        : std::runtime_error(format(msg, line)), line_(line) {}

    /** @brief Returns the 1-based source line where the error occurred, or 0 if unknown. */
    int line() const noexcept { return line_; }
};

// ---------------------------------------------------------------------------

/**
 * @brief Thrown when the lexer encounters a character it does not recognise.
 *
 * Example trigger: `@`, `#`, `$` in source input.
 *
 * @code
 *   throw UnknownCharException('@', 7);
 *   // what() → "[Line 7] Unknown character: '@' (0x40)"
 * @endcode
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

// ---------------------------------------------------------------------------

/**
 * @brief Thrown when a string literal is opened with `"` but never closed
 *        before the end of the source.
 *
 * @code
 *   throw UnterminatedStringException(3);
 *   // what() → "[Line 3] Unterminated string literal: missing closing '\"'"
 * @endcode
 */
class UnterminatedStringException : public LexerException {
public:
    explicit UnterminatedStringException(int line = 0)
        : LexerException("Unterminated string literal: missing closing '\"'", line) {}
};

// ---------------------------------------------------------------------------

/**
 * @brief Thrown when a numeric literal is malformed.
 *
 * Covers cases such as multiple decimal points (`3.14.15`) or a trailing
 * decimal with no digits following it (`42.`).
 *
 * @code
 *   throw InvalidNumberException("3.14.15", 5);
 *   // what() → "[Line 5] Invalid number literal: '3.14.15'"
 * @endcode
 */
class InvalidNumberException : public LexerException {
public:
    explicit InvalidNumberException(const std::string& literal, int line = 0)
        : LexerException("Invalid number literal: '" + literal + "'", line) {}
};

// ---------------------------------------------------------------------------

/**
 * @brief Thrown when a block comment is opened but never closed.
 *
 * Reserved for future use once comment syntax is added to Prometheus.
 * Currently not thrown by the lexer.
 *
 * @code
 *   throw UnterminatedCommentException(12);
 *   // what() → "[Line 12] Unterminated comment: missing closing marker"
 * @endcode
 */
class UnterminatedCommentException : public LexerException {
public:
    explicit UnterminatedCommentException(int line = 0)
        : LexerException("Unterminated comment: missing closing marker", line) {}
};

// ---------------------------------------------------------------------------

/**
 * @brief Thrown by the Parser when a syntactic rule is violated.
 *
 * @code
 *   throw ParseException("Expected SEMICOLON, got RBRACE");
 * @endcode
 */
class ParseException : public std::runtime_error {
public:
    explicit ParseException(const std::string& msg) : std::runtime_error(msg) {}
};

class ReturnException : public std::exception {
public:
    std::any value;
    explicit ReturnException(std::any val) : value(std::move(val)) {}
    // Override what() just to satisfy the interface, though we won't use it for the value
    const char* what() const noexcept override { return "Return Statement"; }
};

#endif // EXCEPTIONS_H