CXX      := g++
CXXFLAGS := -O3 -std=c++17 -Wall -Wextra -march=native

all: nbody

nbody: nbody.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f nbody
