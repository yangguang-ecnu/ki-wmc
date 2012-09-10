#!/bin/sh

boxR=0.15
boxB=$(echo 1-$boxR|bc)
boxS=0.6

gpCommand="set multiplot
set size square
set title 'ROC Curve'
set xlabel 'False positive rate'
set ylabel 'True positive rate'
set xr [0.0:1.0]
set yr [0.0:1.0]
set xtics 0.1
set ytics 0.1
set mxtics 10
set mytics 10

set arrow from 0,$boxB to $boxR,$boxB lw 1 front nohead
set arrow from $boxR,1 to $boxR,$boxB lw 1 front nohead
set arrow from $boxR,$boxB to $(echo $boxR+0.1|bc),$(echo $boxB-0.1|bc) lw 1 back filled

plot '$1' using 1:2 notitle with lines

set size $boxS,$boxS
set origin 0.2,0.15
set title 'Zoom'
set xrange [0:$boxR]
set yrange [$boxB:1]
set xlabel ''
set ylabel ''
unset arrow
set grid
plot '$1' using 1:2 notitle with lines

unset multiplot"

if [ $# -eq 2 ]
then

gnuplot << EOF
set terminal postscript eps color enhanced
set output '$2'
$gpCommand
EOF

else

gnuplot -persist << EOF
$gpCommand
EOF


fi
