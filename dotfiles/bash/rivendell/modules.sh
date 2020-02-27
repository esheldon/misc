f=/etc/profile.d/modules.sh
if [[ -e $f ]]; then
    source "$f"

    module load use.own

    module load local
    module load espy/local
    module load anaconda/3
    module load shell_scripts

fi

