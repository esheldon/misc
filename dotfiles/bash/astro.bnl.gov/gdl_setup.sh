# assume idl_setup was called first

if [ ${GDL_PATH:+1} ]; then
	GDL_PATH=$GDL_PATH:$IDL_PATH
else
	GDL_PATH=$IDL_PATH
fi

# explicitly include the lib directory from IDL since the GDL library is fairly
# incomplete.  we payed for the license for this if nothing else.

GDL_PATH=$GDL_PATH:+$IDL_DIR/lib

export GDL_PATH
