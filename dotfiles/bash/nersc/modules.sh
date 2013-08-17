function nsetup_ess() {
    if [[ $NERSC_HOST == "carver" ]]; then
        source $dotfileDir/modules-carver.sh
    elif [[ $NERSC_HOST == "hopper" ]]; then
        source $dotfileDir/modules-hopper.sh
    elif [[ $NERSC_HOST == "datatran" ]]; then
        source $dotfileDir/modules-datatran.sh
    fi
}
