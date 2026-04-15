CFLAGS=-std=c++17 -Wall -Wextra -Werror

all:
	g++ src/main.cc -o prometheus $(CFLAGS)
