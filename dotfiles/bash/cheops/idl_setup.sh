#
# Setup IDL on cheops cluster
#

export PRODUCTS_DIR=/net/cheops1/home/products
alias products='cd $PRODUCTS_DIR'

export SDSSIDL_SETUP_DIR=/net/cheops1/home/products/sdssidl_config
export IDL_STARTUP=$dotfileDir/idl_startup

#######################################################
# EvilUPS stuff
#######################################################

export PROD_DIR_PREFIX=${PRODUCTS_DIR}/PrincetonNYU
export PRODUCTS=${PROD_DIR_PREFIX}/ups_db
export EUPS_DIR=${PROD_DIR_PREFIX}/evilups
export EUPS_FLAVOR=`/bin/uname`
#source ${EUPS_DIR}/bin/setups.sh

source ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh

# keep a copy of ours
OLD_IDL_PATH=$IDL_PATH
IDL_PATH=""

#setup idlutils
#setup idlspec2d
#setup photoop
#setup kcorrect


################################################
# No way I'm letting evilsetup screw my path
################################################

if [[ ${IDL_PATH:+1} ]]; then
    IDL_PATH=${OLD_IDL_PATH}:${IDL_PATH}
else 
    IDL_PATH=${OLD_IDL_PATH}
fi


MY_IDL_PATH=+~/idl.lib/pro
MY_IDL_PATH=${MY_IDL_PATH}:+~/sdssidl/pro
MY_IDL_PATH=${MY_IDL_PATH}:+${PRODUCTS_DIR}/astrolib/pro/
MY_IDL_PATH=${MY_IDL_PATH}:+~/tmp/shapelets_massey/shapelets
MY_IDL_PATH=${MY_IDL_PATH}:+~/deproj
MY_IDL_PATH=${MY_IDL_PATH}:+/net/cheops1/home/davej/dave_idl
MY_IDL_PATH=${MY_IDL_PATH}:/net/cheops1/home/scranton/SDSS/IDL
MY_IDL_PATH=${MY_IDL_PATH}:+/net/cheops1/home/mckay/idl.lib/intrinsic_align

# My stuff goes first
export IDL_PATH="<IDL_DEFAULT>":${MY_IDL_PATH}:${IDL_PATH}

# Config files
#export MYIDL_CONFIG=${HOME}/.idl_config/myidl_setup_cheops.config
export ESHELDON_CONFIG=${HOME}/.idl_config/esheldon_setup_cheops.config

# blantons vagc code and other stuff
export VAGC_DIR=${PROD_DIR_PREFIX}/vagc_code/vagc
export IDL_PATH=${IDL_PATH}:+${VAGC_DIR}/pro
export VAGCDIR=/net/cheops1/data1/spectra/blanton/vagc/
export SDSS_VAGCDIR=/net/cheops1/data1/spectra/blanton/vagc/sdss/
export VAGC_REDUX=/net/cheops1/data1/spectra/blanton/vagc/vagc0/

# I will reset my path completely since I often have newer stuff from
# sdssidl checked out
export IDL_DLM_PATH="<IDL_DEFAULT>":~/idl.lib/src/DLM:~/ccode/test:~/ccode/objShear/idl:~/sdssidl/src/DLM

export ZDBASE=/net/cheops1/data4/sdss_database

