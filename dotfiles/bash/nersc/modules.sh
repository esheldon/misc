# the nersc system is messed up so I have to check if modules
# is loaded or not

if [[ $hname == "carver" || $hname == "hopper" ]]; then
    source $dotfileDir/modules-carver-hopper.sh
elif [[ $hname == "datatran" ]]; then
    source $dotfileDir/modules-datatran.sh
fi
