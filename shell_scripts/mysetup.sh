# Check out my svn archive and put in place symlinks
# Usage:
#  bash mysetup.sh dotfiles  #  Just gets dotfiles
#  bash mysetup.sh basic # currently everything except latex/fortran/www
#  bash mysetup.sh all   # gets everything
#
# Note you'll have to link the xsession by hand, since it will depend on
# the machine.  e.g. ln -s .dotfiles/.dotfiles/X/xinitrc.xmonad .xsession
# same for the xmonad.hs and xmobarrc
#

# Check out either all or just dotfiles
if [ $# -eq 0 ]; then
	echo "usage: mysetup.sh type [svndir]"
	echo "  type:  dotfiles/basic/all"
	echo "  svndir: default ~/svn"
	exit 45
fi

type=$1

svndir=~/svn
if [ $# -eq 2 ]; then
	svndir=$2
fi

if [ -e $svndir ]; then
	echo "directory already exists $svndir"
	exit 45
fi

mkdir $svndir

if [ $type == "all" ]; then
	checkoutdir=$svndir/esheldon
	cd $svndir
	svn co svn+ssh://howdy.physics.nyu.edu/usr/local/svn/esheldon/trunk esheldon
elif [ $type == "basic" ]; then
	checkoutdir=$svndir
	cd $checkoutdir
	dirs="notes idl_config shell_scripts perllib python dotfiles help idl.lib ccode"
	for dir in $dirs; do
		svn co svn+ssh://howdy.physics.nyu.edu/usr/local/svn/esheldon/trunk/$dir $dir
	done
elif [ $type == "dotfiles" ]; then
	checkoutdir=$svndir
	cd $checkoutdir
	svn co svn+ssh://howdy.physics.nyu.edu/usr/local/svn/esheldon/trunk/dotfiles dotfiles
else
	echo "first argument must be 'basic' or 'all'"
	exit 45
fi

# links into ~
cd $checkoutdir
dirs=`ls -d *`
cd ~
for d in $dirs; do
	ln -fs $checkoutdir/$d
done

# these should be hidden
if [ -e dotfiles ]; then
	mv dotfiles .dotfiles
fi
if [ -e idl_config ]; then
	mv idl_config .idl_config
fi


# Individual dotfile links
ln -fs .dotfiles/vim .vim
ln -fs .dotfiles/vim/vimrc .vimrc
if [ -e .bashrc ]; then
	rm -f .bashrc
fi
if [ -e .bash_profile ]; then
	rm -f .bash_profile
fi
if [ -e .profile ]; then
	rm -f .profile
fi
ln -fs .dotfiles/bash/bashrc .bashrc
ln -fs .dotfiles/bash/bash_profile .bash_profile
ln -fs .dotfiles/inputrc .inputrc
ln -fs .dotfiles/X/Xdefaults .Xdefaults
ln -fs .dotfiles/conky/conkyrc.thin .conkyrc
ln -fs .dotfiles/screen/screenrc .screenrc
ln -fs .dotfiles/mrxvt/mrxvtrc .mrxvtrc
ln -fs .dotfiles/Eterm .Eterm
ln -fs .dotfiles/multitailrc .multitailrc
ln -fs .dotfiles/xmonad .xmonad

ln -fs .dotfiles/hg/hgignore .hgignore

mkdir -p .config/fbpanel
ln -fs .dotfiles/fbpanel/default .config/fbpanel/default

ln -fs .dotfiles/fonts .fonts
ln -fs .dotfiles/icons .icons

if [ -e .fvwm ]; then
	newdir=".fvwm`date +"%Y%m%d%k%M%S"`"
	mv .fvwm $newdir
fi
ln -fs .dotfiles/fvwm .fvwm

if [ ! -d .subversion ]; then
	mkdir .subversion
fi
ln -fs ~/.dotfiles/svn/config .subversion/config
ln -fs ~/.dotfiles/svn/servers.corus .subversion/servers.corus
ln -fs ~/.dotfiles/svn/servers.inside .subversion/servers.inside
ln -fs ~/.dotfiles/svn/servers.outside .subversion/servers.outside


# the modmap won't work in the mac windows system
if [ `uname` != 'Darwin' ]; then
	ln -fs ~/.dotfiles/X/Xmodmap .Xmodmap
else
	ln -fs ~/.dotfiles/mrxvt/mrxvtrc.fangorn .mrxvtrc
	ln -fs .dotfiles/X/Xdefaults.fangorn .Xdefaults
fi

if [ ! -e .ssh ]; then
	mkdir .ssh
	chmod og-rx .ssh
fi
ln -fs ~/.dotfiles/ssh/config .ssh/config


