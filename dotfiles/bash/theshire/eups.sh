return

f=/home/esheldon/local/products/eups/bin/setups.sh
if [ -e "$f" ]; then
	source $f

    setup cosmology -r ~/exports/cosmology-local

    setup local -r ~/local
    setup tmux

    setup swig

    setup parallel

    setup scipy
	setup scons
	setup wl -r ~/exports/wl-local
    setup tmv -r ~/exports/tmv-local

	setup esutil -r ~/exports/esutil-local
	#setup esutil -r ~/exports/esutil-test

    setup recfile -r ~/exports/recfile-local

    # this will also set up numpy and plotutils
    setup biggles
    setup ipython

	setup espy -r ~/python
    setup shell_scripts -r ~/shell_scripts
    setup perllib -r ~/perllib

    setup numpydb -r ~/exports/numpydb-local

    setup stomp -r ~/exports/stomp-local

    setup fimage -r ~/exports/fimage-local
    setup admom -r ~/exports/admom-local

    setup sdsspy -r ~/exports/sdsspy-local
    setup columns -r ~/exports/columns-local
fi


