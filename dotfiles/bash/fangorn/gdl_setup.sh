main_setup=/sw/etc/profile.d/gdl.sh
if [ -e $main_setup ]; then
	source $main_setup

	export GDL_STARTUP=$dotfileDir/gdl_startup

	# my svn checkout
	GDL_PATH=${GDL_PATH}:+~/idl.lib/pro

	# local installed libraries
	LOCAL_LIB_DIR=~/local/idl

	SDSSIDL_SETUP_DIR=~/local/idl/sdssidl_setup
	if [ -e ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh ]; then
		source ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh
	fi

	GDL_PATH=${GDL_PATH}:+${SDSSIDL_DIR}/pro
	GDL_PATH=${GDL_PATH}:+${LOCAL_LIB_DIR}/astrolib/pro

	export GDL_PATH
fi

export ESHELDON_CONFIG=${HOME}/.idl_config/esheldon-fangorn.conf
