CXX := g++
CXXFLAGS := -O3 -march=native
all: mcsp

mcsp: mcsp.cc graph.cc graph.hh
	$(CXX) $(CXXFLAGS) -Wall -std=c++11 -o mcsp graph.cc mcsp.cc -pthread -g -ggdb3
