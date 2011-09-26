append_path PATH ~/python/bin
append_path PATH ~/shell_scripts
append_path PATH ~/shell_scripts/sdss
append_path PATH ~/perllib
append_path PATH ~/perllib/sdss_inventory

if [[ $PYTHONPATH == "" ]]; then
    PYTHONPATH=~/python
else
    append_path PYTHONPATH ~/python
fi

export PATH
export PYTHONPATH

