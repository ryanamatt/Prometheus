CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC_DIR = src
BIN_DIR = bin

# List of source files 
SOURCES = main.cc lexer.cc parser.cc interpreter.cc
OBJECTS = $(SOURCES:%.cc=$(BIN_DIR)/%.o)

.PHONY: all clean debug visualize

all: prometheus

prometheus: $(BIN_DIR) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(CFLAGS)

test: prometheus
	venv/scripts/pytest

# debug
debug: CFLAGS += -DDEBUG
debug: clean prometheus

# visualize
visualize: CFLAGS += -DVISUALIZE
visualize: $(BIN_DIR) $(OBJECTS) $(BIN_DIR)/dot_visitor.o
	$(CXX) -o prometheus-viz $(OBJECTS) $(BIN_DIR)/dot_visitor.o $(CFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -c -o $@ $< $(CFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) prometheus prometheus-viz