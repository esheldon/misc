#!/bin/bash

print_memory() {
    usage=$(free -h | awk '/^Mem:/ {print $3 "/" $2}')
    echo -ne "mem ${usage}"
}

print_power2() {
    status="$(cat /sys/class/power_supply/AC/online)"
    battery="$(cat /sys/class/power_supply/BAT0/capacity)"
    timer="$(acpi -b | grep "Battery" | awk '{print $5}' | cut -c 1-5)"
    if [ "${status}" == 1 ]; then
        echo -ne "AC ON  ${battery}%"
    else
        echo -ne "AC OFF ${battery}%"
    fi
}

print_power() {
	capacity=$(cat /sys/class/power_supply/BAT0/capacity) || exit
	status=$(cat /sys/class/power_supply/BAT0/status)

	if [ "$capacity" -ge 75 ]; then
		color="#00ff00"
	elif [ "$capacity" -ge 50 ]; then
		color="#ffffff"
	elif [ "$capacity" -ge 25 ]; then
		color="#ffff00"
	else
		color="#ff0000"
		warn="❗"
	fi

	[ -z $warn ] && warn=" "

	[ "$status" = "Charging" ] && color="#ffffff"

	# printf "<span color='%s'>%s%s%s</span>\n" "$color" "$(echo "$status" | sed -e "s/,//;s/Discharging/🔋/;s/Not charging/🛑/;s/Charging/🔌/;s/Unknown/♻️/;s/Full/⚡/;s/ 0*/ /g;s/ :/ /g")" "$warn" "$(echo "$capacity" | sed -e 's/$/%/')"
	printf "%s%s%s" "$(echo "$status" | sed -e "s/,//;s/Discharging/🔋/;s/Not charging/🛑/;s/Charging/🔌/;s/Unknown/♻️/;s/Full/⚡/;s/ 0*/ /g;s/ :/ /g")" "$warn" "$(echo "$capacity" | sed -e 's/$/%/')"

}
print_wifiqual() {
    wifiessid="$(/sbin/iwconfig 2>/dev/null | grep ESSID | cut -d: -f2)"
    wifiawk="$(echo $wifiessid | awk -F',' '{gsub(/"/, "", $1); print $1}')"
    wificut="$(echo $wifiawk | cut -d' ' -f1)"
    echo -ne "${wificut}"
}

print_hddfree() {
    hddfree="$(df -Ph /dev/nvme0n1p2 | awk '$3 ~ /[0-9]+/ {print $4}')"
    echo -ne "disk ${hddfree}"
}

print_datetime() {
    datetime="$(date "+%a %d %b %I:%M")"
    echo -ne "${datetime}"
}

# cpu (from: https://bbs.archlinux.org/viewtopic.php?pid=661641#p661641)

while true; do
    # get new cpu idle and total usage
    eval $(awk '/^cpu /{print "cpu_idle_now=" $5 "; cpu_total_now=" $2+$3+$4+$5 }' /proc/stat)
    cpu_interval=$((cpu_total_now-${cpu_total_old:-0}))
    # calculate cpu usage (%)
    let cpu_used="100 * ($cpu_interval - ($cpu_idle_now-${cpu_idle_old:-0})) / $cpu_interval"

    cpu_used=$(printf "%3d" "$cpu_used")

    # output vars
    print_cpu_used() {
        printf "%-1b" "CPU${cpu_used}%"
    }

    # Pipe to status bar, not indented due to printing extra spaces/tabs
    xsetroot -name "$(print_cpu_used)|$(print_memory)|$(print_hddfree)|$(print_power)|$(print_datetime) "

    # reset old rates
    cpu_idle_old=$cpu_idle_now
    cpu_total_old=$cpu_total_now
    sleep 1
done
