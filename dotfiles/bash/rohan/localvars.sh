f=~/local/des-oracle/setup.sh
if [[ -e $f ]]; then
    source "$f"
fi

append_path PATH ~/local/src/dmd/dmd2/linux/bin64

export LENSDIR=~/lensing
export SHAPESIM_FS=local
