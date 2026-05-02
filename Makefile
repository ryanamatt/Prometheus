CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -I include
SRC_DIR = src
BIN_DIR = bin

# List of source files 
SOURCES = main.cc lexer.cc parser.cc interpreter.cc builtins.cc stdlib/math.cc stdlib/random.cc stdlib/time.cc
OBJECTS = $(SOURCES:%.cc=$(BIN_DIR)/%.o)

ifeq ($(OS), Windows_NT)
	CFLAGS += -static -static-libgcc -static-libstdc++
	TARGET = prometheus.exe
	TARGET_VIZ = prometheus_viz.exe
    PREFIX = C:/ProgramData/prometheus
    INSTALL_BIN = $(PREFIX)
	PYTEST = venv/scripts/pytest
else
	TARGET = prometheus
	TARGET_VIZ = prometheus_viz
    PREFIX = /usr/local
    INSTALL_BIN = $(PREFIX)/bin
	PYTEST = venv/bin/pytest
endif

.PHONY: all clean debug visualize

all: prometheus

prometheus: $(BIN_DIR) $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

test: clean prometheus
	$(PYTEST)

# debug
debug: CFLAGS += -DDEBUG
debug: clean prometheus

# visualize
visualize: CFLAGS += -DVISUALIZE
visualize: $(BIN_DIR) $(OBJECTS) $(BIN_DIR)/dot_visitor.o
	$(CXX) -o $(TARGET_VIZ) $(OBJECTS) $(BIN_DIR)/dot_visitor.o $(CFLAGS)


install: $(TARGET)
	@echo "Installing to $(PREFIX)..."
ifeq ($(OS), Windows_NT)
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/
	cp -r stdlib $(PREFIX)/
	@echo "Please add $(PREFIX) to your PATH environment variable."
else
	install -d $(INSTALL_BIN)
	install -m 755 $(TARGET) $(INSTALL_BIN)
	install -d $(PREFIX)/share/prometheus/stdlib
	install -m 644 stdlib/*.prm $(PREFIX)/share/prometheus/stdlib/
endif

uninstall:
ifeq ($(OS), Windows_NT)
	@echo "Removing Prometheus from $(PREFIX)..."
	rm -rf $(PREFIX)
	@echo "Note: You must manually remove $(PREFIX) from your PATH environment variable."
else
	@echo "Removing Prometheus from $(PREFIX)..."
	rm -f $(INSTALL_BIN)/$(TARGET)
	rm -rf $(PREFIX)/share/prometheus
	@echo "Uninstall complete."
endif

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -c -o $@ $< $(CFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
	mkdir -p $(BIN_DIR)/stdlib

clean:
	rm -rf $(BIN_DIR) prometheus prometheus-viz