source /clusterfs/riemann/software/itt/idl/bin/idl_setup.bash

export IDL_DLM_PATH=""
export IDL_PATH=""


IDL_PATH="<IDL_DEFAULT>"
IDL_DLM_PATH="<IDL_DEFAULT>"



export PRODUCTS_DIR=~esheldon/local/products

#export SDSSIDL_SETUP_DIR=$PRODUCTS_DIR/sdssidl

export IDL_STARTUP=$dotfileDir/idl_startup

# UPS
export EUPS_FLAVOR=`uname`;
export EUPS_ROOT=/home/products/eups
source $EUPS_ROOT/bin/setups.sh

#setup sas bosswork
#setup tree
setup kcorrect


# my versions setup later will go to front
#setup bosstile -r ~esheldon/svn/bosstile
setup bosstile -r ~esheldon/exports/bosstile-work
setup bosstilelist -r ~esheldon/exports/bosstilelist-work
setup photoop  v1_10_15
#setup idlutils -r ~esheldon/exports/idlutils-local
setup idlutils v5_4_21

#setup bosstarget -r ~esheldon/exports/bosstarget-work
setup bosstarget -r ~esheldon/exports/bosstarget-local

# for some reason have to do it twice
setup tree dr8

setup sdssidl -r ~esheldon/sdssidl

setup esidl -r ~esheldon/idl.lib

export SDSSIDL_CONFIG=~/.idl_config/sdssidl.config

# on riemann the default is the sgc test stuff

export PHOTO_SWEEP=$PHOTO_SWEEP_BASE/dr8_final
# until the dr8 stuff is restored
#export PHOTO_RESOLVE=$PHOTO_RESOLVE_BASE/2010-05-23
export PHOTO_RESOLVE=/clusterfs/riemann/raid006/bosswork/groups/boss/resolve/2010-05-23
#export PHOTO_CALIB=$PHOTO_CALIB_BASE/dr8_final
export PHOTO_CALIB=/clusterfs/riemann/raid006/bosswork/groups/boss/calib/dr8_final
export BOSS_TARGET=/clusterfs/riemann/raid008/bosswork/groups/boss/target

# this is setup by setup sas bosswork, I'm resetting it for convenience
#export BOSS_TARGET=/clusterfs/riemann/raid006/bosswork/groups/boss/target/esheldon/sgc_test2

export MMT_PLATE_DIR=/mount/early2/esheldon/mmt-plates
export MMT_PLATE_CENTER_NAME=mmt-plate-data.dat



export REGAUSSIM_DIR=~esheldon/oh/regauss-sim
export SWEEP_REDUCE=~esheldon/oh/sweep-reduce


#IDL_PATH=+~/idl.lib/pro:${IDL_PATH}

#if [[ $?IDL_DLM_PATH == 0 ]]; then
#   IDL_DLM_PATH="<IDL_DEFAULT>":~/idl.lib/src/DLM
#else
#   IDL_DLM_PATH=${IDL_DLM_PATH}:~/idl.lib/src/DLM
#fi


export IDL_PATH
export PATH
export IDL_DLM_PATH

# Config files
export ESHELDON_CONFIG=${HOME}/.idl_config/esheldon.conf


# need to recompile all this
# actually python stuff
#setup readline -r ~esheldon/exports/readline/5.2
#setup bzip2 -r ~esheldon/exports/bzip2/1.0.5

#setup tcl -r ~esheldon/exports/tcl/8.5.8
#setup tk -r ~esheldon/exports/tk/8.5.8


#setup numpy -r ~esheldon/exports/numpy-trunk
#setup scipy -r ~esheldon/exports/scipy-trunk

#setup libpng -r ~esheldon/exports/libpng/1.2.39
#setup matplotlib -r ~esheldon/exports/matplotlib/0.99.0

#setup ipython -r ~esheldon/exports/ipython/0.10

#setup esutil -r ~esheldon/exports/esutil-trunk

setup python -r ~esheldon/exports/python/2.6.6

setup espy -r ~esheldon/python

setup shell_scripts -r ~/shell_scripts
setup perllib -r ~/perllib

unset MANPATH

export BOSS_ROOT=/clusterfs/riemann/raid008/bosswork/groups/boss
export BOSS_TARGET=$BOSS_ROOT/target
