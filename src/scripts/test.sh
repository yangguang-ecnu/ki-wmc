#!/bin/bash

function progress_bar
{ 
  cols=$(tput cols)
  # will be like this:  [========                  ] xxx%
  bar_cols=$[$cols-8]
  #draw the empty progress bar
  curpos=$[($1+1)*$bar_cols/$2]
  remcol=$[$bar_cols-$curpos]
  echo -ne "\r["
  printf "%${curpos}s"|tr ' ' '='
  printf "%${remcol}s"
  echo -n "]" $[($1+1)*100/$2]%
}

max=1300

for ((i=0;i<=$max;i+=3));
do
    progress_bar $i $max
    sleep 0.005
done
