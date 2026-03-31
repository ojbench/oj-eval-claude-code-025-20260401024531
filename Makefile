CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

all: cheat anticheat cheat-submit.cpp anticheat-submit.cpp code

cheat: cheat.cpp eval.h
	$(CXX) $(CXXFLAGS) -o cheat cheat.cpp

anticheat: anticheat.cpp eval.h
	$(CXX) $(CXXFLAGS) -o anticheat anticheat.cpp

# Create submit versions by concatenating header with implementation
cheat-submit.cpp: eval.h cheat.cpp
	@echo "// Auto-generated file - DO NOT EDIT" > cheat-submit.cpp
	@cat eval.h | grep -v '^#ifndef EVAL_H' | grep -v '^#define EVAL_H' | grep -v '^#endif // EVAL_H' >> cheat-submit.cpp
	@echo "" >> cheat-submit.cpp
	@cat cheat.cpp | grep -v '#include "eval.h"' >> cheat-submit.cpp

anticheat-submit.cpp: eval.h anticheat.cpp
	@echo "// Auto-generated file - DO NOT EDIT" > anticheat-submit.cpp
	@cat eval.h | grep -v '^#ifndef EVAL_H' | grep -v '^#define EVAL_H' | grep -v '^#endif // EVAL_H' >> anticheat-submit.cpp
	@echo "" >> anticheat-submit.cpp
	@cat anticheat.cpp | grep -v '#include "eval.h"' >> anticheat-submit.cpp

# Build the final executable for OJ (combined version)
code: main.cpp eval.h
	$(CXX) $(CXXFLAGS) -o code main.cpp

clean:
	rm -f cheat anticheat code cheat-submit.cpp anticheat-submit.cpp

.PHONY: all clean
