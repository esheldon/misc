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

