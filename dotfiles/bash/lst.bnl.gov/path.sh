
LP=/opt/lsst/SL53/
newpath=$LP/bin
#newmanpath=$LP/man


if [ ${PATH:+1} ]; then # returns 1 (true) if it exists
    PATH=${PATH}:${newpath}
else
    PATH=${newpath}
fi

#if [ ${MANPATH:+1} ]; then # returns 1 (true) if it exists
#    MANPATH=${MANPATH}:${newmanpath}
#else
#    MANPATH=${newpath}
#fi




