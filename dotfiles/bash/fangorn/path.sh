# because of homebrew we want the paths for /usr/local in 
# front

PATH=/usr/local/bin:$PATH
C_INCLUDE_PATH=/usr/local/include:$C_INCLUDE_PATH
LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#MANPATH=usr/local/share/man:$MANPATH

append_path PATH /opt/local/bin
append_path PATH /opt/local/sbin

prepend_path PATH /Users/esheldon/local/bin

prepend_path PATH $GOROOT/bin

#prepend_path C_INCLUDE_PATH /sw/include
prepend_path C_INCLUDE_PATH /opt/local/include
#prepend_path LD_LIBRARY_PATH /sw/lib
prepend_path LD_LIBRARY_PATH /opt/local/lib
#prepend_path MANPATH /sw/share/man
#prepend_path MANPATH /opt/local/share/man

export PATH
#export MANPATH
export C_INCLUDE_PATH
export LD_LIBRARY_PATH
