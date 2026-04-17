CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC = src/main.cc src/lexer.cc src/parser.cc
OBJ = $(SRC:.cc=.o)

prometheus: $(OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

%.o: %.cc
	$(CXX) -c -o $@ $< $(CFLAGS)