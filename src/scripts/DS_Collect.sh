#!/bin/bash          

COPYRIGHT="Dataset feature vector collector. Copyright (c) 2012 Soheil Damangir"
EXNAME=$(basename $0)
USAGE="
Usage: $EXNAME [OPTIONS] SEQ [SEQ]...\n
Collects feature vectors for SEQs using options\n
\n
Options:\n
   -b           Base directory for sequences. Default: current directory\n
   -e           Collector executive file\n
   -f           Flags to be passed to collector\n
   -v           Verbos flag on\n
\n
Copyright (C) 2012 Soheil Damangir\n
Report $EXNAME bugs to soheil.damangir@ki.se\n
"

COLLECTFV="wmlCollectFV"
FLAGS=
BASEDIR="."
VERBOSE=
while getopts "hb:e:f:v" OPTION
do
  case $OPTION in
    h) echo -e $USAGE;exit 0;;
    b) BASEDIR=$OPTARG;;
    e) COLLECTFV=$OPTARG;;
    f) FLAGS=$OPTARG;;
    v) VERBOSE=TRUE;;
    :) echo "Option -$OPTARG requires an argument." >&2;exit 1;;
    ?) echo -e $USAGE;exit 1;;
  esac
done

SEQs=${@:$OPTIND}
echo $COPYRIGHT
if [ $VERBOSE ];
then
  echo -OPTION------VALUE-------------------
  echo "BASEDIR    '$BASEDIR'"
  echo "FLAGS      '$FLAGS'"
  echo "COLLECTFV  '$COLLECTFV'"
  echo "SEQs       '$SEQs'"
  echo -------------------------------------
fi

if [[ ! -d $BASEDIR ]];
then
  echo "The base directory '${BASEDIR}' does not exist." >&2
  exit 1
fi

if [[ ! -e $COLLECTFV ]];
then
  echo "The feature collector command does not exist or is not executable." >&2
  exit 1
fi

if [[ ! SEQs ]];
then
  echo "No sequence pattern given." >&2
  exit 1
fi

progress_bar()
{
  cols=$(tput cols)
  # will be like this:  [========                  ] xxx%
  bar_cols=$[$cols-8]
  curpos=$[($1+1)*$bar_cols/$2]
  remcol=$[$bar_cols-$curpos]
  echo -ne "\r["
  printf "%${curpos}s"|tr ' ' '='
  printf "%${remcol}s"
  echo -n "]" $[($1+1)*100/$2]%
}

runner(){
args=($@)
first=($BASEDIR/*${args[0]}*)
total=${#first[@]}
for i in "${!first[@]}";
do
  cmd="${COLLECTFV} ${FLAGS}"
  for s in "${!args[@]}";
  do
    lst=($BASEDIR/*${args[$s]}*)
    cmd="${cmd} -i ${lst[$i]}"
  done
  out=${first[$i]%%.*};
  ext=${first[$i]#*.};
  for s in "${args[@]:1}"
  do
    out="${out}_${s}"
  done
  out="$(dirname $out)/STACKED_$(basename $out).${ext}"
  cmd="${cmd} -o ${out}"
  if [ $VERBOSE ];
  then
    echo Processing data $[$i+1]/$total
    echo $cmd
    $cmd
  else
    progress_bar $i $total
    $cmd > /dev/null
  fi
done
}

runner $SEQs
