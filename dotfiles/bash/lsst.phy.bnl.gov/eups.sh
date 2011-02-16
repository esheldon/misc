f=~/local/products/eups/bin/setups.sh
if [ -e "$f" ]; then
	source $f

    setup local -r ~/local
    #setup tmux

    #setup scipy
	#setup scons
	#setup wl -r ~/exports/wl-local
    #setup tmv -r ~/exports/tmv-local

	#setup esutil -r ~/exports/esutil-local

    #setup recfile -r ~/exports/recfile-local

	#setup sdssidl -r ~/svn/sdssidl
	#setup esidl -r ~/idl.lib
	#setup idlgoddard

    # this will also set up numpy and plotutils
    #setup biggles
    #setup ipython

	setup espy -r ~/python
    setup shell_scripts -r ~/shell_scripts
    setup perllib -r ~/perllib

    #setup numpydb -r ~/exports/numpydb-local

    #setup stomp -r ~/exports/stomp-local

fi


