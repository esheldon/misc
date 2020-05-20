# Check out my git archive and put in place symlinks
# Usage:
#  bash mysetup.sh dotfiles  #  Just gets dotfiles
#  bash mysetup.sh basic # currently everything except latex/fortran/www
#  bash mysetup.sh all   # gets everything
#
# Note you'll have to link the xsession by hand, since it will depend on
# the machine.  e.g. ln -s ~/.dotfiles/.dotfiles/X/xinitrc.xmonad .xsession
# same for the xmonad.hs and xmobarrc
#

# Check out either all or just dotfiles
if [ $# -eq 0 ]; then
	echo "usage: mysetup.sh type1 type2 .. "
	echo "  types:  misc"
	exit 45
fi

type=$1


cd ~
if [[ ! -e git ]]; then
    mkdir git
fi

for type; do
    if [[ $type == "misc" ]]; then
        echo "cloning misc (dotfiles, etc)"
        pushd ~/git

        if [[ -e "$type" ]]; then
            echo "$type git directory already exists"
            exit 45
        fi
        git clone git@github.com:esheldon/misc.git
        popd

        echo "  setting symlinks"

        rm -f perllib
        ln -vfs git/misc/perllib
        rm -f shell_scripts
        ln -vfs git/misc/shell_scripts
        rm -f .dotfiles
        ln -vfs git/misc/dotfiles .dotfiles

        ln -vfs .dotfiles/python/pythonrc .pythonrc

        ln -vfs .dotfiles/ack/ackrc .ackrc

        rm -f .vim
        ln -vfs .dotfiles/vim .vim
        ln -vfs .dotfiles/vim/vimrc .vimrc

        ln -vfs .dotfiles/bash/bashrc .bashrc
        ln -vfs .dotfiles/bash/bash_profile .bash_profile
        ln -vfs .dotfiles/inputrc .inputrc
        ln -vfs .dotfiles/X/Xdefaults .Xdefaults
        ln -vfs .dotfiles/X/Xmodmap .Xmodmap
        ln -vfs .dotfiles/screen/screenrc-lightbg .screenrc
        ln -vfs .dotfiles/multitailrc .multitailrc

        ln -vfs .dotfiles/git/gitignore .gitignore
        ln -vfs .dotfiles/git/gitconfig .gitconfig

        rm -f .fonts
        ln -vfs .dotfiles/fonts .fonts
        rm -f .icons
        ln -vfs .dotfiles/icons .icons

        if [ ! -e .ssh ]; then
            mkdir .ssh
            chmod og-rx .ssh
        fi
        ln -vfs .dotfiles/ssh/config .ssh/config

    else
        echo "unknown type: $type"
    fi

done

