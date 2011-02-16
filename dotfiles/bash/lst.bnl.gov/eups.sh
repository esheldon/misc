# make EUPS available
source ~esheldon/local/products/eups/bin/setups.sh

setup scipy
setup ipython
setup matplotlib
setup biggles

# will set up cfitsio/ccfits/tmv/esutil
setup wl -r ~esheldon/exports/wl-local

# override esutil with local install
setup esutil -r ~esheldon/exports/esutil-local

# for cropping eps files
#setup epstool

#setup libtool
#setup gflags
#setup stomp -r ~/exports/stomp-work

# my python directory
setup espy -r ~esheldon/python

setup scons
