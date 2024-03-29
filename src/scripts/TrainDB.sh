#!/bin/bash

usage()
{
cat << EOF
usage: $0 options

Separates test and train sets from a delineated database.

OPTIONS:
   -h      Show this message
   -d      Database directory (default is .)
   -s      Sequence pattern. (default is STACK)
   -g      Ground truth pattern. (default is WM)
   -r      Radius (default is 2)
   -n      Number of desired support-vectors (default is 1000)
   -m      Output trained model filename (default is none i.e. trainer default)
   -c      Trainer command (default is wmcTrain)
   -v      Verbose
   -f      Fake run, just print the command to be runned
EOF
}

fake=
command="wmcTrain"
verbose=
radius=2
basedir=$(pwd)
seqpat="STACK"
gtpat="WM"
numSV=1000
modelFN=
while getopts "hd:s:g:r:n:m:c:vf" OPTION
do
  case $OPTION in
    h) usage;exit 1;;
    d) basedir=$OPTARG;;
    s) seqpat=$OPTARG;;
    g) gtpat=$OPTARG;;
    r) radius=$OPTARG;;
    n) numSV=$OPTARG;;
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
  echo "Base directory $basedir not found!" > 2
  exit 1
fi

seqfiles=($(ls $basedir|grep $seqpat|sort))
gtfiles=($(ls $basedir|grep $gtpat|sort))

if [ "${#seqfiles[@]}" -eq "0" ];
then
  echo "No input sequence file."
  exit 1
fi

if [ "${#gtfiles[@]}" -ne "${#seqfiles[@]}" ];
then
  echo "Number of sequences and ground truth files should be the same."
  exit 1
fi

for ti in ${!seqfiles[@]};
do
  command="$command -i $basedir/${seqfiles[ti]} -m $basedir/${gtfiles[ti]}"
done

command="$command -r $radius -s $numSV -t 0"

if [ "$modelFN" ];
then
  command="$command -o $modelFN"
fi

if [ "$fake" ];
then
  echo "$command"
else
  $command
fi
