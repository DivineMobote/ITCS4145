CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra -I$(HOME)/rapidjson/include
LDFLAGS = -lcurl -pthread

PAR = bfs_par
SEQ = bfs_seq

all: $(PAR) $(SEQ)

$(PAR): client_par.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(SEQ): client_seq.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(PAR) $(SEQ)
