all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES =

GRAPHS = \
	 gen-graph-plain-james-versus-cp-fc-nodes-scatter.pdf \
	 gen-graph-33ved-james-versus-cp-fc-nodes-scatter.pdf \
	 gen-graph-plain-cumulative.pdf \
	 gen-graph-33ved-cumulative.pdf \
	 gen-graph-33ve-cumulative.pdf \
	 gen-graph-33v-cumulative.pdf \
	 gen-graph-33ve-connected-cumulative.pdf \
	 gen-graph-plain-connected-cumulative.pdf \
	 gen-graph-sip-cumulative.pdf \
	 gen-graph-sip-james-versus-kdown-nodes-scatter.pdf

ADJUSTED_HEATMAP_DATA = \
	mcs33ved-james-versus-cp-fc-heatmap.data.adjusted \
	mcsplain-james-versus-cp-fc-heatmap.data.adjusted \
	mcsplain-james-versus-cp-fc-runtimes-heatmap.data.adjusted \
	sip-james-versus-kdown-heatmap.data.adjusted

.SECONDARY: $(ADJUSTED_HEATMAP_DATA)

tables : $(TABLES)

graphs : $(GRAPHS)

gen-graph-%.pdf : graph-%.gnuplot $(ADJUSTED_HEATMAP_DATA)
	gnuplot $<
	sed -i -e '19s/^\(\\path.*\)/\% \1/' gen-graph-$*.tex # epic haxx
	latexmk -pdf gen-graph-$*

%-heatmap.data.adjusted: %-heatmap.data
	sed 's/\b1\b/1.03/g' $< > $@

clean :
	rm *.aux *.bbl *.blg *.fdb_latexmk *.fls *.log *.pdf *.data.adjusted gen-graph*
