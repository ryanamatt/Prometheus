CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC = src/main.cc src/lexer.cc src/parser.cc src/interpreter.cc
VIZ_SRC = src/main.cc src/lexer.cc src/parser.cc src/dot_visitor.cc
OBJ = $(SRC:.cc=.o)

prometheus: $(OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

debug: CFLAGS += -DDEBUG
debug: clean prometheus

visualize: CFLAGS += -DVISUALIZE
visualize: $(VIZ_SRC)
	$(CXX) -o prometheus-viz $^ $(CFLAGS)

%.o: %.cc
	$(CXX) -c -o $@ $< $(CFLAGS)

clean:
	rm -f src/*.o prometheus prometheus-viz debug