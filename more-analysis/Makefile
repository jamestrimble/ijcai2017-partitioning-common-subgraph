GNUPLOT := gnuplot

RUNTIMEPLOTS := plots/mcs33ved-nodes.pdf plots/mcs33ved-runtime.pdf plots/mcsplain-nodes.pdf plots/mcsplain-runtime.pdf plots/mcsplain-runtime-kasc-kdesc.pdf plots/mcs33ved-runtime-kasc-kdesc.pdf

all: ${RUNTIMEPLOTS} 

${RUNTIMEPLOTS}: runtime-comparisons.gnuplot kup-vs-kdown.txt
	mkdir -p plots
	gnuplot runtime-comparisons.gnuplot

kup-vs-kdown.txt:
	python kup-vs-kdown.py > kup-vs-kdown.txt
