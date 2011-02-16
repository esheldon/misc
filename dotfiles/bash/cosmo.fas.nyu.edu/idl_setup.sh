export PRODUCTS_DIR=/home/users/esheldon/idl_libraries
alias products='cd $PRODUCTS_DIR'

export SDSSIDL_SETUP_DIR=${PRODUCTS_DIR}/sdssidl_config
export IDL_STARTUP=$dotfileDir/idl_startup

idlf=/usr/local/rsi/idl/bin/idl_setup.bash
if [ -f $idlf ]; then
    source $idlf
fi

# The NYU stuff
# UPS
export EUPS_FLAVOR=`uname`;
if [[ $EUPS_FLAVOR == "Linux" ]]; then
    test64=`uname -m`
    if [[ $test64 == "x86_64" ]]; then
	    export EUPS_FLAVOR="Linux64"
    fi
fi

export DATA=/global/data
export SDSSDATA=$DATA/sdss
export PHOTO_DATA=$SDSSDATA/imaging
export PHOTO_SKY=$SDSSDATA/sky
export PHOTO_REDUX=$SDSSDATA/redux
export PHOTO_SWEEP=$SDSSDATA/redux/datasweep/dr7
export PHOTO_RESOLVE=$SDSSDATA/redux/resolve/dr7
export PHOTO_CALIB=$SDSSDATA/redux/resolve/full_02apr06/calibs/default2
export SPECTRO_DATA=$SDSSDATA/spectro
export DUST_DIR=$DATA/sfddust
export VAGC_REDUX=$DATA/vagc-dr7/vagc2
export LSS_REDUX=$SDSSDATA/lss
export bc03_dir=$DATA/specmodels/bc03



export IDL_DLM_PATH=""
export IDL_PATH=""
if [[ -f /global/data/products/EvilUPS/bin/setups.sh ]]; then
    source /global/data/products/EvilUPS/bin/setups.sh

    setup idlutils
    setup kcorrect
    setup photoop
    setup dimage
	setup tycho2
	setup idlspec2d
    NYU_IDL_PATH=$IDL_PATH
    IDL_PATH=""

	# I'm working on the idlutils trunk so I need to override that set
	# by "setup idlutils"
	export IDLUTILS_DIR=~/svn/idlutils
	NYU_IDL_PATH=+${IDLUTILS_DIR}/pro:${NYU_IDL_PATH}
else
    NYU_IDL_PATH=""
fi

export RASS_DIR=/mount/early2/esheldon/rass-data
export RASS_FSC_NAME=rass-fsc-1.0rxs.cat
export RASS_BSC_NAME=rass-bsc-1.4.2rxs.cat
export MMT_PLATE_DIR=/mount/early2/esheldon/mmt-plates
export MMT_PLATE_CENTER_NAME=mmt-plate-data.dat



# SDSSIDL
if [ -f ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh ]; then
    source ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh
fi
SDSSIDL_IDL_PATH=$IDL_PATH
IDL_PATH=""

# My stuff
MY_IDL_PATH=+~/idl.lib/pro:+~/sdssidl/pro

# goddard routines
GODDARD_IDL_PATH=+~/idl_libraries/astrolib/pro

# ryan's routines
SCRANTON_IDL_PATH=+~/idl_libraries/scranton_idl

# Now the path, putting things in their place
IDL_PATH=${IDL_PATH}:${MY_IDL_PATH}
IDL_PATH=${IDL_PATH}:${SDSSIDL_IDL_PATH}
IDL_PATH=${IDL_PATH}:${GODDARD_IDL_PATH}
IDL_PATH=${IDL_PATH}:${NYU_IDL_PATH}
IDL_PATH=${IDL_PATH}:${SCRANTON_IDL_PATH}

# temporary
export BOSSTARGET_DIR=~/svn/bosstarget
IDL_PATH=${IDL_PATH}:+$BOSSTARGET_DIR/pro
export BOSSTARGET_DATA=/clusterfs/riemann/raid006/bosswork/groups/boss/target/esheldon

IDL_PATH=`echo $IDL_PATH | sed "s/:<IDL_DEFAULT>:/:/g"`

if [[ `echo $IDL_PATH | grep IDL_DEFAULT` == "" ]]; then
	IDL_PATH="<IDL_DEFAULT>":$IDL_PATH
fi

export IDL_PATH
export PATH=$PATH:$BOSSTARGET_DIR/bin

# Config files
export ESHELDON_CONFIG=${HOME}/.idl_config/esheldon_setup_nyu.config

#IDL_DLM_PATH=~/sdssidl/src/DLM
if [[ $?IDL_DLM_PATH == 0 ]]; then
   export IDL_DLM_PATH="<IDL_DEFAULT>":~/idl.lib/src/DLM
else
   export IDL_DLM_PATH=${IDL_DLM_PATH}:~/idl.lib/src/DLM
fi

if [ $(echo $IDL_DLM_PATH | grep IDL_DEFAULT) == "" ]; then
	export IDL_DLM_PATH="<IDL_DEFAULT>:$IDL_DLM_PATH"
fi

export SDSS_TARGET_DIR='/mount/early1/bosstarget'
