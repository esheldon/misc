setup local -r ~/local

setup mercurial
setup pv
setup parallel

setup vim

setup tmux

setup swig

setup scipy
setup ipython
setup matplotlib
#setup pyfitspatch
setup pyfits

setup scons


# this will setupRequired the current idlutils, so we will call this first
# and then setup up the trunk
setup photoop -r ~/exports/photoop-trunk
setup idlutils -r ~/exports/idlutils-trunk
setup esidl -r ~/idl.lib/
setup idlgoddard
setup sdssidl -r ~/svn/sdssidl


setup recfile -r ~/exports/recfile-local

# clean this because in gdl_setup.sh we are concatenating
# GDL_PATH and IDL_PATH
export GDL_PATH=""
setup gdl
setup gdladd -r ~/gdladd

# will set up cfitsio/ccfits/tmv
setup wl -r ~/exports/wl-local
setup tmv -r ~/exports/tmv-work

# for cropping eps files
setup epstool


setup libtool
setup gflags
setup stomp -r ~/exports/stomp-work

setup sdsspy -r ~/exports/sdsspy-local
setup espy -r ~/python
setup admom -r ~/exports/admom-local
setup fimage -r ~/exports/fimage-local

setup shell_scripts -r ~/shell_scripts
setup perllib -r ~/perllib

setup columns -r ~/exports/columns-local
setup numpydb -r ~esheldon/exports/numpydb-local
setup esutil -r ~/exports/esutil-local

if [ "$hname" == "tutti" ]; then
    setup pgnumpy -r ~esheldon/exports/pgnumpy-tutti
else
    setup pgnumpy -r ~esheldon/exports/pgnumpy-local
fi

setup openssh

setup gnuplotpy
setup biggles

