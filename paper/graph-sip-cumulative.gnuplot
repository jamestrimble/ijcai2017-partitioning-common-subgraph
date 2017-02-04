# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 8cm,6cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-sip-cumulative.tex"

set xlabel "Runtime (ms)"
set ylabel "Number of Instances Solved"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$10^{%T}$'
set yrange [0:5725]
set ytics add ('5725' 5725)
set key off

plot \
    "../experiments/gpgnode-results/sip/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti columnhead(2) at end lc 1, \
    "../experiments/gpgnode-results/sip/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti columnhead(3) at end lc 2, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 4:($4>=1e6?1e-10:1) smooth cumulative w l ti columnhead(4) at end lc 3, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti columnhead(5) at end lc 4, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 6:($6>=1e6?1e-10:1) smooth cumulative w l ti columnhead(6) at end lc 5, \
