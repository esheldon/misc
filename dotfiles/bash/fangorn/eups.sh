f=/Users/esheldon/local/products/eups/bin/setups.sh
if [[ -e "$f" ]]; then
    source "$f"

    setup local -r ~/local
    #setup numpy
    #setup ipython
    #setup pyfitspatch

    #setup wl -r ~/exports/wl-work

    #setup numpy
    #setup scipypatch
    #setup esutil -r ~/exports/esutil-work

    setup espy -r ~/python
    setup shell_scripts -r ~/shell_scripts
    setup perllib -r ~/perllib

    #setup biggles

    #setup ipython

    #setup gflags
    #setup stomp -r ~/exports/stomp-local

    #setup pgnumpy -r ~/exports/pgnumpy-local
else
    echo eups not found, setting up local
    localdir=/Users/esheldon/local
    prepend_path PATH $localdir/bin
    prepend_path LD_LIBRARY_PATH $localdir/lib 
    prepend_path LIBRARY_PATH $localdir/lib 
    prepend_path C_INCLUDE_PATH $localdir/include 

    prepend_path PATH ~/shell_scripts
    prepend_path PATH ~/perllib
    prepend_path PYTHONPATH ~/python
    prepend_path PATH ~/python/bin
fi
