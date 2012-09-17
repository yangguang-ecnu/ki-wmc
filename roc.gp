#!/usr/bin/env gnuplot

set size square
set title 'ROC Curve'
set xlabel 'False positive rate'
set ylabel 'True positive rate'
set xr [0.0:1.0]
set yr [0.0:1.0]
plot 'roc.csv' using 1:2 notitle with lines
