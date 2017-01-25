# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 17.6cm,7.5cm font '\scriptsize' preamble '\usepackage{times,microtype}'
#set terminal tikz standalone color size 3.3in,5.6in font '\scriptsize' preamble '\usepackage{times,microtype}'
set output "gen-graph-mcs.tex"

set multiplot layout 1,2

set rmargin 5

set xrange [1:1e6]
set yrange [0:5725]

set xlabel "Runtime (ms)"
set ylabel "Number of instances"
set logscale x
set border 3
set grid x y
set xtics nomirror
set ytics nomirror add ('$5725$' 5725)
set key off

set format x '$10^{%T}$'

set title "SIP Instances"

plot \
    "../experiments/gpgnode-results/sip-runtime-james-c.data" u 3:($3>1e6?1e-10:1) smooth cumulative w steps ti "Trimble" at end lc 4, \
    "../experiments/aaai17-final-sip-gpgnode-results/runtime-mcis-fc-induced.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w steps ti "FC" at end lc 2, \
    "../experiments/aaai17-final-sip-gpgnode-results/runtime-clique.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w steps ti "Clique" at end lc 3, \
    "../experiments/aaai17-final-sip-gpgnode-results/runtime-sequentialix-d2-induced.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w steps ti "$k\\downarrow$" at end lc 1

set xrange [1:1e6]
set yrange [0:4110]

set xlabel "Runtime (ms)"
set ylabel "Number of instances"
set logscale x
set border 3
set grid x y
set xtics nomirror
set ytics nomirror add ('' 4000) add ('$4110$' 4110)
set key off

set title "Unlabelled Undirected MCS Instances"

set format x '$10^{%T}$'

plot \
    "../experiments/gpgnode-results/mcs-runtime-james-c.data" u 3:($3>1e6?1e-10:1) smooth cumulative w steps ti "Trimble" at end lc 4, \
    "../experiments/aaai17-final-mcs-gpgnode-results/runtime-cp.data" u ($3*1000):($3>=1e3?1e-10:1) smooth cumulative w steps ti "\\raisebox{-0.5mm}{FC}" at end lc 2, \
    "../experiments/aaai17-final-mcs-gpgnode-results/runtime-sequentialix-d2-induced.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w steps ti "\\raisebox{2mm}{$k\\downarrow$}" at end lc 1, \
    "../experiments/aaai17-final-mcs-gpgnode-results/runtime-clique.data" u 3:($3>=1e6?1e-10:1) smooth cumulative w steps ti "Clique" at end lc 3


