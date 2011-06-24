f=~astrodat/setup/setup-eups.sh
if [[ -e $f ]]; then
    source $f

    # this is pure python or data
    setup desfiles -r ~/oh/desfiles
    setup espy -r ~/python

    if [[ $hname == "tutti" ]]; then
        # this will get python and numpy
        echo "doing setups for tutti"
        setup python

        #setup numpy 
        setup scipy
        setup biggles
        setup ipython

        setup esutil -r ~/exports-tutti/esutil-local


        setup ccfits
        setup wl -r ~/exports-tutti/wl-local
        setup tmv -r ~/exports-tutti/tmv-work

        setup sdsspy -r ~/exports-tutti/sdsspy-local

        # columns, sdsspy are under mercurial
        setup mercurial
        # this is pure python, use astro exports
        setup columns -r ~/exports/columns-local

        setup pv

        setup scons
    else
        # this will get numpy and python
        setup scipy

        setup ipython
        setup local -r ~/local

        # this is pure python
        setup espy -r ~/python
        setup shell_scripts -r ~/shell_scripts
        setup perllib -r ~/perllib

        setup cosmology -r ~/exports/cosmology-local
        setup admom -r ~/exports/admom-local
        setup fimage -r ~/exports/fimage-local

        setup esutil -r ~/exports/esutil-local
        setup recfile -r ~/exports/recfile-local

        # columns, sdsspy are under mercurial
        setup mercurial
        setup columns -r ~/exports/columns-local
        setup sdsspy -r ~/exports/sdsspy-local
        setup numpydb -r ~esheldon/exports/numpydb-local


        # biggles requires plotutils
        setup biggles

        # for DES wl
        setup scons

        # will set up cfitsio/ccfits/tmv
        setup wl -r ~/exports/wl-local
        setup tmv -r ~/exports/tmv-work

        setup pv

        # note you still import scikits.learn
        setup scikits_learn -r ~/exports/scikits_learn

        return




        setup parallel

        setup vim

        setup tmux

        setup swig

        setup matplotlib
        #setup pyfitspatch
        setup pyfits



        # this will setupRequired the current idlutils, so we will call this first
        # and then setup up the trunk
        setup photoop -r ~/exports/photoop-trunk
        setup idlutils -r ~/exports/idlutils-trunk
        setup esidl -r ~/idl.lib/
        setup idlgoddard
        setup sdssidl -r ~/svn/sdssidl



        # clean this because in gdl_setup.sh we are concatenating
        # GDL_PATH and IDL_PATH
        #export GDL_PATH=""
        #setup gdl
        #setup gdladd -r ~/gdladd

        # for cropping eps files
        setup epstool


        setup libtool
        setup gflags
        setup stomp -r ~/exports/stomp-work



        if [ "$hname" == "tutti" ]; then
            setup pgnumpy -r ~esheldon/exports/pgnumpy-tutti
        else
            setup pgnumpy -r ~esheldon/exports/pgnumpy-local
        fi

        setup openssh

    fi # not tutti
fi
