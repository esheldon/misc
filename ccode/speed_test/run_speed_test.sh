#!/bin/sh

shortname=`uname -n`
shortname=`echo $shortname | sed "s/\..*//g"`

/usr/bin/time -f "$shortname %e" ./speed_test
