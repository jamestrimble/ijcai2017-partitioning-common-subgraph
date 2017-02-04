all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES =

GRAPHS = \
	 gen-graph-plain-james-versus-cp-fc-nodes-scatter.pdf \
	 gen-graph-33ved-james-versus-cp-fc-nodes-scatter.pdf \
	 gen-graph-plain-cumulative.pdf \
	 gen-graph-33ved-cumulative.pdf \
	 gen-graph-sip-cumulative.pdf \
	 gen-graph-sip-james-versus-kdown-nodes-scatter.pdf

tables : $(TABLES)

graphs : $(GRAPHS)

gen-graph-%.pdf : graph-%.gnuplot
	gnuplot $<
	sed -i -e '19s/^\(\\path.*\)/\% \1/' gen-graph-$*.tex # epic haxx
	latexmk -pdf gen-graph-$*

