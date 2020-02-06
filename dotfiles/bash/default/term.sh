complete -r

# return the current git branch, or "" if not in a git
# repo directory

git_branch() {
    b=$(git branch 2>/dev/null | grep '^*' | colrm 1 2)
    if [[ $b != "" ]]; then
        b=" ($b)"
    fi
    echo "$b"
}

function setp_simple
{
    PS1="[\u@\h \W] "
}

function setp
{

    PS1="[\u@\h \W\$(git_branch)] "

	# if [ -z $STY ]
    # then
    #     # we are not in a screen session
    #     PS1="[\u@\h \W\$(git_branch)] "
	# else
    #     PS1="\[\033k\033\\\][\u@\h \W\$(git_branch)] "
	# fi

}

# don't do this stuff in the linux console
if [[ $TERM != "linux" ]]; then
    if [[ $sysname == "Darwin" ]]; then
        export TERM=xterm-color
    else
        case $TERM in
            screen*)
                # This is so screen can put the current process in the window name
                export TERM=screen-256color ;;
            rxvt*)
                export TERM=xterm-256color ;;
            xterm*)
                export TERM=xterm-256color ;;
            st*)
                export TERM=xterm-256color ;;
            *) ;;
        esac
    fi
fi


unset PROMPT_COMMAND
setp
