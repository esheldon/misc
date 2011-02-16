
package    SDSS::PrintTools;
require    Exporter;

use strict;
use Sort::Naturally;

our @ISA = qw(Exporter);
our @EXPORT = qw(rundb_columns rundb_colformat print_rundb_hash
		 print_rundb_columns print_rundb_header);
our $VERSION = 1.0;

#####################################################
# Some predefined sets of columns
#####################################################

sub rundb_columns {

    # The user sends the option list as a hash!

    my $nargs = @_;

    # default options

    my $all = 0;
    my $byrun = 0;
    my $bystripe = 0;
    my $docorr = 0;

    if ($nargs > 0) {
	my $options = shift;

	# undefined is untrue
	$all      = $$options{all};
	$byrun    = $$options{byrun};
	$bystripe = $$options{bystripe};
	$docorr   = $$options{docorr};
    }
    
    my @columns;

    my @stripecols = ("stripe", "strip" );
    my @runcols = ("run","rerun");

    # Should we add camcol?
    if (! $byrun) { 
	push @runcols, "camcol";
    }

    if ($bystripe) {
	push @columns, @stripecols, @runcols;
    } else {
	push @columns, @runcols, @stripecols;
    }
    
    if ($all) {
	push @columns,  ("n_tsObj",
			 "n_tsField",
			 "n_fpAtlas",
			 "n_fpM",
			 "n_psField",
			 "n_adatc",
			 
			 "tsObj_photo_v",
			 "fpAtlas_photo_v",
			 "adatc_photo_v",
			 "baye_ver",
			 "phtz_ver",
			 
			 "imagingDir",
			 "adatcDir",
			 
			 "host");
	return @columns;
    }

    if ($docorr) {
	push @columns, ("n_tsObj",
			"n_adatc",
			
			"adatcDir");
	return @columns;
    }
    
    push @columns, ("n_tsObj",
		    "n_tsField",
		    "n_fpAtlas",
		    "n_fpM",
		    "n_psField",
		    "n_adatc",
			
		    "imagingDir");

    return @columns;



}



#############################################################
# Formats for nice table output
#############################################################

sub rundb_colformat {

    my %colformat = (
		     "run"    => {"f"=>"d", "w"=>6},
		     "rerun"  => {"f"=>"d", "w"=>5},
		     "camcol" => {"f"=>"d", "w"=>6},
		     "stripe" => {"f"=>"d", "w"=>6},
		     "strip"  => {"f"=>"s", "w"=>5},
		     
		     "n_tsObj"   => {"f"=>"d", "w"=>7},
		     "n_tsField" => {"f"=>"d", "w"=>9},
		     "n_fpAtlas" => {"f"=>"d", "w"=>9},
		     "n_fpM"     => {"f"=>"d", "w"=>5},
		     "n_psField" => {"f"=>"d", "w"=>9},
		     "n_adatc"   => {"f"=>"d", "w"=>7},
		     
		     "tsObj_photo_v"   => {"f"=>"f", "w"=>13.3},
		     "fpAtlas_photo_v" => {"f"=>"f", "w"=>15.3},
		     "adatc_photo_v"   => {"f"=>"f", "w"=>13.3},
		     "baye_ver"        => {"f"=>"f", "w"=>8.3},
		     "phtz_ver"        => {"f"=>"f", "w"=>8.3},
		     
		     "imagingDir" => {"f"=>"s", "w"=>45},
		     "adatcDir"   => {"f"=>"s", "w"=>45},
		     
		     "host" => {"f"=>"s", "w"=>10}
		     );
    
    return %colformat;

}

######################################
# print the hash of hashes by column
######################################

sub print_rundb_hash {

    my $nargs = @_;

    my $RI = shift;
    my $columns = shift;
    my $formats = shift;

    my $docorr = 0;
    my $notcorr = 0;

    my @keys;

    if ($nargs > 3) {
	my $options = shift;
	
	$docorr = $$options{docorr} + 0;
	$notcorr = $$options{notcorr} + 0;
	if ( exists($$options{keys}) ) {	    
	    @keys = @{ $$options{keys} };
	} else {
	    @keys = nsort keys %$RI;
	}

    } else {
	#print "Using sorted RI keys\n";
	@keys = nsort keys %$RI;
    }
    
    foreach my $key (@keys) {

	if ($docorr) {
	    # If corrected, there are two cases
	    if ($notcorr && ($$RI{$key}{n_adatc} < $$RI{$key}{n_tsObj})
		) {
		&print_rundb_columns($RI, $key, $columns, $formats);
	    } elsif ( ( !$notcorr) && 
		      ( $$RI{$key}{n_adatc} == $$RI{$key}{n_tsObj}) && 
		      ( $$RI{$key}{n_adatc} > 0 ) ) {
		&print_rundb_columns($RI, $key, $columns, $formats);
	    }
	} else {
	    # If not corrected, just print
	    &print_rundb_columns($RI, $key, $columns, $formats);
	}

    }


}

###############################
# Print the requested columns
###############################

sub print_rundb_columns {

    my $RI = shift;
    my $key = shift;
    my $columns = shift;
    my $formats = shift;

    my $f;
    my $w;
    foreach my $col (@$columns) {
	
	if (exists($$RI{$key}{$col})) {
	    if (!exists($$formats{$col}{w})) {
		# default to string with width equal to its name
		$w = length($col);
		$f = "s";
	    } else {
		$w = $$formats{$col}{w};
		$f = $$formats{$col}{f};
	    }
	    printf("%${w}${f} ",$$RI{$key}{$col});
	    
	} # make sure column actually exists
    } # loop over columns
    print "\n";

}

###############################
# Print a nice header
###############################

sub print_rundb_header {

    my $RI = shift;
    my $columns = shift;
    my $formats = shift;

    # get existing keys,just to choose one
    my @RI_keys = (nsort keys %$RI);

    my $header="";
    foreach my $col (@$columns) {
	if (exists($$RI{$RI_keys[0]}{$col})) {

	    my $w;
	    if (!exists($$formats{$col}{w})) {
		# default to string with width equal to its name
		$w = length($col);
	    } else {
		$w = $$formats{$col}{w};
	    }


	    # round off the widths for strings
	    my $cw = sprintf("%.0f",$w);
	    $header = $header . sprintf("%${cw}s ", $col);
	}
    }
    
    # print it
    print "$header\n";
    
    # print the dashes
    my $hlen = length $header;
    for (my $i=0; $i < $hlen; ++$i) {
	print "-";
    }
    print "\n";
    
}

1
