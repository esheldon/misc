#setproxy_inside() {
#    export http_proxy=http://192.168.1.130:3128
#    export ftp_proxy=ftp://ftpgateway.sec.bnl.local
#}
#setproxy_corus() {
#    export http_proxy=http://192.168.1.140:3128
#    export ftp_proxy=ftp://ftpgateway.sec.bnl.local
#}

#if [ -e "/sbin/ifconfig" ] ; then
#    ip=$(/sbin/ifconfig  | grep 'inet ' | grep -v 'inet addr:127' | awk '{print $2}' | cut -d ':' -f 2)

#    check_corus=$(echo $ip | grep 130\.199\.175)
#    check_inside=$(echo $ip | grep 172\.16\.100)

#    if [ "$check_corus" == "$ip" ]; then
#        # corus
#        export http_proxy=http://192.168.1.140:3128
#        export https_proxy=http://192.168.1.140:3128
#        export ftp_proxy=ftp://ftpgateway.sec.bnl.local
#    elif [ "$check_inside" == "$ip" ]; then
#        export http_proxy=http://192.168.1.130:3128
#        export https_proxy=http://192.168.1.130:3128
#        export ftp_proxy=ftp://ftpgateway.sec.bnl.local
#    else
#        unset http_proxy
#        unset ftp_proxy
#    fi
#fi
