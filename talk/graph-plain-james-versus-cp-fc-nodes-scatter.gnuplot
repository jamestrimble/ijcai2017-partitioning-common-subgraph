# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 9cm,9cm font '\large'
set output "gen-graph-plain-james-versus-cp-fc-nodes-scatter.tex"

set xlabel 'CP-FC Recursive Calls\vphantom{${\downarrow}$}'
set ylabel 'McSplit Recursive Calls'
set border 3
set grid x y front
set xtics nomirror
set ytics nomirror
set tics front
set size square
set xrange [1:1e10]
set yrange [1:1e10]
set x2range [-0.5:50.5]
set y2range [-0.5:50.5]
set cbrange [1:100]
set cbtics out nomirror offset character -1
set logscale x
set logscale y
set format x '$\mathsf{10^{%T}}$'
set format y '$\mathsf{10^{%T}}$'
set format cb '$\mathsf{%h}$'
set logscale cb

load "../paper/magmafromwhite-adjusted-for-heatmaps.pal"
set palette negative

plot \
    "../paper/mcsplain-james-versus-cp-fc-heatmap.data.adjusted" u 2:1:3 matrix w image axes x2y2 notitle, \
    x w l ls 0 notitle

