GNUPLOT := gnuplot

RUNTIMEPLOTS := plots/mcs33ved-nodes.pdf plots/mcs33ved-runtime.pdf plots/mcsplain-nodes.pdf plots/mcsplain-runtime.pdf plots/mcsplain-runtime-kasc-kdesc.pdf plots/mcs33ved-runtime-kasc-kdesc.pdf

all: ${RUNTIMEPLOTS} 

${RUNTIMEPLOTS}: runtime-comparisons.gnuplot kup-vs-kdown-mcsplain.txt kup-vs-kdown-mcs33ved.txt
	mkdir -p plots
	gnuplot runtime-comparisons.gnuplot

kup-vs-kdown-mcsplain.txt:
	python kup-vs-kdown.py mcsplain 7 8 > kup-vs-kdown-mcsplain.txt

kup-vs-kdown-mcs33ved.txt:
	python kup-vs-kdown.py mcs33ved 6 7 > kup-vs-kdown-mcs33ved.txt

clean:
	rm plots/*.pdf
	rm kup-vs-kdown*.txt
