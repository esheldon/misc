f="$HOME/local/Modules/3.2.10/init/bash"

if [[ -e $f ]]; then
    . "$f"

    module load null

    export MODULE_INSTALLS=~/local/module-installs

    # those marked with * have platform dependent code, e.g. the are
    # stand-alone C or extensions for python, etc.

    module load use.own

    # python
    module load anaconda/py2
    module load espy/local

    module load local      # *

    module load tmv/0.73   # *

    #module load parallel
    module load shell_scripts
    module load perllib
fi
