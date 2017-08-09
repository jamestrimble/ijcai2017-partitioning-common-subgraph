# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 11in,8in font '\large'
set output "gen-graph-sip-cumulative.tex"

set title "Large instances" offset 0,2,0
set xlabel "Runtime (ms)" offset 0,-5,0
set ylabel "Number of Instances Solved" offset -10,0,0
set border 3
set grid x y
set xtics nomirror offset 0,-2,0
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$\mathsf{10^{%T}}$'
set format y '$\mathsf{%h}$'
set yrange [0:5725]
set ytics add ('$\mathsf{5725}$' 5725)
set key off

plot \
    "../experiments/gpgnode-results/sip/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3 lw 9, \
    "../experiments/gpgnode-results/sip/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti 'CP-FC' at end lc 4 lw 9, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 4:($4>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{-3mm}{k${\downarrow}$}' at end lc 2 lw 9, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{1mm}{\textproc{McSplit}}' at end lc 1 lw 9, \
    "../experiments/gpgnode-results/sip/runtimes.data" u 6:($6>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{2mm}{\textproc{McSplit}${\downarrow}$}' at end lc 7 lw 9
