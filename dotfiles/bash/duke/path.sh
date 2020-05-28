localdir=$HOME/local

prepend_path PATH ${localdir}/bin

prepend_path LD_LIBRARY_PATH ${localdir}/lib
prepend_path LIBRARY_PATH    ${localdir}/lib
prepend_path C_INCLUDE_PATH  ${localdir}/include
prepend_path CPATH           ${localdir}/include

espy_dir=$HOME/python
prepend_path PATH ${espy_dir}/bin
prepend_path PYTHONPATH ${espy_dir}

prepend_path PATH $HOME/perllib
prepend_path PATH $HOME/shell_scripts

export ANACONDA_DIR=$HOME/miniconda3
prepend_path PATH ${ANACONDA_DIR}/bin
