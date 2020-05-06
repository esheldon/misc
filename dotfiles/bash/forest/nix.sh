if [ -e $HOME/.nix-profile/etc/profile.d/nix.sh ]; then

    # don't do it twice
    if [ -z ${NIX_PROFILES+x} ]; then
        . /home/esheldon/.nix-profile/etc/profile.d/nix.sh
    fi
fi
