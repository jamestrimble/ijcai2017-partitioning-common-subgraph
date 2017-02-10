# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 8cm,6cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-plain-cumulative.tex"

set xlabel "Runtime (ms)"
set ylabel "Number of Instances Solved"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$10^{%T}$'
set yrange [0:4110]
set ytics add ('4110' 4110) add ('' 4000)
set key off

plot \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti columnhead(2) at end lc 1, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti columnhead(3) at end lc 2, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti columnhead(5) at end lc 4, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 8:($8>=1e6?1e-10:1) smooth cumulative w l ti columnhead(8) at end lc 3, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 11:($11>=1e6?1e-10:1) smooth cumulative w l ti columnhead(11) at end lc 1 dt '.', \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 13:($13>=1e6?1e-10:1) smooth cumulative w l ti columnhead(13) at end lc 3 dt '.'

