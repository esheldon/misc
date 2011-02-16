# if on cheops1, use local socket
if [[ $hname != "cheops1" ]]; then
    export PGHOST=cheops1.uchicago.edu
fi

export PGDATABASE=sdss
export PGUSER=sdss
