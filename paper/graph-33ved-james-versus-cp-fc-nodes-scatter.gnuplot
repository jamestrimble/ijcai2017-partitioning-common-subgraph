# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 6.5cm,5.5cm font '\scriptsize' preamble '\usepackage{times,microtype,algorithm2e,algpseudocode,amssymb}'
set output "gen-graph-33ved-james-versus-cp-fc-nodes-scatter.tex"

set xlabel 'CP-FC Recursive Calls\vphantom{$\downarrow$}'
set ylabel '\textproc{McSplit} Recursive Calls'
set border 3
set grid x y front
set xtics nomirror
set ytics nomirror
set tics front
set size square
set xrange [1:1e10]
set yrange [1:1e10]
set x2range [0:50]
set y2range [0:50]
set cbrange [1:100]
set logscale x
set logscale y
set format x '$10^{%T}$'
set format y '$10^{%T}$'
set logscale cb

load "magmafromwhite.pal"
set palette negative

plot \
    "mcs33ved-james-versus-cp-fc-heatmap.data" u 2:1:($3+1) matrix w image axes x2y2 notitle, \
    x w l ls 0 notitle

