#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <variant>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main (int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Prometheus Requires a File to Run. ./prometheus [filename]" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    // std::cout << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // std::cout << source << std::endl;

    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "--- Tokens Found ---" << std::endl;
        for (const auto& token : tokens) {
            token.print();
        }

        Parser parser(tokens);
        std::vector<std::unique_ptr<ASTNode>> nodes = parser.parse();

        std::cout << "--- Parsed " << nodes.size() << " statement(s) ---" << std::endl;

        Interpreter interpreter(std::move(nodes));
        std::unordered_map<std::string, PrometheusValue> variables = interpreter.interpret();
        std::cout << "--- Final Memory State ---" << std::endl;
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


    } 
    catch (const LexerException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const ParseException& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
}