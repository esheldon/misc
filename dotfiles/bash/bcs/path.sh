
PATH=${HOME}/local/bin

PATH=${PATH}:/usr/X11R6/bin
PATH=${PATH}:/usr/local/bin:
PATH=${PATH}:/bin
PATH=${PATH}:/usr/bin
PATH=${PATH}:/sbin
PATH=${PATH}:/usr/sbin
PATH=${PATH}:${HOME}/python
PATH=${PATH}:${HOME}/shell_scripts
PATH=${PATH}:${HOME}/shell_scripts/sdss
PATH=${PATH}:${HOME}/perllib
PATH=${PATH}:${HOME}/perllib/sdss_inventory
PATH=${PATH}:/usr/local/lib
PATH=${PATH}:/usr/local/mysql/bin

#PATH=${PATH}:/oracle/10.2.0/client_1/bin:/oracle/10.2.0/client_1/lib:$PATH
PATH=${PATH}:/oracle/11.1.0/bin:/oracle/11.1.0/lib

C_INCLUDE_PATH=/home/esheldon/local/include:$DES_PREREQ/include
LD_LIBRARY_PATH=/home/esheldon/local/lib:$DES_PREREQ/lib:/oracle/11.1.0/lib

export C_INCLUDE_PATH
export LD_LIBRARY_PATH
export PATH

