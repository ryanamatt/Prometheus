#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <variant>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "exceptions.h"

int main (int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Prometheus Requires a File to Run. ./prometheus [filename]" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
#ifdef DEBUG
    std::cout << "Filename: " << filename << std::endl;
#endif

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

#ifdef DEBUG
    std::cout << "Source:\n" << source << std::endl;
#endif

    try {
        // ----------------------------------------------------------------
        // Lex
        // ----------------------------------------------------------------
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

#ifdef DEBUG
        std::cout << "--- Tokens Found ---" << std::endl;
        for (const auto& token : tokens) {
            token.print();
        }
#endif

        // ----------------------------------------------------------------
        // Parse
        // ----------------------------------------------------------------
        Parser parser(tokens);
        std::vector<std::unique_ptr<ASTNode>> nodes = parser.parse();

#ifdef DEBUG
        std::cout << "--- Parsed " << nodes.size() << " statement(s) ---" << std::endl;
        std::cout << "Now Interpreting Program\n" << std::endl;
#endif

        // ----------------------------------------------------------------
        // Interpret
        // ----------------------------------------------------------------
        Interpreter interpreter(std::move(nodes));
        std::unordered_map<std::string, PrometheusValue> variables = interpreter.interpret();

#ifdef DEBUG
        std::cout << "\n--- Final Memory State ---" << std::endl;
        for (auto const& [name, val] : variables) {
            std::cout << name << " = ";
            std::visit([](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    std::cout << "None";
                } else {
                    std::cout << v;
                }
            }, val);
            std::cout << std::endl;
        }
#endif
    }

    // ----------------------------------------------------------------
    // Lexer errors
    // ----------------------------------------------------------------
    catch (const UnterminatedStringException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const InvalidNumberException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const UnknownCharException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const LexerException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Parse errors
    // ----------------------------------------------------------------
    catch (const MissingBraceException& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const MissingSemicolonException& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const DuplicateParamException& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const ParseException& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Runtime errors — most specific first
    // ----------------------------------------------------------------
    catch (const DivisionByZeroException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const UndefinedVariableException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const UndefinedFunctionException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const ArgumentCountException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const TypeMismatchException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const ConversionException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const OperatorException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const TypeException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const RuntimeException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Catch-all (should never be reached in normal operation)
    // ----------------------------------------------------------------
    catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}