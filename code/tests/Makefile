CXX := g++
CXXFLAGS := -O3 -march=native
all: bin/generate

bin/generate: generate.cpp 
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -Wall -std=c++11 -o bin/generate generate.cpp
