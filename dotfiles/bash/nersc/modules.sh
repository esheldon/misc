if [[ $NERSC_HOST == "cori" ]]; then
    source $dotfileDir/modules-cori.sh
elif [[ $NERSC_HOST == "datatran" ]]; then
    source $dotfileDir/modules-datatran.sh
fi
