# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 7cm,4.6cm font '\scriptsize' preamble '\usepackage{times,microtype,algorithm2e,algpseudocode,amssymb}'
set output "gen-graph-33ve-connected-cumulative.tex"

set multiplot

set arrow from 1e5, 7140 to screen 0.7, screen 0.6 lw 1 back filled

set arrow from 1e4,7140 to 1e6,7140 front nohead
set arrow from 1e4,8140 to 1e6,8140 front nohead
set arrow from 1e4,7140 to 1e4,8140 front nohead
set arrow from 1e6,7140 to 1e6,8140 front nohead

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
set ytics add ('$8140$' 8140) add ('' 8000)

plot \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l notitle lc 3, \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w l notitle lc 1, \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u ($7*1000):($7>=1e3?1e-10:1) smooth cumulative w l notitle lc 4

set size 0.3, 0.3
set origin 0.55, 0.3
set bmargin 0; set tmargin 0; set lmargin 0; set rmargin 0
unset arrow
set border 15
clear

set nokey
set xrange [1e4:1e6]
set yrange [7140:8140]
set xlabel ""
set ylabel ""
set y2label ""
unset xtics
unset ytics
unset y2tics
unset grid

plot \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u 2:($2>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{1mm}{clique}' at end lc 3, \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w l ti '\raisebox{-1mm}{\textproc{McSplit}}' at end lc 1, \
    "../experiments/gpgnode-results/mcs33ve-connected/runtimes.data" u ($7*1000):($7>=1e3?1e-10:1) smooth cumulative w l ti '\raisebox{0mm}{CP-MAC}' at end lc 4
