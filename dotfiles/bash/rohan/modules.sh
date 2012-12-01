f=~/local/Modules/3.2.8/init/bash
if [[ -e $f ]]; then
    source "$f"

    export MODULE_INSTALLS=~/local/module-installs

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    module load wq

    module load emcee

    module load galsim     # *
    module load admom      # *
    module load fimage/local     # *
    module load mangle     # *
    module load pymangle   # *
    module load sdsspy     # *

    module load biggles    # *
    module load esutil     # *
    module load fitsio     # *
    module load shell_scripts
    module load perllib

    module load cosmology

    module load stomp      # *
    module load local      # *

    module unload tmv && module load tmv/0.71   # *
    module load wl/local   # *

    module load espy/local

    module load gmix_image/local # *
fi

