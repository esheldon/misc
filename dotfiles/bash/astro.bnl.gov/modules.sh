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

    # for oracle libraries
    module load libaio     # *

    module load parallel

    module load cjson      # *

    #module load pyyaml
    # also loads pcre      # *
    module load swig       # *

    module load ccfits     # *

    module load emcee

    # these are under my ~/privatemodules
    # and installed generally under ~/exports

    module load perllib
    module load shell_scripts
    module load espy/local

    module load desfiles

    module load local      # *

    # this is currently just the python extension
    module load stomp      # *

    module unload esutil && module load esutil/local     # *
    module load recfile    # *

    module load cosmology  # *
    module load admom      # *
    module load fimage/local     # *
    module load fitsio

    module load numpydb    # *
    module load pgnumpy    # *

    module load scikits_learn # *

    # python only
    module load sdsspy
    module load columns


    # these get loaded in other scripts, be careful
    module unload tmv && module load tmv/0.71     # *

    # prereq: ccfits, tmv, desfiles, esutil
    # also numpy if not using system
    module unload wl && module load wl/local   # *


    module load esidl
    module load sdssidl
    module load idlastron
fi


