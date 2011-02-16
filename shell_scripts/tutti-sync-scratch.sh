if [ "$#" -lt 2 ]; then
	echo "usage: $(basename $0) machine run"
	exit 0
fi

machine=$1
run=$2

scratch_dir=/scratch/esheldon/DES/wlbnl/$run
outdir=/global/data/DES/wlbnl/$run

if [ ! -e $outdir ]; then
	mkdir -p $outdir
fi

echo "rsync -av $machine:$scratch_dir/ $outdir/"
rsync -av $machine:$scratch_dir/ $outdir/
