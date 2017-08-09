all : tables graphs poster.tex
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' poster

TABLES =

GRAPHS = gen-graph-plain-cumulative.tex gen-graph-33ved-cumulative.tex gen-graph-plain-james-versus-cp-fc-nodes-scatter.tex

tables : $(TABLES)

graphs : $(GRAPHS)

gen-graph-%.tex : graph-%.gnuplot
	gnuplot $<
	sed -i -e '4,5s/^\(\\path.*\)/\% \1/' $@ # epic haxx

