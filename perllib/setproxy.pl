#! /usr/bin/perl -w
#
#  Script to check IP address and, based on that, set the active
#  proxy file used by Firefox or other applications.  Also set
#  the servers file for subversion if requested.  This file is usually
#  kept in ~/.subversion/servers
#
# Three cases;
#
#  inside Lab  - wpad_inside.pac
#  on Corus    - wpad_corus.pac
#  outside Lab - wpad_outside.pac
#
# Assume that most applications will read the file at startup.
# If the application is running when the script is run, you will
# need to refresh or reload the pac file in the application 
#
# For svn it is similar except the names are
#
#  inside Lab  - servers.inside
#  on Corus    - servers.corus
#  outside Lab - servers.outside
#
#
use strict;

# pac files. set do_pac to 0 to skip
my $do_pac=1;
my $WPAD_Onsite = 'wpad_inside.pac';
my $WPAD_Offsite = 'wpad_outside.pac';
my $WPAD_Corus = 'wpad_corus.pac';
my $WPAD = 'wpad.pac';
my $WPAD_Active;

# subversion settings. Set do_svn to 0 to skip
my $do_svn=1;
my $svn_servers_onsite = 'servers.inside';
my $svn_servers_offsite = 'servers.outside';
my $svn_servers_corus = 'servers.corus';
my $svn_servers = 'servers';
my $svn_servers_active;

my $pac_dir = "/Users/esheldon/proxy";
my $svn_dir = "/Users/esheldon/.subversion";

my $ip;

#------------------------------------------------------------#
# here are at least two ways of getting the IP address on a Mac
#
# ifconfig -u | grep "inet " | grep -v 127 | cut -d " " -f 2
#
# or
#
# networksetup -getinfo AirPort 2>&1 | grep "^IP address" | cut -d ":" -f 2
# networksetup -getinfo Ethernet 2>&1 | grep "^IP address" | cut -d ":" -f 2
#
#  Since the networksetup method requires two calls, one for each
#  type of interface, use the first.  There can be a problem when
#  both the wireless and the wire are active.

$ip = `ifconfig -u | grep "inet " | grep -v "inet 127" | cut -d " " -f 2`;
chomp $ip;

# Known Corus subnets are 130.199.152, 153, and 155, there may be others
#  at the moment do one check for 130.19.15 - if this overlaps with an internal
#  subnet you might use, then you will have to make a more specific test

if ($ip =~ /130\.199\.15/)
  {
    $WPAD_Active = $WPAD_Corus;
	$svn_servers_active = $svn_servers_corus;
  }
elsif ($ip =~ /130\.199\./)
  {
    $WPAD_Active = $WPAD_Onsite;
	$svn_servers_active = $svn_servers_onsite;
  }
else
  {
    $WPAD_Active = $WPAD_Offsite;
	$svn_servers_active = $svn_servers_offsite;
  }

# go to directory  holding .pac files
if ($do_pac eq 1) {
	print("Doing pac\n");
	chdir $pac_dir;
	&setlink($WPAD_Active, $WPAD);
}

# Now svn
if ($do_svn eq 1) {
	print("Doing svn\n");
	chdir $svn_dir;
	&setlink($svn_servers_active, $svn_servers);
}
exit;

sub setlink {
	my $fname = $_[0];
	my $linkname = $_[1];

	if (readlink($linkname) ne $fname)
	{
		unlink $linkname;
		symlink $fname, $linkname;
	}
}


