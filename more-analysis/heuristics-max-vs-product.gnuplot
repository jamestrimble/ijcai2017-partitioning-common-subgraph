# vim: set et ft=gnuplot sw=4 :

set terminal pdfcairo size 12cm,12cm

set grid
set logscale
set xtics nomirror
set ytics nomirror

set xlabel "Heuristic: minimise product"
set ylabel "Heuristic: minimise max(leftsize, rightsize)"
set format x '10^%T'
set format y '10^%T'

set output "mcsplain-runtime.pdf"
set title "Plain MCS instances, run time"
plot "../experiments/gpgnode-results/mcsplain/runtimes.data" u 6:8 notitle w points pointtype 7 pointsize .1, x notitle

set output "mcsplain-nodes.pdf"
set title "Plain MCS instances, nodes"
plot "../experiments/gpgnode-results/mcsplain/nodes.data" u 6:8 notitle w points pointtype 7 pointsize .1, x notitle

set output "mcs33ved-runtime.pdf"
set title "33 per cent labelled MCS instances, run time"
plot "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 5:7 notitle w points pointtype 7 pointsize .1, x notitle

set output "mcs33ved-nodes.pdf"
set title "33 per cent labelled MCS instances, nodes"
plot "../experiments/gpgnode-results/mcs33ved/nodes.data" u 5:7 notitle w points pointtype 7 pointsize .1, x notitle

