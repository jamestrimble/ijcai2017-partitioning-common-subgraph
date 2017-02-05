CXX := g++
CXXFLAGS := -O3 -march=native
all: mcsp

mcsp: mcsp.cc graph.cc graph.hh
	$(CXX) $(CXXFLAGS) -fcilkplus -pthread -Wall -std=c++11 -o mcsp graph.cc mcsp.cc
