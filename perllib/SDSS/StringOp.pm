
package    SDSS::StringOp;
require    Exporter;

use strict;
use String::Between;

our @ISA = qw(Exporter);
our @EXPORT = qw(padchar run2string string2run field2string stripe2string getver_from_header);
our $VERSION = 1.0;

sub padchar {

    my $input = shift;
    my $base = shift;

    my $base_len = length $base;

    my $input_len = length $input;

    if ($input_len > $base_len) {
	return $input;
    } else {
	return substr($base, 0, $base_len-$input_len) . $input;
    }

}

sub run2string {

    my $run = $_[0];
    my $base = "000000";
    my $baselength = 6;

    my $runlength = length $run;

    substr($base, 0, $baselength-$runlength) . $run;

}

sub string2run {

    $_[0] + 0;

}

sub stripe2string {

    my $stripe_str = &field2string($_[0]);

}

sub field2string {

    my $field = $_[0];
    my $base = "0000";
    my $baselength = 4;

    my $fieldlength = length $field;

    substr($base, 0, $baselength-$fieldlength) . $field;

}

sub getver_from_header {

    # take a 'v5_3_33 ' type and turn it into a float
    # note, we will still store it as a string though

    my $header = shift;
    my $vstring = shift;

    my $vers;
    my $tvers;
    my @vstr = grep { /$vstring/ } @$header;

    if (@vstr == 1) {
	
	$vers = &between($vstr[0], "'", "'");

	# remove the front 'v'
	$tvers = substr($vers, 1, length $vers);
	
	# make sure splits into v5_3_33 or something
	# else just return the string
	my @sp = split "_",$tvers;
	if (@sp == 3) {
	    $vers = "$sp[0]\.$sp[1]$sp[2]";
	    $vers =~ s/\s//g;
	}
	return $vers;

    } else {
	return '-1';
    }

}

1
