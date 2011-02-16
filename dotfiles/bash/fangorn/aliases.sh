# gnu ls
#alias  ls='ls --color=auto'
#alias  ll='ls --color=auto -lh'
#alias  la='ls --color=auto -a'
#alias  lla='ls --color=auto -lah'
#alias  lb='ls --color=auto -B'

# bsd ls
alias  ls='ls -G'
alias  ll='ls -G -lh'
alias  la='ls -G -a'
alias  lla='ls -G -lah'

alias  mv='mv -i'
alias  cp='cp -i'
alias  less='less -R'

alias ssh='ssh -Y'

alias oh='cd /Volumes/LaCie\ Disk/data'
alias m2l='cd ~/latex/maxbcg_m2l'

alias top='top -o cpu -s 2'
alias gvim='gvim -geometry 80x50'

alias fetchmail='fetchmail --mda "procmail -f %F"'

# sudo loses the environment variable.  Use this to do port commands
# when at BNL
alias bnlport='sudo env http_proxy=http://192.168.1.140:3128 port'

# shortcut for quicklook
alias ql='qlmanage -p 2>/dev/null'

# have man output postscript and send it to Preview
pman () {
    man -t "${1}" | ps2pdf - - | open -f -a /Applications/Preview.app
}

