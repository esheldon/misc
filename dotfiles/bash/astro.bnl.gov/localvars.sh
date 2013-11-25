# sets up a ton of stuff

f=~astrodat/setup/setup.sh
if [[ -e $f ]]; then
    source "$f"
fi

# all these require mount of /opt/astro/....
# tutti does not mount these
#f=/opt/astro/SL64/bin/setup.astro.sh
#if [[ -e $f ]]; then
#    source "$f"
#fi

source ~astrodat/setup/setup-wq.sh

if [[ $(hostname) != "tutti.astro.bnl.gov" ]]; then
    source ~/local/des-oracle/setup.sh
fi

export MAXBCG_CATDIR=/astro/tutti1/esheldon/lensinputs-v1/maxbcg/catalog
export CLUSTERS_INPUT=~/oh/clusters-input

export PHOTOZ_DIR=~esheldon/photoz
export SWEEP_REDUCE=~esheldon/sweep-reduce

export MASK_DIR=~esheldon/masks

export LENSDIR=~esheldon/lensing
export LENSDIR_HDFS=hdfs:///user/esheldon/lensing
export SHAPESIM_DIR=~esheldon/lensing/shapesim
export GMIX_SDSS=~esheldon/gmix-sdss
export SHAPESIM_FS=nfs

export PIXEL_MASK_BASIC=pixel_mask_dr4_basic
export PIXEL_MASK_BOUND=pixel_mask_dr4_bound
export PIXEL_MASK_SIMPLE=pixel_mask_dr4_simple
export PIXEL_MASK_BASIC_PRINCETON=$MASK_DIR/pixel_mask_princeton_basic

# location we keep simulations of the regauss algorithm
export REGAUSSIM_DIR=~esheldon/lensing/regauss-sim
export REGAUSSIM_HDFS_DIR=hdfs:///user/esheldon/lensing/regauss-sim

export SGSEP_DIR=~esheldon/oh/star-galaxy-separation/

export CLUSTERSTEP=~/lensing/cluster-step
export CLUSTERSTEP_HDFS=hdfs:///user/esheldon/lensing/cluster-step

export TMPDIR=/data/esheldon/tmp

export DESWL_CHECKOUT=~/git/deswl

export DESREMOTE_RSYNC=desar2.cosmology.illinois.edu::DESFiles
export DES_RSYNC_PASSFILE=~/.des_rsync_pass
