# after this we only have the local conda environ
# and espy/work

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
module load espy/work

source activate gmix-meds-work

export NSIM_DIR=~/envs/gmix-meds-work
export GMIX_MEDS_DIR=~/envs/gmix-meds-work
