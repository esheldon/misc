# if on cheops1, use local socket
if [[ $hname != "early" ]]; then
    export PGHOST=early.cosmo.fas.nyu.edu
fi

export PGDATABASE=sdss
export PGUSER=sdss
