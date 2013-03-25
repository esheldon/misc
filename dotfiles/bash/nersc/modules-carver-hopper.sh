module load use.own
if [[ $hname == "carver" ]]; then
    . /project/projectdirs/cmb/modules/carver/hpcports.sh
    hpcports gnu
    module load hpcp
elif [[ $hname == "hopper" ]]; then
    . /project/projectdirs/cmb/modules/hopper/hpcports.sh
    hpcports gnu
    module load hpcp
fi

#
# global
#

module load gcc
#module load mkl                             # needed for numpy/lapack
module load git
module load openmpi-gnu

#
# the special hpcp line
#
module load scons-hpcp
module load blas-hpcp
module load atlas-hpcp

module load readline-hpcp                   # needed by python
module load python-hpcp
module load lapack-hpcp                     # needs mkl above
module load numpy-hpcp
module load scipy-hpcp
module load ipython-hpcp
#module load pyyaml-hpcp

#module load cfitsio-hpcp
#module load ccfits-hpcp


#
# des module installs
#

module load pyyaml
module load libaio                          # * for oracle
module unload tmv && module load tmv/0.71   # *

module load desoracle
module unload desdb-ess && module load desdb-ess/local
module unload deswl-ess && module load deswl-ess/local

# local git checkout
module load deswl-checkout/local

module load plotutils
module load biggles

module load mkl-des

#
# installs in my home
#

module load perllib
module load shell_scripts
module unload espy && module load espy/local

# need to move away from fitsio-ess
module load fitsio-ess/local

# -ess required because of missing include paths in hpcp stuff
module load cfitsio-ess/3310
module load ccfits-ess/2.4

module load fitstools-hpcp

module unload esutil && module load esutil/local

module unload shapelets-ess && module load shapelets-ess/local   # *

#module load recfile    # *

#module load cosmology  # *
module load admom      # *
#module load fimage/local     # *
#module load columns

module load gmix_image/local # *
module load emcee
module load acor

