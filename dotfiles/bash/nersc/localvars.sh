umask u=wrx,g=rx

if [[ $hname == "datatran" ]]; then
    source /usr/share/Modules/init/bash
fi

#source /global/project/projectdirs/des/wl/setup/setup.sh

export export CLUSTERSTEP=/global/project/projectdirs/des/wl/cluster-step
export DESWL_CHECKOUT=~/git/deswl

export LENSDIR=~esheldon/lensing
export SHAPESIM_DIR=~esheldon/lensing/shapesim
