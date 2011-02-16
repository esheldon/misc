#!/usr/bin/perl -w

sub rdmyascii {

    use HashOp;

    # first line must contain headers
    # following lines that do not have same number of
    # columns are ignored

    my $nargs = @_;

    my $file = shift;

    my $rekey=0;
    my @colkey;
    if ($nargs > 1) {
	my $options = shift;
	if ( exists($$options{keyby}) ) {
	    $rekey = 1;
	    @colkey = @{ $$options{keyby} };
	}
	
    }


    my %data;

    open(IN, $file) || die "Can't open file $file: $!\n";

    my $firstline = 1;
    my @columns;

    my $rownum = 0;
    my @split;
    my $line;
    my $numcols;

    while ($line = <IN>) {

	chomp($line);

	if ($firstline) {
	    # get headers
	    @columns = split " ", $line;
	    $numcols = @columns;
	    $firstline=0;
	} else {
	    
	    @split = split " ", $line;
	    if (@split == $numcols) {
		
		for(my $i=0; $i<$numcols; $i++) {
		    #loop over column data in this row
		    my $cname = $columns[$i];
		    $data{$rownum}{$cname} = $split[$i];
		}
		$rownum++;
	    } # correct number of columns

	} # Data rows
	

    }
    close(IN);

    ##############################################################
    # Either return the hash as-is or re-key it by requested
    # column values.  If so, we will delete the old hash as
    # we go to save memory
    ##############################################################

    if ($rekey) {
	my %ndata = &rekey_hoh_bycolumn(\%data, \@colkey, {delete=>1});
	return %ndata;
    } else {
	return %data;
    }

}

1
