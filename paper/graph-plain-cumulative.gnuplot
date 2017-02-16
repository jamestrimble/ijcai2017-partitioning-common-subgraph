# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 7cm,4.6cm font '\scriptsize' preamble '\usepackage{times,microtype,algorithm2e,algpseudocode,amssymb}'
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
set ytics add ('$4110$' 4110) add ('' 4000)
set key off

plot \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 8:($8>=1e6?1e-10:1) smooth cumulative w l ti '\textproc{McSplit}' at end lc 1, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 5:($5>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{2mm}{$k\downarrow$}' at end lc 2, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3, \
    "../experiments/gpgnode-results/mcsplain/runtimes.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w l ti '\raisebox{-0.5mm}{CP-FC}' at end lc 4

