# awk simplifies life
alias mailtail="tail -n 100 -f ~/.getmail/gmail.log  | awk '{print \$1,\$2,\$4,\$10}'"
alias astro='ssh esheldon@astro.physics.nyu.edu'
alias astrox='ssh -x esheldon@astro.physics.nyu.edu'
alias bias='ssh esheldon@bias.cosmo.fas.nyu.edu'
alias biasx='ssh -x esheldon@bias.cosmo.fas.nyu.edu'

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
alias llc='ls -lh --color=auto -B -I "*.pyc" -I "*.o"'

alias mv='mv -i' 
alias cp='cp -i' 
alias less='less -R'

alias gvim='gvim -geometry 90x55'

alias rsqlplus='rlwrap --always-readline sqlplus  pipeline/dc01user@desdb/prdes'

alias lib='cd ~/idl.lib/pro'
alias oh='cd ~/oh'

alias bt='btlaunchmanycurses.bittornado --display_interval 3 --max_upload_rate'

alias idlwrap='rlwrap --always-readline idl'

alias lbl='ssh scs-gw.lbl.gov'


alias ipythonl='ipython -colors LightBG'

alias 256='export TERM=xterm-256color'
alias 8='export TERM=xterm-color'

alias lscreen='screen -c ~/.dotfiles/screen/screenrc-lightbg'
alias lipython='ipython -colors LightBG'

alias fmplayer='mplayer -fstype none'

alias setcorus="export http_proxy=http://192.168.1.140:3128"
alias unsetcorus="unset http_proxy"

alias mrxvt10='mrxvt -xft -xftfn Monaco -xftsz 10'
alias lmrxvt10='mrxvt -cf ~/.dotfiles/mrxvt/mrxvtrc-lightbg -xft -xftfn Monaco -xftsz 10'

alias ackp='ack --pager="less -R"'

alias grep='grep --color=auto'

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


