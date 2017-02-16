# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 7cm,5cm font '\scriptsize' preamble '\usepackage{times,microtype,algorithm2e,algpseudocode,amssymb}'
set output "gen-graph-plain-connected-cumulative.tex"

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
    "../experiments/gpgnode-results/mcsplain-connected/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti 'clique' at end lc 3, \
    "../experiments/gpgnode-results/mcsplain-connected/runtimes.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w l ti '\textproc{McSplit}' at end lc 1, \
    "../experiments/gpgnode-results/mcsplain-connected/runtimes.data" u ($6*1000):($6>=1e3?1e-10:1) smooth cumulative w l ti 'CP-FC' at end lc 4

