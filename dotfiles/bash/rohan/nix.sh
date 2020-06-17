if [ -e $HOME/.nix-profile/etc/profile.d/nix.sh ]; then

    # don't do it twice
    if [ -z ${NIX_PROFILES+x} ]; then
        . /home/esheldon/.nix-profile/etc/profile.d/nix.sh
    fi
    np='/home/esheldon/.nix-profile' 
    prepend_path C_INCLUDE_PATH ${np}/include
    prepend_path LD_LIBRARY_PATH ${np}/lib
fi
