f=/etc/profile.d/modules.sh
if [[ -e $f ]]; then
    source "$f"

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    # python
    module load espy/local
    module load fitsio/local
    module load biggles/local
    module load healpix/local

    module load local      # *
    module load shell_scripts

    module load ngmix/local

fi

