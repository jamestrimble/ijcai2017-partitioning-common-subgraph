GNUPLOT := gnuplot

PLOTS := plots/mcs33ved-nodes.pdf plots/mcs33ved-runtime.pdf plots/mcsplain-nodes.pdf plots/mcsplain-runtime.pdf

all: ${PLOTS}

${PLOTS}: heuristics-max-vs-product.gnuplot
	mkdir -p plots
	gnuplot heuristics-max-vs-product.gnuplot
