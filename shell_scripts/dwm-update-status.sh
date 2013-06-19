#!/bin/bash

function getnet() {
    UP1=$( awk '/eth1/ { print $10 }' /proc/net/dev )
    DOWN1=$( awk '/eth1/ { print $2 }' /proc/net/dev )
    sleep 1
    UP2=$( awk '/eth1/ { print $10 }' /proc/net/dev )
    DOWN2=$( awk '/eth1/ { print $2 }' /proc/net/dev )

    let "DIFF_UP=$UP2-$UP1"
    let "DIFF_DOWN=$DOWN2-$DOWN1"

    let "UPK=DIFF_UP/1024"
    let "DOWNK=DIFF_DOWN/1024"

    printf "dn: %4dK/s up: %3dK/s" "$DOWNK" "$UPK"
}

function getcpu() {

    CPU1=(`cat /proc/stat | grep '^cpu '`) # Get the total CPU statistics.
    sleep 1
    CPU2=(`cat /proc/stat | grep '^cpu '`) # Get the total CPU statistics.

    unset CPU1[0]                          # Discard the "cpu" prefix.
    IDLE1=${CPU1[4]}                        # Get the idle CPU time.
    unset CPU2[0]                          # Discard the "cpu" prefix.
    IDLE2=${CPU2[4]}                        # Get the idle CPU time.
     
    # Calculate the total CPU time.
    TOTAL1=0
    for VALUE in "${CPU1[@]}"; do
        let "TOTAL1=$TOTAL1+$VALUE"
    done
    TOTAL2=0
    for VALUE in "${CPU2[@]}"; do
        let "TOTAL2=$TOTAL2+$VALUE"
    done


    # Calculate the CPU usage since we last checked.
    let "DIFF_IDLE=$IDLE2-$IDLE1"
    let "DIFF_TOTAL=$TOTAL2-$TOTAL1"
    let "DIFF_USAGE=(1000*($DIFF_TOTAL-$DIFF_IDLE)/$DIFF_TOTAL+5)/10"
    printf "CPU: %3d%%" "$DIFF_USAGE"
}

function getwlan() {
    WIRELESS=""
    #INTERFACE="wlan0"

    #read STATE < /sys/class/net/${INTERFACE}/operstate 2>/dev/null

    #if [[ ${STATE} == "up" ]]; then
    #  ESSID=$( /sbin/iwconfig $INTERFACE | egrep -o "ESSID:\".*\"" | sed -r 's/ESSID:"(.*)"/\1/' )
    #  SIGNAL=$( /sbin/iwconfig $INTERFACE | grep -oP "\d+\/\d+" | awk -F\/ '{ printf("%d%\n",$1/$2*100) }' )

    #  WIRELESS=" ${ESSID} ${SIGNAL} ·"
    #fi

    echo "$WIRELESS"
}

function getbattery() {
    #BATTERY=C1C5 ### <- Battery ID
    #
    #grep ' charged' /proc/acpi/battery/${BATTERY}/state 2>&1 >/dev/null && TIME_LEFT="" || DO_BATTERY=1

    #Wyłączam tymczasem:
    #DO_BATTERY=""

    #if [[ -n ${DO_BATTERY} ]]; then
    #  TIME_LEFT=`acpi -t | head -n1 | sed -r \
    #  's/^.*Battery....//; s/discharging, /-/; s/charging, /+/; s/,//g; s/ ([0-9][0-9]:[0-9][0-9]):[0-9][0-9] .*$/ \1 /'`
    #
    #  TIME_LEFT="${TIME_LEFT} · "
    #fi

    echo ""
}

while [ true ]; do

    WIRELESS=$(getwlan)

    NET=$(getnet)
    DATE=$(  LC_ALL=en_US date +'%a %b %d %I:%M%P' )

    CPU=$(getcpu)

    STATUS="${CPU} | ${WIRELESS}${NET} | ${DATE}"

    xsetroot -name "$STATUS"

done
