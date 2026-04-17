CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC = src/main.cc src/lexer.cc src/parser.cc src/interpreter.cc
OBJ = $(SRC:.cc=.o)

prometheus: $(OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

debug: CFLAGS += -DDEBUG
debug: clean prometheus

%.o: %.cc
	$(CXX) -c -o $@ $< $(CFLAGS)

clean:
	rm -f src/*.o prometheus debug