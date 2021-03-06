SHELL := /bin/bash
FILTER := -1
MODULUS := 1
CATTER := cat
INSTANCES_FILE := sipinstances.txt
INSTANCES := $(shell cut -d' ' -f1 $(INSTANCES_FILE) | if [[ $(FILTER) == -1 ]] ; then cat ; else awk "(NR % $(MODULUS)) == ($(FILTER) % $(MODULUS))" ; fi | $(CATTER) )
TIMEOUT := 1000
ALGORITHMS := clique cp-fc kdown james-cpp-max james-cpp-max-down
STACK_SPACE := 10485760
RESULTS := gpgnode-results/sip

all : $(foreach i, $(INSTANCES), $(foreach a, $(ALGORITHMS), $(RESULTS)/$(a)/$i.out ))

$(RESULTS)/clique/%.out :
	mkdir -p $(RESULTS)/clique
	ulimit -s $(STACK_SPACE) ; ../clique-code/solve_max_common_subgraph --undirected --unlabelled --format lad --timeout $(TIMEOUT) $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1

$(RESULTS)/kdown/%.out :
	mkdir -p $(RESULTS)/kdown
	ulimit -s $(STACK_SPACE) ; ../kdown-code/solve_subgraph_isomorphism sequentialix --format lad --induced --timeout $(TIMEOUT) $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1

$(RESULTS)/cp-fc/%.out :
	mkdir -p $(RESULTS)/cp-fc
	ulimit -s $(STACK_SPACE) ; ../french-code/mcis_fc/executable.out -d 0 -l 0 -v 0 -p $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2 ) -t $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f3 ) > >(tee $@ ) 2>&1

$(RESULTS)/james-cpp-product/%.out :
	mkdir -p $(RESULTS)/james-cpp-product
	ulimit -s $(STACK_SPACE) ; ../../partitioning-common-subgraph/code/james-cpp/mcsp -l --timeout $(TIMEOUT) min_product $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1

$(RESULTS)/james-cpp-product-down/%.out :
	mkdir -p $(RESULTS)/james-cpp-product-down
	ulimit -s $(STACK_SPACE) ; ../../partitioning-common-subgraph/code/james-cpp/mcsp -l --timeout $(TIMEOUT) min_product --big-first $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1

$(RESULTS)/james-cpp-max/%.out :
	mkdir -p $(RESULTS)/james-cpp-max
	ulimit -s $(STACK_SPACE) ; ../../partitioning-common-subgraph/code/james-cpp/mcsp -l --timeout $(TIMEOUT) min_max $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1

$(RESULTS)/james-cpp-max-down/%.out :
	mkdir -p $(RESULTS)/james-cpp-max-down
	ulimit -s $(STACK_SPACE) ; ../../partitioning-common-subgraph/code/james-cpp/mcsp -l --timeout $(TIMEOUT) min_max --big-first $(shell grep "^`basename $*` " < $(INSTANCES_FILE) | cut -d' ' -f2-3 ) > >(tee $@ ) 2>&1
