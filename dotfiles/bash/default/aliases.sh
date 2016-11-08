# this is a way to have firefox use it's own gtk settings
# this is nice because I need to use a dark theme for fbpanel
# but dark themes never work right in firefox.
alias myfirefox='GTK2_RC_FILES="/usr/share/themes/Clearlooks/gtk-2.0/gtkrc" firefox'
alias mds9='ds9 -xpa local'

export treebeard=192.168.127.102

alias hdfs="hadoop fs"

alias sattach='grabssh ; screen -DR'
alias tattach='grabssh ; tmux a -d'
alias fixssh='source $TMPDIR/fixssh'

# awk simplifies life
alias mailtail="tail -n 100 -f ~/.getmail/gmail.log  | awk '{print \$1,\$2,\$4,\$10}'"
alias astro='ssh esheldon@astro.physics.nyu.edu'
alias astrox='ssh -x esheldon@astro.physics.nyu.edu'
alias bias='ssh esheldon@bias.cosmo.fas.nyu.edu'
alias biasx='ssh -x esheldon@bias.cosmo.fas.nyu.edu'

alias webfaction='ssh esheldon.webfactional.com'

alias treebeard='ssh esheldon@192.168.127.108'

alias des='ssh esheldon@deslogin.cosmology.illinois.edu'
alias desx='ssh -x esheldon@deslogin.cosmology.illinois.edu'

alias desdb='ssh esheldon@desdb.cosmology.illinois.edu'
alias desdbx='ssh -x esheldon@desdb.cosmology.illinois.edu'
alias desar='ssh -x esheldon@desar.cosmology.illinois.edu'


# these use tunnels.  A connection to the gateway rssh is created if not
# already established
alias tbach='setup-bach start && ssh tbach'
alias ttutti='setup-bach start && ssh ttutti'
alias tbachx='setup-bach start && ssh -x tbach'
alias ttuttix='setup-bach start && ssh -x ttutti'


alias rssh='ssh esheldon@rssh.rhic.bnl.gov'
alias rsshx='ssh -x esheldon@rssh.rhic.bnl.gov'

alias ls='ls --color=auto'
alias ll='ls --color=auto -lh'
alias la='ls --color=auto -a'
alias lla='ls --color=auto -lah'
alias lb='ls --color=auto -B -I "*.pyc"'
alias llb='ls -lh --color=auto -B -I "*.pyc"'
alias lc='ls --color=auto -B -I "*.pyc" -I "*.o" -I "*.dep"'
alias llc='ls -lh --color=auto -B -I "*.pyc" -I "*.o" -I "*.dep"'

alias mv='mv -i' 
alias cp='cp -i' 
alias less='less -R'

alias mgvim='GTK2_RC_FILES=~/.themes/Clearlooks-DarkOrange/gtk-2.0/gtkrc gvim'


alias lib='cd ~/idl.lib/pro'
alias oh='cd ~/oh'

alias bt='btlaunchmanycurses.bittornado --display_interval 3 --max_upload_rate'

alias idlwrap='rlwrap --always-readline idl'

alias lbl='ssh scs-gw.lbl.gov'


alias ipython5='ipython --TerminalInteractiveShell.editing_mode=vi'

alias 256='export TERM=xterm-256color'
alias 8='export TERM=xterm-color'

alias lscreen='screen -c ~/.dotfiles/screen/screenrc-lightbg'

alias fmplayer='mplayer -fstype none'

alias setcorus="export http_proxy=http://192.168.1.140:3128"
alias unsetcorus="unset http_proxy"

alias urxvtb='urxvt -fn "xft:peep"'
alias mrxvt10='mrxvt -xft -xftfn Monaco -xftsz 10'
alias lmrxvt10='mrxvt -cf ~/.dotfiles/mrxvt/mrxvtrc-lightbg -xft -xftfn Monaco -xftsz 10'

alias ackp='ack --pager="less -R"'

alias grep='grep --color=auto'

alias startnx='/usr/NX/bin/nxplayer --session ~/nxsessions/session1'

alias yt="youtube-dl --restrict-filenames"

export slac="rhel6-64.slac.stanford.edu"
export comet="comet.sdsc.edu"

alias pyprof="python -m cProfile -s time "

alias pcat=pygmentize
function pless() {
    pcat "$1" | less -R
}

function printpath
{
	echo -e $(echo $1 | sed 's/:/\\n/g')
}

function mydvips {
	# remove the .tex
	DVTMP=`echo $* | sed "s/.tex//"`
	latex $DVTMP
	dvips -t letter $DVTMP -o ${DVTMP}.ps
}

function mydvipdf {
	DVTMP=`echo $* | sed "s/.tex//"`
	latex $DVTMP
	dvips -t letter $DVTMP -o ${DVTMP}.ps
	ps2pdf ${DVITMP}.ps ${DVTMP}.pdf
}

#test
