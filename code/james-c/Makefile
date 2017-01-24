all: mcsp mcsp_labelled

mcsp: mcsp.c graph.c graph.h
	gcc -O3 -march=native -Wall -std=c11 -o mcsp graph.c mcsp.c

mcsp_labelled: mcsp.c graph.c graph.h
	gcc -O3 -march=native -Wall -std=c11 -DLABELLED -o mcsp_labelled graph.c mcsp.c
