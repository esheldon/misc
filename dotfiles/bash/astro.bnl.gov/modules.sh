# sets up modules and loads afs, anaconda, astrodat and wq
source /opt/astro/SL64/bin/setup.astro.sh

#
# AFS modules
#

# tmv using intel now, and galsim needs tmv
#module load intel_compilers
#module load tmv/0.71         # *
#module load galsim/jarvis    # *

#
# my stuff
#

module load use.own
module load local      # *
module load perllib
module load shell_scripts

# -python

module load espy/local

# this module sets env vars into ~/exports/espy_packages-local
# and also loads acor,emcee,pycallgraph,biggles from afs
module load espy_packages/local

# need their own modules because they hold data, and
# thus need to set the NSIM_DIR etc
module load nsim/local
module load deswl/local
module load gmix_meds/local


# these modules were installed into espy_packages
#module load fitsio/local     # *
#module unload esutil && module load esutil/local     # *

#module load psfex-ess/local

#module load meds/local       # *
#module load gmix_meds/local

#module load ngmix/local  # requires numba

#module load desdb/local
#module load deswl/local
#module load pymangle   # *

#module load recfile/local      # *

#module load cosmology  # *

#module load sdsspy

# not installed in espy_packages
#module load fimage/local     # *
#module load admom/local      # *
#module load gmix_image/local # *
#module load stomp/local      # *
#module load numpydb    # *
#module load columns
