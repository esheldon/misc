f=/home/esheldon/local/products/eups/bin/setups.sh
if [ -e "$f" ]; then
	source $f

	#setup scons
	#setup ccfits
	#setup tmv
	#setup wl -r ~/exports/wl-work
	#setup esutil -r ~/exports/esutil-work

	#setup sdssidl -r ~/svn/sdssidl
	#setup esidl -r ~/idl.lib
	#setup idlgoddard

    setup numpy
    setup ipython
	setup esutil -r ~/exports/esutil-local
	setup espy -r ~/python
    setup perllib -r ~/perllib
    setup shell_scripts -r ~/shell_scripts

    setup numpydb -r ~/exports/numpydb-local

    setup stomp -r ~/exports/stomp-local

    setup biggles

    setup recfile -r ~/exports/recfile-local
fi


