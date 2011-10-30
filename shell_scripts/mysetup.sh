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
	echo "  types:  misc espy latex esidl fimage cosmology admom"
	exit 45
fi

type=$1


cd ~
if [[ ! -e git ]]; then
    mkdir git
fi

for type; do
    if [[ $type == "espy" ]]; then
        cd ~/git
        if [[ -e "$type" ]]; then
            echo "$type git directory already exists"
            exit 45
        fi
        echo "cloning espy"
        git clone git@github.com:esheldon/espy.git
        echo "  setting symlinks"
        cd ~
        ln -vfs ~/git/espy python

    elif [[ $type == "esidl" ]]; then
        cd ~/git

        if [[ -e "$type" ]]; then
            echo "$type git directory already exists"
            exit 45
        fi

        echo "cloning esidl"
        git clone git@github.com:esheldon/esidl.git
        echo "  setting symlinks"
        cd ~
        ln -vfs ~/git/esidl idl.lib
    elif [[ $type == "misc" ]]; then
        echo "cloning misc (dotfiles, etc)"
        cd ~/git

        if [[ -e "$type" ]]; then
            echo "$type git directory already exists"
            exit 45
        fi
        git clone git@github.com:esheldon/misc.git


        echo "  setting symlinks"
        cd ~
        ln -vfs ~/git/misc/ccode
        ln -vfs ~/git/misc/perllib
        ln -vfs ~/git/misc/shell_scripts
        ln -vfs ~/git/misc/dotfiles .dotfiles

        ln -vfs ~/.dotfiles/vim .vim
        ln -vfs ~/.dotfiles/vim/vimrc .vimrc

        ln -vfs ~/.dotfiles/mailcap .mailcap

        if [ -e .bashrc ]; then
            rm -f .bashrc
        fi
        if [ -e .bash_profile ]; then
            rm -f .bash_profile
        fi
        if [ -e .profile ]; then
            rm -f .profile
        fi
        ln -vfs ~/.dotfiles/bash/bashrc .bashrc
        ln -vfs ~/.dotfiles/bash/bash_profile .bash_profile
        ln -vfs ~/.dotfiles/inputrc .inputrc
        ln -vfs ~/.dotfiles/X/Xdefaults .Xdefaults
        ln -vfs ~/.dotfiles/conky/conkyrc.treebeard .conkyrc
        ln -vfs ~/.dotfiles/screen/screenrc .screenrc
        ln -vfs ~/.dotfiles/mrxvt/mrxvtrc .mrxvtrc
        ln -vfs ~/.dotfiles/Eterm .Eterm
        ln -vfs ~/.dotfiles/multitailrc .multitailrc
        ln -vfs ~/.dotfiles/xmonad .xmonad

        ln -vfs ~/.dotfiles/hg/hgignore .hgignore

        ln -vfs ~/.dotfiles/git/gitignore .gitignore
        ln -vfs ~/.dotfiles/git/gitconfig .gitconfig

        mkdir -p .config/fbpanel
        ln -vfs ~/.dotfiles/fbpanel/default .config/fbpanel/default

        ln -vfs ~/.dotfiles/fonts .fonts
        ln -vfs ~/.dotfiles/icons .icons

        if [ -e .fvwm ]; then
            newdir=".fvwm`date +"%Y%m%d%k%M%S"`"
            mv .fvwm $newdir
        fi
        ln -vfs ~/.dotfiles/fvwm .fvwm

        if [ ! -d .subversion ]; then
            mkdir .subversion
        fi

        ln -vfs ~/.dotfiles/svn/config .subversion/config
        ln -vfs ~/.dotfiles/svn/servers.corus .subversion/servers.corus
        ln -vfs ~/.dotfiles/svn/servers.inside .subversion/servers.inside
        ln -vfs ~/.dotfiles/svn/servers.outside .subversion/servers.outside

        ln -vfs ~/.dotfiles/proxy

        # the modmap won't work in the mac windows system
        if [ `uname` != 'Darwin' ]; then
            ln -vfs ~/.dotfiles/X/Xmodmap .Xmodmap
        else
            ln -vfs ~/.dotfiles/mrxvt/mrxvtrc.fangorn .mrxvtrc
            ln -vfs ~/.dotfiles/X/Xdefaults.fangorn .Xdefaults
        fi

        if [ ! -e .ssh ]; then
            mkdir .ssh
            chmod og-rx .ssh
        fi
        ln -vfs ~/.dotfiles/ssh/config .ssh/config

    else
        echo "cloning $type"
        cd ~/git

        if [[ -e "$type" ]]; then
            echo "$type git directory already exists"
            exit 45
        fi
        git clone git@github.com:esheldon/$type.git
    fi
done

