# make sure you have sourced the stuff in the default
# bashrc put in your home when your account was created.
# NERSC_HOST is set there, and the module system is set up
#
# also you may want to add these to your startup file
#
#    . /project/projectdirs/cmb/modules/carver/hpcports.sh
#    hpcports gnu
#    module load hpcp
#
# and touch ~/.no_default_modules in your home

module use /global/project/projectdirs/des/wl/modules/$NERSC_HOST/modulefiles
module load des-nersc
