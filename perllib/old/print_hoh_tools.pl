
use Sort::Naturally;

######################################
# print the hash of hashes by column
######################################

sub print_hoh {

    use strict;

    my $nargs = @_;

    # the hash of hashes
    my $HoH = shift;

    
    my $options;
    if ($nargs > 1) {
	$options = shift;
    }

    my @keys;
    if ( exists($$options{keys}) ) {
	@keys = @{ $$options{keys} };
    } else {
	@keys = nsort keys %$HoH;
    }

    my @columns;
    if ( exists($$options{columns}) ) {
	@columns = @{ $$options{columns} };
    } else {

	#############################################
	# just use all the "columns" in the hash, 
	# sorted.  This only picks up the defined 
	# columns for the first key in keys
	#############################################

	my %thash = %{ $$HoH{$keys[0]} };
	@columns = nsort keys %thash;
    }

    my %formats;
    if ( exists($$options{formats}) ) {
	%formats = %{ $$options{formats} };
    } else {
	# If doing a header, then create formats from the column names
	# with string format.  Otherwise, no format
	foreach my $col (@columns) {
	    $formats{$col}{w} = length($col);
	    $formats{$col}{f} = "s";
	}
    }

    # Should we print a header?
    if ( exists($$options{header}) ) {
	if ($$options{header}) {
	    &print_hoh_header($HoH, {columns => [@columns], 
				     formats => {%formats} });
	}
    }
    
    foreach my $key (@keys) {
	# If not corrected, just print
	&print_hoh_columns($HoH, $key, \@columns, \%formats);
    }

}

sub print_hoh_columns {

    my $HoH = shift;
    my $key = shift;
    my $columns = shift;
    my $formats = shift;

    my $f;
    my $w;

    foreach my $col (@$columns) {
	
	if (exists($$HoH{$key}{$col})) {
	    
	    if (!exists($$formats{$col}{w})) {
		# default to string with width equal to its name
		$w = length($col);
		$f = "s";
	    } else {
		$w = $$formats{$col}{w};
		$f = $$formats{$col}{f};
	    }
	    printf("%${w}${f} ",$$HoH{$key}{$col});
	    
	} # make sure column actually exists
    } # loop over columns
    print "\n";


}

###############################
# Print a nice header
###############################

sub print_hoh_header {

    my $nargs = @_;
    my $HoH = shift;

    my $options;
    if ($nargs > 1) {
	$options = shift;
    }

    my @columns;
    if ( exists($$options{columns}) ) {
	@columns = @{ $$options{columns} };
    } else {

	#############################################
	# just use all the "columns" in the hash, 
	# sorted.  This only picks up the defined 
	# columns for the first key in keys
	#############################################

	my @keys = nsort keys %$HoH;
	my %thash = %{ $$HoH{$keys[0]} };
	@columns = nsort keys %thash;
    }

    my %formats;
    if ( exists($$options{formats}) ) {
	%formats = %{ $$options{formats} };
    } else {
	# If doing a header, then create formats from the column names
	# with string format.  Otherwise, no format
	foreach my $col (@columns) {
	    $formats{$col}{w} = length($col);
	    $formats{$col}{f} = "s";
	}
    }

    # get existing keys,just to choose one
    my @HoH_keys = (keys %$HoH);

    my $header="";
    foreach my $col (@columns) {
	if (exists($$HoH{$HoH_keys[0]}{$col})) {

	    if (!exists($formats{$col}{w})) {
		# default to string with width equal to its name
		$w = length($col);
	    } else {
		$w = $formats{$col}{w};
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
