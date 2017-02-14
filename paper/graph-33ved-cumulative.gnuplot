# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 7cm,5cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-33ved-cumulative.tex"

set xlabel "Runtime (ms)"
set ylabel "Number of Instances Solved"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$10^{%T}$'
set yrange [0:8140]
set key off
set ytics add ('8140' 8140) add ('' 8000)

plot \
    "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti columnhead(2) at end lc 1, \
    "../experiments/gpgnode-results/mcs33ved/runtimes.data" u ($4*1000):($4>=1e3?1e-10:1) smooth cumulative w l ti columnhead(4) at end lc 2, \
    "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 7:($7>=1e6?1e-10:1) smooth cumulative w l ti columnhead(7) at end lc 3, \
    "../experiments/gpgnode-results/mcs33ved/runtimes.data" u 10:($10>=1e6?1e-10:1) smooth cumulative w l ti columnhead(10) at end lc 1 dt '.'

