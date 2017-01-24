BUILD_DIR := intermediate
TARGET_DIR := ./

boost_ldlibs := -lboost_regex -lboost_thread -lboost_system -lboost_program_options

override CXXFLAGS += -O3 -march=native -std=c++14 -I./ -W -Wall -g -ggdb3 -pthread
override LDFLAGS += -pthread

TARGET := solve_max_common_subgraph

SOURCES := \
    sequential.cc \
    graph.cc \
    lad.cc \
    dimacs.cc \
    vf.cc \
    solve_max_common_subgraph.cc

TGT_LDLIBS := $(boost_ldlibs)

