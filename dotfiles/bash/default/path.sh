# add to end of path
append_path()
{
    # if zero length, just set equal to the input
    if eval test -z "\$$1"; then 
        eval "$1=$2"
    else
        if ! eval test -z "\"\${$1##*:$2:*}\"" -o -z "\"\${$1%%*:$2}\"" -o -z "\"\${$1##$2:*}\"" -o -z "\"\${$1##$2}\"" ; then
            eval "$1=\$$1:$2"
        fi
    fi
}

# add to front of path
prepend_path()
{
    # if zero length, just set equal to the input
    if eval test -z "\$$1"; then 
        eval "$1=$2"
    else
        if ! eval test -z "\"\${$1##*:$2:*}\"" -o -z "\"\${$1%%*:$2}\"" -o -z "\"\${$1##$2:*}\"" -o -z "\"\${$1##$2}\"" ; then
            eval "$1=$2:\$$1"
        fi
    fi
}

export -f append_path
export -f prepend_path

return














# don't want to re-read this for screen
case $TERM in
	screen*) return ;;
	*) ;;
esac

prepend_path PATH /usr/local/bin

# local installs
if [ -e ~/local ]; then
	
	# Our local installs get precedence
	localdir=~/local
	
	prepend_path PATH ${localdir}/bin
    prepend_path C_INCLUDE_PATH $localdir/include
	prepend_path LD_LIBRARY_PATH ${localdir}/lib
	prepend_path LIBRARY_PATH ${localdir}/lib

fi

#PATH=${PATH}:~/python
#append_path PATH ~/python
append_path PATH ~/shell_scripts
append_path PATH ~/shell_scripts/sdss
append_path PATH ~/perllib
append_path PATH ~/perllib/sdss_inventory

export ROCK_DIST=~/rock
if [ -e $ROCK_DIST ]; then
    append_path PATH $ROCK_DIST/bin
fi

export PATH
export C_INCLUDE_PATH
export LD_LIBRARY_PATH
#export MANPATH
