


function setp_simple
{
    PS1="[\u@\h \W] "
}

function setp_screen
{

    # time warner gives an annoying hostname, so just map all Darwins to fangorn
    if [[ $sysname == "Darwin" ]]; then
        hm="fangorn"
        local TITLEBAR='\[\033]0;$hm\007\]'
        TITLEBAR="${SCREEN_PROCESS}${TITLEBAR}"
        PS1="${TITLEBAR}[\u@$hm \W] "
    else
        local TITLEBAR='\[\033]0;\h\007\]'
        TITLEBAR="${SCREEN_PROCESS}${TITLEBAR}"
        PS1="${TITLEBAR}[\u@\h \W] "
    fi

}

function setp
{

    # time warner gives an annoying hostname, so just map all Darwins to fangorn
    if [[ $sysname == "Darwin" ]]; then
        PS1="[esheldon@fangorn \W] "
    elif [[ $TERM == "linux" ]]; then
        # console, no title bar
        PS1="[\u@\h \W] "
    else
        if [[ $hname == "astro0034" ]]; then
            PS1="[\u@astro \W] "
        else
            local TITLEBAR='\[\033]0;\h\007\]'
            TITLEBAR="${TITLEBAR}"
            PS1="${TITLEBAR}[\u@\h \W] "
        fi
    fi


}

SCREEN_PROCESS=''

case $TERM in
    screen*)
        # This is so screen can put the current process in the window name
        SCREEN_PROCESS='\[\033k\033\\\]' ;;
    *) ;;
esac

# don't do this stuff in the linux console
if [[ $TERM != "linux" ]]; then
    if [[ $sysname == "Darwin" ]]; then
        export TERM=xterm-color
    else
        case $TERM in
            screen*)
                # This is so screen can put the current process in the window name
                export TERM=screen-256color ;;
            xterm*)
                export TERM=xterm-256color ;;
            *) ;;
        esac
    fi
fi


setp
