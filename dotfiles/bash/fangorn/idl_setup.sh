main_setup=/Applications/itt/idl/bin/idl_setup.bash
if [ -e $main_setup ]; then
	source $main_setup

	export IDL_STARTUP=$dotfileDir/idl_startup

	SDSSIDL_SETUP_DIR=~/local/idl/sdssidl_setup
	if [ -e ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh ]; then
		source ${SDSSIDL_SETUP_DIR}/sdssidl_setup.sh

		IDL_PATH=${IDL_PATH}:+~/idl.lib/pro
		IDL_PATH=${IDL_PATH}:+~/local/idl/sdssidl/pro
		IDL_PATH=${IDL_PATH}:+~/local/idl/astrolib/pro

		export IDL_PATH

		# Config files
		export ESHELDON_CONFIG=${HOME}/.dotfiles/idl_config/esheldon_setup.config

		if [[ $?IDL_DLM_PATH == 0 ]]; then
			export IDL_DLM_PATH="<IDL_DEFAULT>":~/idl.lib/src/DLM:~/local/idl/sdssidl/src/DLM
		else
			export IDL_DLM_PATH=${IDL_DLM_PATH}:~/idl.lib/src/DLM:~/local/idl/sdssidl/src/DLM
		fi

	fi
fi


