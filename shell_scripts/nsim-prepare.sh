# this has become multi-purpose since we cannot make new conda environments
# that use numba

h=$MODULESHOME
if [[ $h == "" ]]; then
    # starting from scratch
    source /opt/astro/SL64/Modules/default/etc/profile.modules
    module load use.own
else
    module unload wq &> /dev/null
    module unload espy_packages &> /dev/null
fi

module unload espy
module load espy/work
module load galsim/local

module unload great3-config
module load great3-config/work

# needs to go last becaus of the prepends
module load nsim/env2


