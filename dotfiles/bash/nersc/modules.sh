if [[ $hname == "carver" ]]; then
    . /project/projectdirs/cmb/modules/carver/hpcports.sh
    hpcports gnu
    module load hpcp
elif [[ $hname == "hopper" ]]; then
    . /project/projectdirs/cmb/modules/hopper/hpcports.sh
    hpcports gnu
    module load hpcp
fi

module load use.own

#
# global
#

module load gcc
module load mkl                             # needed for numpy/lapack
module load git

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

module load cfitsio-hpcp
#module load ccfits-hpcp

#
# des module installs
#

module load libaio                          # * for oracle
module unload tmv && module load tmv/0.71   # *

module load desoracle
module load desdb-ess/local

#module load desfiles
#module load des # create this

#
# installs in my home
#

module load perllib
module load shell_scripts
module unload espy && module load espy/local



module load fitsio-ess/local

module unload esutil && module load esutil-ess/local

#module unload wl-ess && module load wl-ess/local   # *

#module load recfile    # *

#module load cosmology  # *
#module load admom      # *
#module load fimage/local     # *
#module load columns

#module load gmix_image/local # *
#module load emcee
#module load acor

