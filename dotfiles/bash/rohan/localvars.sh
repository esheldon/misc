f=~/local/des-oracle/setup.sh
if [[ -e $f ]]; then
    source "$f"
fi

append_path PATH ~/local/src/dmd/dmd2/linux/bin64

export LENSDIR=~/lensing
export SHAPESIM_FS=local

export CLUSTERSTEP=~/data/cluster-step

export DESDATA=~/data/DES
export DESREMOTE=https://desar2.cosmology.illinois.edu:7443/DESFiles/desardata
export DESPROJ=OPS
