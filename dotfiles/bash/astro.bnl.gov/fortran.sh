if [ "$INTEL_LICENSE_FILE" == "" ]; then
    f="/home/users/anze/local/intel/Compiler/11.1/059/bin/ifortvars.sh"
    if [[ -e $f ]]; then
        source  $f intel64
    fi
fi
