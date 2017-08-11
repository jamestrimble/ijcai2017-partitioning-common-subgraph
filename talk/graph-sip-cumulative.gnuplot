# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 9cm,7cm font '\large'
set output "gen-graph-sip-cumulative.tex"

set xlabel "Runtime (ms)"
set ylabel "Number of Instances Solved"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$\mathsf{10^{%T}}$'
set format y '$\mathsf{%h}$'
set yrange [0:5725]
set ytics add ('$\mathsf{5725}$' 5725)
set key off

plot \
    "../experiments/gpgnode-results/sip/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3 lw 3, \
    "../experiments/gpgnode-results/sip/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti '\raisebox{-.4mm}{CP-FC}' at end lc 4 lw 3, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 4:($4>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{-1mm}{k${\downarrow}$}' at end lc 2 lw 3, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{.4mm}{McSplit}' at end lc 1 lw 3, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 6:($6>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{1mm}{McSplit${\downarrow}$}' at end lc 7 lw 3
