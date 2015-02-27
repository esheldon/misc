# reverted ngmix, espy and gmix_meds
# espy copied to espy/work2

h=$MODULESHOME
if [[ $h == "" ]]; then
    # starting from scratch
    source /opt/astro/SL64/Modules/default/etc/profile.modules
    module load use.own
fi

module unload wq &> /dev/null
module unload espy_packages &> /dev/null
module unload biggles &> /dev/null
module unload anaconda &> /dev/null
module unload espy &> /dev/null

module load anaconda/gpfs
module load espy/work2

source activate great-des-revert

export GMIX_MEDS_DIR=~/envs/great-des-revert
export GMIX_MEDS_DATADIR=/astro/u/astrodat/data/DES/wlpipe
