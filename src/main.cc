#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <variant>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "exceptions.h"

#ifdef VISUALIZE
#include "dot_visitor.h"
#endif

void runREPL() {
    std::string line;

    std::unordered_map<std::string, PrometheusValue> globalVariables;
    // In interactive (REPL) mode there is no source file, so use sentinels
    // that match Python's own behaviour for the interactive session.
    globalVariables["__name__"] = std::string("__repl__");
    globalVariables["__file__"] = std::string("<repl>");
    globalVariables["__dir__"]  = std::string("");

    std::cout << "Prometheus REPL — type 'exit' to quit." << std::endl << std::endl;

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line) || line == "exit") break;
        if (line.empty()) continue;

        try {
            Lexer lexer(line);
            std::vector<Token> tokens = lexer.tokenize();

            Parser parser(tokens);
            std::vector<std::unique_ptr<ASTNode>> nodes = parser.parse();

            Interpreter interpreter(nodes, globalVariables, "");
            globalVariables = interpreter.interpret();
        }

        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main (int argc, char* argv[])
{
    if (argc < 2) {
        runREPL();
        return 0;
    }

    std::string first_arg = argv[1];

    if (first_arg == "--version" || first_arg == "-v") {
        std::cout << PROMETHEUS_VERSION << std::endl;
        return 0;
    }

    std::string filename = first_arg;
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
        // Visualize (skips interpretation entirely)
        // ----------------------------------------------------------------
#ifdef VISUALIZE
        // Derive output path: replace source extension with .dot
        std::string dot_path = filename;
        auto dot_pos = dot_path.rfind('.');
        if (dot_pos != std::string::npos)
            dot_path = dot_path.substr(0, dot_pos);
        dot_path += ".dot";

        std::ofstream dot_file(dot_path);
        if (!dot_file.is_open()) {
            std::cerr << "Error: Could not open output file '" << dot_path << "'" << std::endl;
            return 1;
        }

        DOTVisitor visitor(dot_file);
        visitor.generate(nodes);
        dot_file.close();

        std::cout << "AST written to: " << dot_path << std::endl;
        std::cout << "Render with:    dot -Tpng " << dot_path
                  << " -o ast.png" << std::endl;
        return 0;
#endif

        // ----------------------------------------------------------------
        // Interpret
        // ----------------------------------------------------------------
        std::string base_dir = std::filesystem::path(filename).parent_path().string();

        // Resolve the script to an absolute path so __file__ is always
        // unambiguous, even when the user supplies a relative path.
        std::string abs_file = std::filesystem::weakly_canonical(filename).string();

        std::unordered_map<std::string, PrometheusValue> initial_vars;
        initial_vars["__name__"] = std::string("__main__");
        initial_vars["__file__"] = abs_file;
        initial_vars["__dir__"]  = base_dir;

        Interpreter interpreter(nodes, std::move(initial_vars), base_dir);
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