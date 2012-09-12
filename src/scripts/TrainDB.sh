#!/bin/bash

usage()
{
cat << EOF
usage: $0 options

Separates test and train sets from a delineated database.

OPTIONS:
   -h      Show this message
   -d      Database directory (default is '.')
   -s      Sequence pattern. Be sure to put * in quotes. (default is 'STACK*')
   -g      Ground truth pattern. Be sure to put * in quotes. (default is '*WM*')
   -r      Radius (default is 2)
   -n      Number of desired support-vectors (default is 1000)
   -m      Output trained model filename (default is none i.e. trainer default)
   -c      Trainer command (default is wmlTrain)
   -v      Verbose
EOF
}

command="wmlTrain"
verbose=
radius=2
basedir=$(pwd)
seqpat="STACK*"
gtpat="*WM*"
numSV=1000
modelFN=
while getopts “hd:s:g:r:n:m:c:v” OPTION
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
    ?) usage;exit;;
  esac
done

if [ ! -d "$basedir" ];
then
  echo "Base directory $basedir not found!" > 2
  exit 1
fi

seqfiles=($(find $basedir -name $seqpat))
gtfiles=($(find $basedir -name $gtpat))

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
  command="$command -i ${gtfiles[ti]} -m ${seqfiles[ti]}"
done

command="$command -r $radius -s $numSV -t 0"

if [ "$modelFN" ];
then
  command="$command -o $modelFN"
fi

$command
