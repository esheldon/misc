f=/etc/profile.d/modules.sh

if [[ -e $f ]]; then
    . "$f"

    module load use.own

    module load espy/local
    module load anaconda/py3
    source activate default

fi
