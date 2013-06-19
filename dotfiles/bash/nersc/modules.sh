function nsetup_ess() {
    if [[ $NERSC_HOST == "carver" || $NERSC_HOST == "hopper" ]]; then
        source $dotfileDir/modules-carver-hopper.sh
    elif [[ $NERSC_HOST == "datatran" ]]; then
        source $dotfileDir/modules-datatran.sh
    fi
}
