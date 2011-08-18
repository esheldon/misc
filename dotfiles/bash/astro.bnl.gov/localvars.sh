# sets up a ton of stuff

f=~astrodat/setup/setup.sh
if [[ -e $f ]]; then
    source "$f"
fi

f=/opt/astro/SL53/bin/setup.astro.sh
if [[ -e $f ]]; then
    source "$f"
    export C_INCLUDE_PATH=`/opt/astro/SL53/bin/addpath -env C_INCLUDE_PATH - ${ROOTSYS}/include/root /opt/astro/SL53/include /usr/local/include`
    export CPATH=`/opt/astro/SL53/bin/addpath -env CPATH - ${ROOTSYS}/include/root /opt/astro/SL53/include /usr/local/include`

fi

f=/opt/astro/SL53/bin/setup.hadoop.sh
if [[ -e $f ]]; then
    source "$f"
fi

export MAXBCG_CATDIR=/mount/tutti1/esheldon/lensinputs-v1/maxbcg/catalog
export MAXBCG_INPUT=/mount/tutti1/esheldon/maxbcg-input

export PHOTOZ_DIR=~esheldon/photoz
export SWEEP_REDUCE=~esheldon/sweep_reduce

export MASK_DIR=~esheldon/masks

export LENSDIR=~esheldon/lensing

export PIXEL_MASK_BASIC=pixel_mask_dr4_basic
export PIXEL_MASK_BOUND=pixel_mask_dr4_bound
export PIXEL_MASK_SIMPLE=pixel_mask_dr4_simple
export PIXEL_MASK_BASIC_PRINCETON=$MASK_DIR/pixel_mask_princeton_basic

# location we keep simulations of the regauss algorithm
export REGAUSSIM_DIR=~esheldon/regauss-sim

export SGSEP_DIR=~esheldon/oh/star-galaxy-separation/
