# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 9cm,7cm font '\large'
set output "gen-graph-plain-cumulative.tex"

set xlabel "Runtime (ms)" #offset 0,-5,0
set ylabel "Number of Instances Solved" #offset -10,0,0
set border 3
set grid x y
set xtics nomirror #offset 0,-2,0
set ytics nomirror
set xrange [1:1e6]
set logscale x
set format x '$\mathsf{10^{%T}}$'
set format y '$\mathsf{%h}$'
set yrange [0:4110]
set ytics add ('$\mathsf{4110}$' 4110) add ('' 4000)
set key off

plot \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 8:($8>=1e6?1e-10:1) smooth cumulative w l ti 'McSplit' at end lc 1 lw 3, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{1.7mm}{$k{\downarrow}$}' at end lc 2 lw 3, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3 lw 3, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti '\raisebox{-1mm}{CP-FC}' at end lc 4 lw 3

