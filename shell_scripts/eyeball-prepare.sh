# source this before submitting nsim jobs
h=$MODULESHOME
if [[ $h == "" ]]; then
    # starting from scratch
    source /opt/astro/SL64/Modules/default/etc/profile.modules
    module load use.own
else
    module unload wq &> /dev/null
    module unload espy_packages &> /dev/null
fi

module load eyeball-env/work
