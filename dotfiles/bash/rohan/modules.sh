f=~/local/Modules/3.2.8/init/bash
if [[ -e $f ]]; then
    source "$f"

    export MODULE_INSTALLS=~/local/module-installs

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    # python
    module load anaconda
    module load espy/local

    module load emcee
    module load acor       # *

    module load fitsio     # *

    module load ngmix/local

    module load psfex/local # *
    module load meds/local # *
    module load gmix_meds/local
    # deprecated gmix image tools
    module load gmix_image/local # *

    module load desdb/local
    module load deswl/local

    module load cosmology   # *
    #module load stomp      # *

    module load wq

    # waiting for astropy.fits to be supported
    #module load galsim     # *
    module load admom      # *
    module load fimage/local     # *
    module load pymangle   # *
    module load sdsspy     # *

    module load pgnumpy    # *
    module load biggles    # *
    module load esutil     # *
    module load recfile    # *

    # not python

    module load mangle     # *
    module load gsim_ring/local

    module load local      # *

    module load tmv/0.71   # *
    #module load wl/local   # * don't need currently

    module load parallel
    module load shell_scripts
    module load perllib


fi

