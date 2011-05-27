f=/astro/u/esheldon/local/products/eups/bin/setups.sh
if [[ -e $f ]]; then
    source $f

    # this will get numpy and python
    setup scipy

    setup ipython
    setup local -r ~/local
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



    return




    setup pv
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


fi
