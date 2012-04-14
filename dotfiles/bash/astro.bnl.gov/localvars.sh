# sets up a ton of stuff

f=~astrodat/setup/setup.sh
if [[ -e $f ]]; then
    source "$f"
fi

# all these require mount of /opt/astro/....
# tutti does not mount these
f=/opt/astro/SL53/bin/setup.astro.sh
if [[ -e $f ]]; then
    source "$f"

    f=~astrodat/setup/setup-wq.sh
    source "$f"

    f=/opt/astro/SL53/bin/setup.hadoop.sh
    source "$f"

    # make want to change to not have hadoop at the end?
    append_path C_INCLUDE_PATH $HADOOP_HOME/src/c++/libhdfs/
    append_path CPATH $HADOOP_HOME/src/c++/libhdfs/
    append_path LD_LIBRARY_PATH $HADOOP_HOME/c++/Linux-amd64-64/lib/
    append_path LIBRARY_PATH $HADOOP_HOME/c++/Linux-amd64-64/lib/

fi

if [[ $(hostname) != "tutti.astro.bnl.gov" ]]; then
    source ~/local/des-oracle/setup.sh
fi

append_path C_INCLUDE_PATH /usr/java/jdk1.6.0_14/include
append_path CPATH /usr/java/jdk1.6.0_14/include
append_path C_INCLUDE_PATH /usr/java/jdk1.6.0_14/include/linux
append_path CPATH /usr/java/jdk1.6.0_14/include/linux

export MAXBCG_CATDIR=/astro/tutti1/esheldon/lensinputs-v1/maxbcg/catalog
export CLUSTERS_INPUT=/astro/tutti1/esheldon/clusters-input

export PHOTOZ_DIR=~esheldon/photoz
export SWEEP_REDUCE=~esheldon/sweep-reduce

export MASK_DIR=~esheldon/masks

export LENSDIR=~esheldon/lensing

export PIXEL_MASK_BASIC=pixel_mask_dr4_basic
export PIXEL_MASK_BOUND=pixel_mask_dr4_bound
export PIXEL_MASK_SIMPLE=pixel_mask_dr4_simple
export PIXEL_MASK_BASIC_PRINCETON=$MASK_DIR/pixel_mask_princeton_basic

# location we keep simulations of the regauss algorithm
export REGAUSSIM_DIR=~esheldon/lensing/regauss-sim
export REGAUSSIM_HDFS_DIR=hdfs:///user/esheldon/lensing/regauss-sim

export SGSEP_DIR=~esheldon/oh/star-galaxy-separation/


