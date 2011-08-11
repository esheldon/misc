f=~/tmp/modules-test/Modules/3.2.8/init/bash
if [[ -e $f ]]; then
    source "$f"

    module load use.own
    module load shell_scripts
    module load espy

fi


