alias ptp='ptpython -i ~/.ptpython/startup.py'
alias ptip='ptipython -i ~/.ptpython/startup.py'

alias mds9='ds9 -xpa local'

alias sattach='grabssh ; screen -DR'
alias tattach='grabssh ; tmux a -d'
alias fixssh='source $TMPDIR/fixssh'

alias webfaction='ssh esheldon.webfactional.com'

alias rssh='ssh esheldon@rssh.rhic.bnl.gov'

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

alias fmplayer='mplayer -fstype none'

alias setcorus="export http_proxy=http://192.168.1.140:3128"
alias unsetcorus="unset http_proxy"

alias ackp='ack --pager="less -R"'

alias grep='grep --color=auto'

alias startnx='/usr/NX/bin/nxplayer --session ~/nxsessions/session1'

alias yt="youtube-dl --restrict-filenames"

export slac="rhel6-64.slac.stanford.edu"
export lsstdev="lsst-dev01.ncsa.illinois.edu"
export comet="comet.sdsc.edu"
export bebop='ac.esheldon@bebop.lcrc.anl.gov'
export duke='login.duke.ci-connect.net'
export osg='login05.osgconnect.net'
export in2p3='cca.in2p3.fr'

alias pyprof="python -m cProfile -s time "
alias feh='feh --force-aliasing -B black '
alias mfeh='feh --force-aliasing -B black -g 2000x2000'

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
