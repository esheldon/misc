# assume idl_setup was called first

GDL_PATH=$IDL_PATH

# explicitly include the lib directory from IDL since the GDL library is fairly
# incomplete.  we payed for the license for this if nothing else.

append_path GDL_PATH +$IDL_DIR/lib

export GDL_PATH
