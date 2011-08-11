# will want a different one for tutti
f=~esheldon/local/Modules/3.2.8/init/bash
if [[ -e $f ]]; then
    # modules get installed under MODULE_INSTALLS/flavor
    # will need a different one for tutti, module-installs-tutti
    export MODULE_INSTALLS=~/local/module-installs
    source "$f"

    module load use.own

    module load parallel
    module load perllib
    module load shell_scripts
    module load espy

    module load desfiles

    module load local

    module load pyyaml
    
    # also loads pcre
    module load swig


    # these are local
    module load esutil
    module load recfile

    module load cosmology
    module load admom
    # need to move code into /fimage subdir because 2.7 has
    # a built-in stat module
    module load fimage

    module load numpydb

    module load sdsspy
    module load columns

    # this is currently python only
    module load stomp

    module load scikits_learn

    # todo local installs
    #
    # tmv
    # wl
    # 
    # weighting
    # objshear

fi


