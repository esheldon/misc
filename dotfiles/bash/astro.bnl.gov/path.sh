append_path PATH /sbin
append_path PATH /usr/sbin

localdir=$HOME/local

prepend_path PATH /opt/astro/SL64/texlive/2013/bin/x86_64-linux/
prepend_path PATH ${localdir}/bin

prepend_path LD_LIBRARY_PATH ${localdir}/lib
prepend_path LIBRARY_PATH    ${localdir}/lib
prepend_path C_INCLUDE_PATH  ${localdir}/include
prepend_path CPATH           ${localdir}/include

prepend_path PATH $HOME/perllib
prepend_path PATH $HOME/shell_scripts

export ANACONDA_DIR=/gpfs02/astro/desdata/esheldon/miniconda3
if [[ -e $ANACONDA_DIR ]]; then
    prepend_path PATH ${ANACONDA_DIR}/bin
fi
