f=/Users/esheldon/local/products/eups/bin/setups.sh
if [[ -e "$f" ]]; then
    source "$f"

    #setup numpy
    #setup ipython
    #setup pyfitspatch

    #setup wl -r ~/exports/wl-work

    setup numpy
    setup scipypatch
    setup esutil -r ~/exports/esutil-work

    setup espy -r ~/python

    setup biggles

    setup ipython

    setup gflags
    setup stomp -r ~/exports/stomp-local

    setup pgnumpy -r ~/exports/pgnumpy-local
fi
