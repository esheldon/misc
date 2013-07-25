module load use.own

#
# global
#

module load gcc
module load git
module load openmpi-gnu

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
# the special hpcp line
#
module load scons-hpcp
module load blas-hpcp
module load atlas-hpcp

module load readline-hpcp                   # needed by python
module load python-hpcp
module load lapack-hpcp                     # needs mkl
module load numpy-hpcp
module load scipy-hpcp
module load ipython-hpcp


#
# des module installs
#

module load pyyaml
module load parallel
module load libaio                          # * for oracle
module unload tmv && module load tmv/0.71   # *

module load desoracle



# local
module unload desdb && module load desdb/local
module unload deswl && module load deswl/local
module unload meds && module load meds/local
module unload psfex && module load psfex/local
module unload gmix_meds && module load gmix_meds/local

module unload gsim_ring && module load gsim_ring/local

module load deswl-checkout/local

module load plotutils
module load biggles

module load mkl-des

module load PIL/local
module load eyeballer/local

module load fimage/local

#
# installs in my home
#

module load perllib
module load shell_scripts
module unload espy && module load espy/local

module load fitsio/local

module load cfitsio/beta
module load ccfits/2.4

module unload esutil && module load esutil/local

module unload shapelets && module load shapelets/local   # *


module load admom      # *

module load gmix_image/local # *
module load emcee
module load acor

