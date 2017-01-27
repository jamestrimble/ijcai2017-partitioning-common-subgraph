all: mcsp

mcsp: mcsp.c graph.c graph.h
	g++ -fcilkplus -pthread -O3 -march=native -Wall -std=c++11 -o mcsp graph.c mcsp.c
