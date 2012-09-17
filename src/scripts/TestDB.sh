#!/bin/bash

usage()
{
cat << EOF
usage: $0 options

Run the segmentation on a database.

OPTIONS:
   -h      Show this message
   -d      Database directory (default is '.')
   -s      Sequence pattern. (default is 'STACK')
   -g      Ground truth pattern. (default is 'WM')
   -r      Radius (default is 2)
   -m      Output trained model filename (default is none i.e. trainer default)
   -c      Segmentation command (default is wmcSegmentation or wmcExpand)
   -v      Verbose
   -f      Fake run, just print the command to be runned
EOF
}

fake=
command=
verbose=
radius=2
basedir=$(pwd)
seqpat="STACK"
gtpat="WM"
modelFN=
kernelMode=
while getopts "hd:s:g:r:m:c:vf" OPTION
do
  case $OPTION in
    h) usage;exit 1;;
    d) basedir=$OPTARG;;
    s) seqpat=$OPTARG;;
    g) gtpat=$OPTARG;kernelMode=yes;;
    r) radius=$OPTARG;;
    m) modelFN=$OPTARG;;
    c) command=$OPTARG;;
    v) verbose=yes;;
    f) fake=yes;;
    ?) usage;exit;;
  esac
done

basedir=$(readlink -f $basedir)

if [ ! -d "$basedir" ];
then
  echo "Base directory $basedir not found!" >&2
  exit 1
fi

if [ ! -f $modelFN ];
then
  echo "Model file $modelFN not found!" >&2
  exit 1
fi

seqfiles=($(ls $basedir|grep $seqpat|sort))
if [ "${#seqfiles[@]}" -eq "0" ];
then
  echo "No input sequence file."
  exit 1
fi

if [ $kernelMode ];
then
  
  if [ ! "$command" ];
  then
    command="wmcTest"
  fi  

  gtfiles=($(ls $basedir|grep $gtpat|sort))
  if [ "${#gtfiles[@]}" -ne "${#seqfiles[@]}" ];
  then
    echo "Number of sequences and ground truth files should be the same."
    exit 1
  fi
  for ti in ${!seqfiles[@]};
  do
    command="$command -i ${seqfiles[ti]}"
    command="$command -g ${gtfiles[ti]}"
  done
  command="$command -f roc.csv -o conf_"
else
  if [ ! "$command" ];
  then
    command="wmcSegmentation"
  fi  

  for ti in ${!seqfiles[@]};
  do
    command="$command -i ${seqfiles[ti]}"
  done
  command="$command -o seg_"
fi
command="$command -d $basedir -r $radius -m $modelFN"

if [ "$fake" ];
then
  echo "$command"
else
  $command
fi
