# sets up modules and loads afs, anaconda, astrodat and wq
source /opt/astro/SL64/bin/setup.astro.sh

#
# AFS modules
#


#
# my stuff
#

module load use.own
module load tmv/0.72-static         # *
module load cfitsio/3370-static         # *
module load ccfits/2.4-static         # *
module load local      # *
module load perllib
module load shell_scripts

module load galsim/local     # *

# -python

# loads the espy_packages stuff plus some other modules
module load espy_packages/local

# need their own modules because they hold data, and
# thus need to set the NSIM_DIR etc
#module load nsim/local
#module load deswl/local
#module load gmix_meds/local

# this has python in it and can't be installed to
# the packages dir
#module load des-oracle


# these modules were installed into espy_packages
#module load fitsio/local     # *
#module unload esutil && module load esutil/local     # *

#module load psfex-ess/local

#module load meds/local       # *

#module load ngmix/local  # requires numba

#module load desdb/local
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
