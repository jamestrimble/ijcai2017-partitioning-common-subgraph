# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 8cm,6cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-sip-james-versus-kdown-nodes-scatter.tex"

set xlabel "KDown Nodes"
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
    "../experiments/gpgnode-results/sip/nodes.data" u 4:6 w p ti (columnhead(4) . " vs " . columnhead(6)), \
    x w l ls 0 notitle

