#!/bin/bash

usage()
{
cat << EOF
usage: $0 options

Separates test and train sets from a delineated database.

OPTIONS:
   -h      Show this message
   -d      Database directory (default is '.')
   -s      Sequence pattern. Be sure to put * in quotes ''
   -g      Ground truth pattern. Be sure to put * in quotes ''
   -r      Ratio of samples to be considered in training (default is 0.2)
   -c      Copy files instead of making link
   -b      Base directory for train and test folders
   -v      Verbose
EOF
}

rootdir=$(pwd)
copycommand="ln -s"
verbose=
ratio=0.2
basedir=$(pwd)
while getopts “hd:s:g:bb:cv” OPTION
do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    d)
      rootdir=$OPTARG
      ;;
    b)
      basedir=$OPTARG
      ;;
    s)
      seqpat=$OPTARG
      ;;
    g)
      gtpat=$OPTARG
      ;;
    r)
      ratio=$OPTARG
      ;;
    c)
      copycommand=cp
      ;;
    v)
      verbose=yes
      ;;
    ?)
      usage
      exit
      ;;
  esac
done

testdir=$basedir/test
traindir=$basedir/train

seqfiles=($(ls $rootdir/$seqpat))
gtfiles=($(ls $rootdir/$gtpat))

if [ ! -d "$rootdir" ];
then
  echo "Root directory $rootdir not found!" > 2
  exit 1
fi

if [ "${#gtfiles[@]}" -ne "${#seqfiles[@]}" ];
then
  echo "Number of sequences and ground truth files should be the same."
  exit 1
fi

totalLen=${#gtfiles[@]}
trainLen=$(echo $ratio'*'$totalLen|bc)
trainLen=${trainLen/\.*}
totalIndex=( $(shuf -i 0-$((totalLen-1)) )) 
trainIndex="${totalIndex[@]:0:$trainLen}"
testIndex="${totalIndex[@]:$trainLen}"

[ ! -d "$traindir" ] && mkdir $traindir
[ ! -d "$testdir" ] && mkdir $testdir 

for ti in $trainIndex;
do
  $copycommand --target-directory=$traindir ${gtfiles[ti]}
  $copycommand --target-directory=$traindir ${seqfiles[ti]}
done

for ti in $testIndex;
do
  $copycommand --target-directory=$testdir ${gtfiles[ti]}
  $copycommand --target-directory=$testdir ${seqfiles[ti]}
done


