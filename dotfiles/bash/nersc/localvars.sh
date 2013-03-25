echo "setting umask"
umask u=wrx,g=rx,o=r

if [[ $hname == "datatran" ]]; then
    source /usr/share/Modules/init/bash
fi
source ~/des-setup/setup.sh

export export CLUSTERSTEP=/global/project/projectdirs/des/wl/cluster-step
