# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 6.5cm,6.5cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-33ved-james-versus-cp-fc-nodes-scatter.tex"

set xlabel "CP FC Nodes"
set ylabel "Our Nodes"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set size square
set key above
set xrange [1:1e10]
set yrange [1:1e10]
set logscale x
set logscale y
set format x '$10^{%T}$'
set format y '$10^{%T}$'

plot \
    "../experiments/gpgnode-results/mcs33ved/nodes.data" u 3:8 w p ti (columnhead(3) . " vs " . columnhead(7)), \
    x w l ls 0 notitle

