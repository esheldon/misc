#!/usr/bin/perl -w
# perl script that takes an ascii file list of commands 
# and makes it into a f95 namelist 
#       usage:
#          mknamelist.pl
#

$i = 0;

print "&listcmd\n";

while(<ARGV>) {

chomp;

$i = $i + 1;

$_="cmd($i)='$_\'\,\n";

print $_; 

}

print "/\n";
