# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 6.5cm,5.5cm font '\scriptsize' preamble '\usepackage{times,microtype,algorithm2e,algpseudocode,amssymb}'
set output "gen-graph-sip-james-versus-kdown-nodes-scatter.tex"

set xlabel '$k\downarrow$ Recursive Calls'
set ylabel '\textproc{McSplit$\downarrow$} Recursive Calls'
set border 3
set grid x y
set xtics nomirror
set ytics nomirror
set size square
set xrange [1:1e10]
set yrange [1:1e10]
set logscale x
set logscale y
set format x '$10^{%T}$'
set format y '$10^{%T}$'

plot \
    "../experiments/gpgnode-results/sip/nodes.data" u 4:6 w p notitle, \
    x w l ls 0 notitle

