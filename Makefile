CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

all: code

# Build the final executable for OJ
code: main.cpp eval.h
	$(CXX) $(CXXFLAGS) -o code main.cpp

clean:
	rm -f code

.PHONY: all clean
