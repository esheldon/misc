f=/etc/profile.d/modules.sh
if [[ -e $f ]]; then
    source "$f"

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    module load espy/local
    module load local_packages

    module load local      # *
    module load shell_scripts

fi

