GNUPLOT := gnuplot

RUNTIMEPLOTS := plots/mcs33ved-nodes.pdf plots/mcs33ved-runtime.pdf plots/mcsplain-nodes.pdf plots/mcsplain-runtime.pdf plots/mcsplain-runtime-kasc-kdesc.pdf plots/mcs33ved-runtime-kasc-kdesc.pdf

all: ${RUNTIMEPLOTS} plots/kup-vs-kdown.pdf

${RUNTIMEPLOTS}: runtime-comparisons.gnuplot
	mkdir -p plots
	gnuplot runtime-comparisons.gnuplot

plots/kup-vs-kdown.pdf: kup-vs-kdown.gnuplot kup-vs-kdown.txt
	gnuplot kup-vs-kdown.gnuplot

kup-vs-kdown.txt:
	python kup-vs-kdown.py > kup-vs-kdown.txt
