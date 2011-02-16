export PRODUCTS_DIR=/home/esheldon/idl_libraries
alias products='cd $PRODUCTS_DIR'

export SDSSIDL_SETUP_DIR=${PRODUCTS_DIR}/sdssidl_setup
export IDL_STARTUP=$dotfileDir/idl_startup

idlf=/usr/local/rsi/idl/bin/idl_setup.bash
if [ -f $idlf ]; then
    source /usr/local/rsi/idl/bin/idl_setup.bash
fi

export DATA=/global/data
export SDSSDATA=$DATA/sdss
export PHOTO_DATA=$SDSSDATA/imaging
export PHOTO_REDUX=$SDSSDATA/redux
export PHOTO_RESOLVE=$SDSSDATA/redux/resolve/full_02apr06
export PHOTO_CALIB=$SDSSDATA/redux/resolve/full_02apr06
export SPECTRO_DATA=$SDSSDATA/spectro
export DUST_DIR=$DATA/sfddust
export VAGC_REDUX=$DATA/vagc-dr4/vagc0
export LSS_REDUX=$SDSSDATA/lss

#export EUPS_SETUP_FILE=$HOME/idl_libraries/princeton/share/eups/bin/setups.sh

#if [[ -f $EUPS_SETUP_FILE ]]; then
#    source $EUPS_SETUP_FILE

#    setup idlutils
#    setup kcorrect
#    #setup photoop
#    #setup dimage
#    NYU_IDL_PATH=$IDL_PATH
#    IDL_PATH=""
#else
#    NYU_IDL_PATH=""
#fi

pdir=$HOME/idl_libraries/princeton
export IDLUTILS_DIR=$pdir/idlutils
export KCORRECT_DIR=$pdir/kcorrect
export PHOTOOP_DIR=$pdir/photoop

NYU_IDL_PATH=+$IDLUTILS_DIR/pro:+$PHOTOOP_DIR/pro:+$KCORRECT_DIR/pro

# SDSSIDL

if [ -f ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh ]; then
    source ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh
fi
SDSSIDL_IDL_PATH=$IDL_PATH

# My stuff
MY_IDL_PATH=+~/idl.lib/pro:+~/sdssidl/pro

# goddard routines
GODDARD_IDL_PATH=+~/idl_libraries/astrolib/pro

# ryan's routines
RYAN_IDL_PATH=+~/idl_libraries/ryan_idl

# mpfit
MPFIT_PATH=+~/idl_libraries/mpfit

# Now the path, putting things in their place
IDL_PATH="<IDL_DEFAULT>"
IDL_PATH=${IDL_PATH}:${MY_IDL_PATH}
IDL_PATH=${IDL_PATH}:${SDSSIDL_IDL_PATH}
IDL_PATH=${IDL_PATH}:${GODDARD_IDL_PATH}
IDL_PATH=${IDL_PATH}:${NYU_IDL_PATH}
IDL_PATH=${IDL_PATH}:${RYAN_IDL_PATH}
IDL_PATH=${IDL_PATH}:${MPFIT_PATH}

export IDL_PATH

export PATH=$IDLUTILS_DIR/bin:$KCORRECT_DIR/bin:$PHOTOOP_DIR/bin:$PATH

# Config files
export ESHELDON_CONFIG=${SDSSIDL_SETUP_DIR}/esheldon.conf

#IDL_DLM_PATH=~/sdssidl/src/DLM
if [[ $?IDL_DLM_PATH == 0 ]]; then
   export IDL_DLM_PATH="<IDL_DEFAULT>":~/idl.lib/src/DLM
else
   export IDL_DLM_PATH=${IDL_DLM_PATH}:~/idl.lib/src/DLM
fi


