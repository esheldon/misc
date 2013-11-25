# will want a different one for tutti
source ~astrodat/setup/setup-modules.sh

module load afs
module load anaconda

module load biggles

module load acor
module load emcee

module load pycallgraph

module unload tmv && module load tmv/0.71     # *
module load galsim/jarvis                     # *

# my stuff
module load use.own

module load local      # *

module load perllib
module load shell_scripts
module load espy/local

module load fitsio/local     # *
module unload esutil && module load esutil/local     # *
module load fimage/local     # *
module load admom/local      # *

module load psfex-ess/local

module load gmix_image/local # *

module load meds/local       # *
module load gmix_meds/local

module load ngmix/local  # requires numba

module load gsim_ring/local

module load desdb/local
module load deswl/local

module load gsim_ring/local # *

module load pymangle   # *

# this is currently just the python extension
module load stomp/local      # *

module load recfile/local      # *

module load cosmology  # *

module load numpydb    # *
module load columns

module load sdsspy

#ModuleCmd_Load.c(204):ERROR:105: Unable to locate a modulefile for 'libaio'
#ModuleCmd_Load.c(204):ERROR:105: Unable to locate a modulefile for 'swig'

if [[ $check == "blahblah" ]]; then


    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    # this are under $MODULESHOME/modulefiles
    # and installed under $MODULE_INSTALLS
    #module load use.own

    #module load pylint

    module load mangle     # *
    #module load pymangle   # *
    #module load gmix_image/local # *

    #module load meds/local
    #module load gmix_meds/local
    #module load psfex-ess/local

    # for oracle libraries
    module load libaio     # *

    #module load parallel

    module load cjson      # *

    #module load pyyaml
    # also loads pcre      # *
    module load swig       # *

    #module load cfitsio/3350   # *
    #module load ccfits/2.4     # *

    #module load emcee
    #module load acor

    # these are under my ~/privatemodules
    # and installed generally under ~/exports

    #module load perllib
    #module load shell_scripts
    #module load espy/local

    module load desfiles
    #module load desdb

    #module load local      # *

    # this is currently just the python extension
    #module load stomp      # *

    #module unload esutil && module load esutil/local     # *
    #module load recfile    # *

    #module load cosmology  # *
    #module load fimage/local     # *
    #module load fitsio/local

    #module load numpydb    # *
    module load pgnumpy    # *

    #module unload deswl-checkout && module load deswl-checkout/local

    #module load scikits_learn # *
    module load scikits_learn/new # *

    # python only
    #module load sdsspy
    #module load columns

    # these get loaded in other scripts, be careful
    module unload tmv && module load tmv/0.71     # *

    # prereq: ccfits, tmv, desfiles, esutil
    module unload shapelets && module load shapelets/local   # *

    module load galsim/local


    module load esidl
    module load sdssidl
    module load idlastron
fi


