f=/etc/profile.d/modules.sh
if [[ ! -e $f ]]; then
    f="$HOME/local/Modules/3.2.10/init/bash"
fi

if [[ -e $f ]]; then
    . "$f"

    module load use.own

    module load local
    module load espy/local
    module load anaconda/py3
    module load shell_scripts
    source activate default

fi
