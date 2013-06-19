#!/bin/bash
#
# load work versions of esutil, desdb, and deswl

if [[ $# -lt 1 ]]; then
    echo "usage: source des-load type"
    echo "  type should be local or work"
    return
fi

type=$1

module unload esutil && module load esutil/${type}
module unload desdb && module load desdb/${type}
module unload deswl && module load deswl/${type}
