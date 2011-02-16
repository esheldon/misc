# this -u for ifconfig doesn't work on linux
ip=$(/sbin/ifconfig -u | grep "inet " | grep -v "inet 127" | cut -d " " -f 2)

check=$(echo $ip | grep 130\.199\.15)
if [ "$check" == "$ip" ]; then
    iscorus="yes"
    export http_proxy=http://192.168.1.140:3128
    export ftp_proxy=ftp://ftpgateway.sec.bnl.local
else
    unset http_proxy
    unset ftp_proxy
fi

