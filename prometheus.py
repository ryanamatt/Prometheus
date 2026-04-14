import sys
from lexer import Lexer
from parser import Parser
from interpreter import Interpreter

def main():
    if len(sys.argv) < 2:
        print("Prometheus Requires a File to Run. python prometheus.py [filename]")
        sys.exit()
    
    filename: str = sys.argv[1]

    lexer: Lexer = Lexer(filename=filename)
    tokens = lexer.tokenize()

    parser: Parser = Parser(tokens)
    nodes = parser.parse()

    interpreter: Interpreter = Interpreter()
    interpreter.interpret(nodes)

    print("Final Memory State:", interpreter.variables)

if __name__ == "__main__":
    main()