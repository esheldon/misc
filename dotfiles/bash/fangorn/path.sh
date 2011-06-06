# paths in addition to those set up by eups

if [[ -e /opt/local ]]; then
    append_path PATH /opt/local/bin
    append_path PATH /opt/local/sbin
    prepend_path C_INCLUDE_PATH /opt/local/include
    prepend_path LD_LIBRARY_PATH /opt/local/lib
fi

if [[ -e /sw ]]; then
    prepend_path C_INCLUDE_PATH /sw/include
    prepend_path LD_LIBRARY_PATH /sw/lib
fi

export PATH
export C_INCLUDE_PATH
export LD_LIBRARY_PATH
