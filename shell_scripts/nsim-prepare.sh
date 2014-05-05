# source this before submitting nsim jobs
module unload wq &> /dev/null
module unload espy_packages &> /dev/null
#module load nsim/env
module load nsim/env2

module unload espy
module load espy/work
module load galsim/local

module unload great3-config
module load great3-config/work

