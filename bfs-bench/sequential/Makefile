CXX      := g++
CXXFLAGS := -O2 -std=c++17 -pthread -I$(HOME)/rapidjson/include
LDFLAGS  := -lcurl

all: level_client par_level_client

level_client: level_client.o
	$(CXX) $(CXXFLAGS) level_client.o -o $@ $(LDFLAGS)

par_level_client: par_level_client.o
	$(CXX) $(CXXFLAGS) par_level_client.o -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o level_client par_level_client
