
if [[ $hname == "carver" ]]; then
    . /project/projectdirs/cmb/modules/carver/hpcports.sh
    hpcports gnu
    module load hpcp
elif [[ $hname == "hopper" ]]; then
    . /project/projectdirs/cmb/modules/hopper/hpcports.sh
    hpcports gnu
    module load hpcp
fi

module load use.own

# use specific versions in case different machines have different defaults
module load python-hpcp
module load numpy-hpcp
module load scipy-hpcp

# my privatemodules
module load esutil-ess/local

# this is for the module-install script
export MODULE_INSTALLS=/global/project/projectdirs/des/wl/modules/${hname}/module-installs
