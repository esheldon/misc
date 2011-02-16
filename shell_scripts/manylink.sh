#!/bin/sh

basedir=/usr/sdss/data01/imaging
olddir=$PWD

nargs=0
for args
do
    nargs=`expr $nargs + 1`
    case $nargs in
	1) run=$args ;;
	2) rerun1=$args ;;
	3) rerun2=$args ;;
	4) subdir=$args ;;
	5) camcol=$args ;;
	6) prefix=$args ;;
	*) ;;
    esac
done

if [ $nargs -eq 0 ]
then
    echo  -Syntax: manylink run rerun1 rerun2 subdir camcol prefix
    exit
fi

indir=$basedir/$run/$rerun1/$subdir/$camcol
outdir=$basedir/$run/$rerun2/$subdir/$camcol

list=`find $indir -name "$prefix*"`

cd $outdir

for b in $list
do
    echo $b
    addstr=`echo $b | sed "sx"$indir/"xxg"`
    echo "ln -s $b ./$addstr"
#    ln -s $b ./$addstr
done

cd $olddir
