all : tables graphs poster.tex
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' poster

TABLES =

GRAPHS = gen-graph-plain-cumulative.tex
#GRAPHS = gen-graph-phase-transition.tex gen-graph-non-induced.tex gen-graph-induced.tex

tables : $(TABLES)

graphs : $(GRAPHS)

gen-graph-%.tex : graph-%.gnuplot
	gnuplot $<
	sed -i -e '4,5s/^\(\\path.*\)/\% \1/' $@ # epic haxx

