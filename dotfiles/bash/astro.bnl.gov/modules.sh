# sets up modules and loads afs, anaconda, astrodat and wq
#source /opt/astro/SL64/bin/setup.astro.sh

if [ -n ${MODULEPATH+1} ]; then
   unset MODULEPATH
fi

. /opt/astro/SL64/Modules/default/etc/profile.modules

# load the default modules here
module load afs
module load anaconda/gpfs
module load astrodat
module load wq

# for latex
module load TeX


#
# my stuff
#

module load use.own
module load tmv/0.72-static         # *
module load cfitsio/3370-static         # *
module load ccfits/2.4-static         # *
module load local      # *
module load perllib
module load shell_scripts

# python
module load espy/local
module load des-oracle

source activate conda-local
