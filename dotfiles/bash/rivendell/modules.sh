f=/etc/profile.d/modules.sh
if [[ -e $f ]]; then
    source "$f"

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    # python
    #module load espy/local
    #module load fitsio/py3
    #module load pymangle/py3
    #module load biggles/python3
    #module load healpix/local

    module load anaconda/3
    module load local      # *
    module load shell_scripts

fi

