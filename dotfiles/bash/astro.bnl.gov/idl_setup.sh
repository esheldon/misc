#tf=/usr/local/itt/idl/bin/idl_setup.bash
#if [ -f $tf ]; then
#	source $tf
#fi

#source ~/.idl_config/sdssidl_setup.sh

#if [ "$IDL_PATH" == "" ]; then
#	echo "setting idlpath to default"
#	IDL_PATH="<IDL_DEFAULT>"
#fi


#setup idlgoddard
#setup sdssidl -r ~/svn/sdssidl
#setup esidl -r ~/idl.lib/ups
#IDL_PATH=+${HOME}/idl.lib/pro:${IDL_PATH}

if [ "$(echo $IDL_PATH | grep IDL_DEFAULT)" == "" ]; then
	IDL_PATH=${IDL_PATH}:"<IDL_DEFAULT>"
fi

export IDL_PATH=$IDL_PATH:+~/exports/scranton

export IDL_PATH
export IDL_STARTUP=$dotfileDir/idl_startup

export ESHELDON_CONFIG=~esheldon/.idl_config/esheldon.conf
export SDSSIDL_CONFIG=~/.idl_config/sdssidl.config
