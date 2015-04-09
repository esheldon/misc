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
module unload galsim &> /dev/null

module load anaconda/gpfs
module load espy/work

source activate great3-work

module load great3sims/work

# add great3 datadir etc here

export GREAT3_DATA_DIR=/gpfs01/astro/workarea/esheldon/mygreat3
export GREAT3_CONFIG_DIR=/astro/u/esheldon/git/great3-config
