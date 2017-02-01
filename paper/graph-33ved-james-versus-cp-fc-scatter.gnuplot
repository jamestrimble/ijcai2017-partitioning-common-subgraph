# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 8cm,6cm font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-33ved-james-versus-cp-fc-scatter.tex"

set xlabel "CP FC Runtime (ms)"
set ylabel "Our Runtime (ms)"
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set size square
set key above
set xrange [1:1e6]
set yrange [1:1e6]
set logscale x
set logscale y
set format x '$10^{%T}$'
set format y '$10^{%T}$'

plot \
    "../experiments/gpgnode-results/mcs33ved/runtimes.data" u ($3*1000):8 w p ti (columnhead(3) . " vs " . columnhead(7)), \
    x w l ls 0 notitle

