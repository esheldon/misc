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

# local installs
localdir=~/local
if [[ -e "$localdir" ]]; then
	
	# Our local installs get precedence
	prepend_path PATH ${localdir}/bin
    prepend_path C_INCLUDE_PATH $localdir/include
    prepend_path CPATH $localdir/include
	prepend_path LD_LIBRARY_PATH ${localdir}/lib
	prepend_path LIBRARY_PATH ${localdir}/lib

fi


prepend_path PATH ~/shell_scripts
prepend_path PATH ~/perllib
prepend_path PATH ~/python/bin
prepend_path PYTHONPATH ~/python

append_path PATH /sbin
append_path PATH /usr/sbin

export PATH
export C_INCLUDE_PATH
export CPATH
export LD_LIBRARY_PATH
export LIBRARY_PATH
export PYTHONPATH

