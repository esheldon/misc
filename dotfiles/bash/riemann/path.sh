prepend_path()
{
    if ! eval test -z "\"\${$1}\""; then
        # input path variable exists
        eval "$1=$2:\$$1"
    else
        # input path variable does not yet exist
        eval "$1=$2"
    fi
}

append_path()
{
    if ! eval test -z "\"\${$1}\""; then
        # input path variable exists
        eval "$1=:\$$1:$2"
    else
        # input path variable does not yet exist
        eval "$1=$2"
    fi
}



prepend_path C_INCLUDE_PATH $HOME/local/include
prepend_path LD_LIBRARY_PATH $HOME/local/lib

export C_INCLUDE_PATH
export LD_LIBRARY_PATH 

prepend_path PATH $HOME/local/bin


