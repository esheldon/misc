# fink stuff.  Sets paths and stuff.  We will override some
if [[ -f /sw/bin/init.sh ]]; then
   source /sw/bin/init.sh
fi

# also add library and include stuff here later
if [[ -e /sw ]]; then
    finktest=$(echo $C_INCLUDE_PATH | grep sw)
    if [[ "$finktest" == "" ]]; then
        #export $PATH=/sw/bin:$PATH
        export C_INCLUDE_PATH=$C_INCLUDE_PATH:/sw/include
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/sw/lib
    fi
fi
