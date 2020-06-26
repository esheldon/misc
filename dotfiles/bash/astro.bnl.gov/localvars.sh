# shopt -s direxpand
export TMPDIR=/data/esheldon/tmp

export DESDATA=/astro/u/astrodat/data/DES

export LENSDIR=/gpfs/mnt/gpfs02/astro/workarea/esheldon/lensing
export DES_LENSDIR=$LENSDIR/des-lensing

export SHAPESIM_DIR=$LENSDIR/shapesim

export DESREMOTE_RSYNC=desar2.cosmology.illinois.edu::ALLDESFiles/desarchive
export DES_RSYNC_PASSFILE=$HOME/.des_rsync_pass

export DESMEDS_CONFIG_DIR=$HOME/lensing/des-lensing/medsconf

export COSMOS_DIR=$LENSDIR/galsim-cosmos-data

export WEBDIR=/astro/u/esheldon/www 

export NGMIXER_OUTPUT_DIR=$DESDATA/wlpipe
export NGMIXER_CONFIG_DIR=$DES_LENSDIR/ngmixer-config

export MEDS_DIR=/astro/u/astrodat/data/DES/meds

export NBRMIXER_OUTPUT_DIR=$LENSDIR/nbrmixer

# this is where the piff outputs live
export PIFF_DATA_DIR=/astro/u/mjarvis/work/y3_piff

# psf map files for piff
export PIFF_MAP_DIR=/astro/u/astrodat/data/DES/meds/piff-psf-maps

# locations for astrometry files by zone
export ASTROM_DIR=/astro/u/mjarvis/work/y3_piff/astro/

export FITVD_DIR=/gpfs02/astro/desdata/esheldon/fitvd
export FITVD_CONFIG_DIR=/gpfs02/astro/desdata/esheldon/fitvd/config

export SHREDX_CONFIG_DIR=/gpfs02/astro/desdata/esheldon/shredder/configs

export SHREDDER_DIR=/gpfs02/astro/desdata/esheldon/shredder

# used for pizza cutter tests that require data. Those tests
# will be skipped if this isn't set
export TEST_DESDATA=/astro/u/esheldon/git/des-y3-test-data

export SLDIR=/astro/u/astrodat/data/DES/jpg/stronglens
export CATSIM_DIR=$HOME/lensing/catsim
