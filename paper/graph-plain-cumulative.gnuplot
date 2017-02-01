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
set yrange [1:4110]
set key off

plot \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti columnhead(2) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti columnhead(3) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($4*1000):($4>=1e3?1e-10:1) smooth cumulative w l ti columnhead(4) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti columnhead(5) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 6:($6>=1e6?1e-10:1) smooth cumulative w l ti columnhead(6) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 7:($7>=1e6?1e-10:1) smooth cumulative w l ti columnhead(7) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 8:($8>=1e6?1e-10:1) smooth cumulative w l ti columnhead(8) at end, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 9:($9>=1e6?1e-10:1) smooth cumulative w l ti columnhead(9) at end

