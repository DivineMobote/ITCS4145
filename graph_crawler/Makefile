# Makefile for building bfs_web
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I$(HOME)/rapidjson/include
SRC = src/bfs_web.cpp
BIN = bfs_web

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lcurl

clean:
	rm -f $(BIN)
