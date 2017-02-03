# vim: set et ft=gnuplot sw=4 :

set terminal pdfcairo size 12cm,12cm

set grid
set logscale y
set xtics nomirror
set ytics nomirror

set xlabel "Solution size divided by pattern graph order"
set ylabel "k-ascending run time divided by k-descending runtime"
#set format x '10^%T'
#set format y '10^%T'

set output "plots/kup-vs-kdown.pdf"
set title ""
plot "kup-vs-kdown.txt" u 8:6 notitle w points pointtype 7 pointsize .1

