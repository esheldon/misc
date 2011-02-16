#if [ ${PYTHONPATH:+1} ]; then
#	PYTHONPATH=${HOME}/python:$PYTHONPATH
#else
#	PYTHONPATH=${HOME}/python
#fi

# add some local directories if they exist
l="2.3 2.4 2.5 2.6 2.7 2.8 2.9 3.0 3.1 3.2 3.3 3.4 3.5 3.6 3.7 3.8 3.9"
for python_vers in $l; do
	local_install=${HOME}/local/lib/python$python_vers/site-packages
	if [ -e $local_install ]; then
		PYTHONPATH=$local_install:$PYTHONPATH
	fi
done

export PYTHONPATH

#export PYTHONSTARTUP=${HOME}/.pythonrc.py
#export SDSSPY_CONFIG=~esheldon/python.global/sdsspy_config.py
#export NUMERIX=numpy

# For some reason -p sh and inputrc vi style don't play
# well together; oh well
#alias ipython='ipython -pylab -tk -p sh'
#alias ipython='ipython -pylab -tk'
alias pc='python -ic "from __future__ import division; from math import *"'
