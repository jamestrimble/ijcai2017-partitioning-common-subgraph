# vim: set et ft=gnuplot sw=4 :

set terminal pdfcairo size 12cm,12cm

set grid
set logscale x
set logscale y
set xtics nomirror
set ytics nomirror

set xlabel "Heuristic: minimise product"
set ylabel "Heuristic: minimise max(leftsize, rightsize)"
set format x '10^%T'
set format y '10^%T'

set output "plots/mcsplain-runtime.pdf"
set title "Plain MCS instances, run time"
plot "../experiments/gpgnode-results/mcsplain/runtimes.data" u 6:8 notitle w points pointtype 7 pointsize .1, x notitle

set output "plots/mcsplain-nodes.pdf"
set title "Plain MCS instances, nodes"
plot "../experiments/gpgnode-results/mcsplain/nodes.data" u 6:8 notitle w points pointtype 7 pointsize .1, x notitle

set output "plots/mcs33ved-runtime.pdf"
set title "33 per cent labelled MCS instances, run time"
plot "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 5:7 notitle w points pointtype 7 pointsize .1, x notitle

set output "plots/mcs33ved-nodes.pdf"
set title "33 per cent labelled MCS instances, nodes"
plot "../experiments/gpgnode-results/mcs33ved/nodes.data" u 5:7 notitle w points pointtype 7 pointsize .1, x notitle


set xlabel "Heuristic: minimise max(leftsize, rightsize), k-asc"
set ylabel "Heuristic: minimise max(leftsize, rightsize), k-desc"

plot "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 7:8 notitle w points pointtype 7 pointsize .1, x notitle


load 'moreland.pal'
set cbrange [0:1]

set output "plots/mcsplain-runtime-kasc-kdesc.pdf"
set title "Plain MCS instances, run time, k-ascending vs k-descending"
plot "kup-vs-kdown-mcsplain.txt" u 4:5:7 w points pointtype 7 pointsize .1 lc palette notitle, x notitle, 2.6*x with line notitle

set output "plots/mcs33ved-runtime-kasc-kdesc.pdf"
set title "33 per cent labelled MCS instances, run time, k-ascending vs k-descending"
plot "kup-vs-kdown-mcs33ved.txt" u 4:5:7 w points pointtype 7 pointsize .1 lc palette notitle, x notitle, 2.6*x with line notitle
