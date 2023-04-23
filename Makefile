CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: serverM serverA serverB client

serverM: serverM.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

serverA: serverA.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

serverB: serverB.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

client: client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f serverM serverA serverB client
