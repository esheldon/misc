# will want a different one for tutti
f=~astrodat/setup/setup-modules.sh
if [[ -e $f ]]; then
    source "$f"
fi
check=`module 2>&1`
check=`echo $check | grep "command not found"`

if [[ $check == "" ]]; then


    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    # this are under $MODULESHOME/modulefiles
    # and installed under $MODULE_INSTALLS
    module load use.own

    #module load wq

    module load pylint

    module load mangle     # *
    module load pymangle   # *
    module load gmix_image/local # *

    module load meds/local
    module load gmix_meds/local
    module load psfex-ess/local
    module load gsim_ring/local

    # for oracle libraries
    module load libaio     # *

    module load parallel

    module load cjson      # *

    #module load pyyaml
    # also loads pcre      # *
    module load swig       # *

    module load cfitsio/3350   # *
    module load ccfits/2.4     # *

    module load emcee
    module load acor

    # these are under my ~/privatemodules
    # and installed generally under ~/exports

    module load perllib
    module load shell_scripts
    module load espy/local

    module load desfiles
    module load desdb

    module load local      # *

    # this is currently just the python extension
    module load stomp      # *

    module unload esutil && module load esutil/local     # *
    module load recfile    # *

    module load cosmology  # *
    module load admom      # *
    module load fimage/local     # *
    module load fitsio/local

    module load numpydb    # *
    module load pgnumpy    # *

    module unload deswl-checkout && module load deswl-checkout/local

    #module load scikits_learn # *
    module load scikits_learn/new # *

    # python only
    module load sdsspy
    module load columns


    # these get loaded in other scripts, be careful
    module unload tmv && module load tmv/0.71     # *

    # prereq: ccfits, tmv, desfiles, esutil
    module unload shapelets && module load shapelets/local   # *

    module load galsim/local


    module load esidl
    module load sdssidl
    module load idlastron
fi


