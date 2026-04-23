CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC = src/main.cc src/lexer.cc src/parser.cc src/interpreter.cc
VIZ_SRC = src/main.cc src/lexer.cc src/parser.cc src/interpreter.cc src/dot_visitor.cc
OBJ = $(patsubst src/%.cc, bin/%.o, $(SRC))

prometheus: bin $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CFLAGS)

debug: CFLAGS += -DDEBUG
debug: clean prometheus

visualize: CFLAGS += -DVISUALIZE
visualize: $(VIZ_SRC)
	$(CXX) -o prometheus-viz $^ $(CFLAGS)

bin:
	mkdir -p bin

bin/%.o: src/%.cc
	$(CXX) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf bin/ prometheus prometheus-viz debug