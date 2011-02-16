#!/bin/bash

reroot() {
    tdir=$1
    oldf=$2
    newf=$tdir/$(basename $oldf)
    echo "$newf"
}
usage() {
    echo "usage calcpofz.sh [-d outdir] weightsfile photofile"
}

if [ $# -lt 2 ]; then
    usage
    exit 45;
fi

newdir="default"
while getopts "d:" Option
  do
  case $Option in
      d)  newdir=$OPTARG ;;
      [?]) usage
           exit 45;;
  esac
done
shift $(($OPTIND - 1))

n_near=100
res=$n_near
nz=20
zmin=0.0
zmax=1.1

weightsfile=$1
photfile=$2

suffix=${n_near}-${nz}-${zmin}-${zmax}
pzfile=${photfile%.*}-pofz-$suffix.dat
zfile=${photfile%.*}-z-$suffix.dat


if [ "$newdir" != "none" ]; then
    pzfile=$(reroot $newdir $pzfile)
    zfile=$(reroot $newdir $zfile)
fi

echo
echo "Running calcpofz"
echo
calcpofz $weightsfile $photfile $n_near $nz $res $zmin $zmax $pzfile $zfile

if [ "$?" != "0" ]; then
    echo Halting
    exit 45
fi

