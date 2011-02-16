
package    SDSS::HashOp;
require    Exporter;

use strict;
use SDSS::StringOp;

our @ISA = qw(Exporter);
our @EXPORT = qw(sort_bystripe rekey_bystripe rekey_byruncam rekey_byrun);
our $VERSION = 1.0;

sub sort_bystripe {

    my $narg = @_;

    my $RI = shift;

    my $byrun = 0;
    if ($narg > 1) { 
	my $dobyrun = shift;
	if ($dobyrun) {
	    $byrun = 1; 
	}
    }

    my %skeyhash;
    if ($byrun) {
	%skeyhash = &rekey_bystripe($RI, $byrun);
    } else {
	%skeyhash = &rekey_bystripe($RI);
    }

    my @stripe_keys;

    foreach my $key (sort keys %skeyhash) {
	push @stripe_keys, $skeyhash{$key};
    }

    return @stripe_keys;

}

sub rekey_bystripe {

    # first is stripes, second run, third rerun
    # fourth is optional, so can key by that as well

    # can use the output of this function as a new key
    # my %newkeys = &rekey_bystripe(\%RI);
    # for my $nkey (sort keys %newkeys) {
    #    my $tkey = $newkeys{$nkey};
    #    print "$RI{$tkey}{stripe} $RI{$tkey}{strip} ";
    #    print "$RI{$tkey}{run} $RI{$tkey}{rerun}
    # }

    my $narg = @_;

    my $RI = shift;

    my $byrun = 0;
    if ($narg > 1) { 
	my $dobyrun = shift;
	if ($dobyrun) {
	    $byrun = 1; 
	}
    }

    my %stripe_hash;

    foreach my $key (keys %$RI) {

	my $runstring = &run2string($$RI{$key}{run});
	my $stripestring = &stripe2string( $$RI{$key}{stripe} );
	
#	my $runstring = $$RI{$key}{run};
#	my $stripestring = $$RI{$key}{stripe};

	my $newkey = 
	    "${stripestring}-$$RI{$key}{strip}-${runstring}-$$RI{$key}{rerun}";
	if (! $byrun) {
	    
	    $newkey = "${newkey}-$$RI{$key}{camcol}";
	}
	$stripe_hash{$newkey} = $key;

    }

    return %stripe_hash;

}

sub rekey_byruncam {

    my $RI = shift;

    my %rchash;

    foreach my $key (keys %$RI) {

	my $runstring = &run2string($$RI{$key}{run});
	my $cam = $$RI{$key}{camcol};

	
	my $newkey = "${runstring}-${cam}";
	$rchash{$newkey} = $key;
    }

    return %rchash;

}

sub rekey_byrun {

    my $RI = shift;

    my %rhash;

    foreach my $key (keys %$RI) {
	my $newkey = &run2string($$RI{$key}{run});
	$rhash{$newkey} = $key;
    }

    return %rhash;

}

1
