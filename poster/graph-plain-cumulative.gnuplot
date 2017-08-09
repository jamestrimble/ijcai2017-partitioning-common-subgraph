# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 13in,10in font '\large'
set output "gen-graph-plain-cumulative.tex"

set title "Unlabelled instances" offset 0,2,0
set xlabel "Runtime (ms)" offset 0,-4,0
set ylabel "Number of Instances Solved" offset -10,0,0
set border 7
set grid x y
set xtics nomirror offset 0,-2,0
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$10^{%T}$'
set yrange [0:4110]
set ytics add ('$4110$' 4110) add ('' 4000)
set key off

plot \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 8:($8>=1e6?1e-10:1) smooth cumulative w l ti '\textproc{McSplit}' at end lc 1 lw 4, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{4mm}{$k{\downarrow}$}' at end lc 2 lw 4, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3 lw 4, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti '\raisebox{-4mm}{CP-FC}' at end lc 4 lw 4

