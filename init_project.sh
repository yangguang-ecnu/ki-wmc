#!/bin/bash

usage()
{
cat << EOF
usage: $0 options

This setup the build envirnment.

OPTIONS:
   -h      Show this message
   -e      Use Eclipse generator
   -i      In place installation
   -p      Installation prefix
EOF
}

origdir=$(pwd)
rootdir=$(readlink -f $(dirname $0))
srcdir=$rootdir/src
builddir=$(readlink -f $rootdir/../build)

installdir=
generator=
verbose=

while getopts "heip:v" OPTION
do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    e)
      generator="Eclipse CDT4 - Unix Makefiles"
      ;;
    i)
      installdir="$rootdir"
      ;;
    p)
      installdir="$(readlink -f $OPTARG)"
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

-DCMAKE_INSTALL_PREFIX=

log()
{
  [ $verbose ] && echo -e $@
}
runcmd()
{
  log running $(tput bold)$(tput setaf 1) "$@"$(tput sgr0)
  "$@"
}

if [ ! -d $srcdir ];
then
  echo "No 'src' directory." > 2
  exit 1
elif [ ! -f $srcdir/CMakeLists.txt ];
then
  echo "$srcdir/CMakeLists.txt not found." > 2
  exit 1
fi

log "root:\t$rootdir"
log "source:\t$srcdir"
log "build:\t$builddir"
if [ -n "$installdir" ];
then
log "install:\t$installdir"
fi

if [ ! -d $builddir ];
then
  log "Creating build directory ($builddir)"
  runcmd mkdir $builddir
elif [ $srcdir/CMakeLists.txt -nt $builddir/CMakeCache.txt ];
then
  log "Clearing CMake cache."
  [ -d $builddir/CMakeFiles ] && runcmd rm -rf $builddir/CMakeFiles
  [ -f $builddir/CMakeCache.txt ] && runcmd rm $builddir/CMakeCache.txt
fi

itkdir=/home/soheil/lib/cmake/ITK-4.2/
itkdir=/home/soheil/lib/ITK/InsightToolkit-4.1.0-binary/
cmakefnd=ITKConfig.cmake
cmakearg=
if [ -f "$itkdir/$cmakefnd" ];
then
  cmakearg="$cmakearg -DITK_DIR=$itkdir"
fi

if [ "$installdir" ];
then
  cmakearg="$cmakearg -DCMAKE_INSTALL_PREFIX=\"$installdir\""
fi

cmakearg="$cmakearg $srcdir"

runcmd cd "$builddir"
if [ "$generator" ];
then
  runcmd cmake -G "$generator" $cmakearg
else
  runcmd cmake $cmakearg
fi
runcmd cd "$origdir"
