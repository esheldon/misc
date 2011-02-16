sub rekey_hoh_bycolumn {

    my $nargs = @_;
    my $data = shift;
    my $key_columns = shift;

    my $delete = 0;
    if ($nargs > 2) {
	my $options = shift;
	if ( exists($$options{delete}) ) {
	    $delete = $$options{delete};
	}
    }

    # The new hash
    my %ndata;

    foreach my $okey (keys %$data) {

	# build the new key
	my $nkey = "";
	foreach my $col (@$key_columns) {

	    if ( exists($$data{$okey}{$col}) ) {
		if ($nkey eq "") {
		    $nkey = $$data{$okey}{$col};
		} else {
		    $nkey = "${nkey}-$$data{$okey}{$col}";
		}
	    } else {
		printf(STDERR "REKEY_HOH_BYCOLUMN: column \"$col\" ");
		printf(STDERR "not found at key \"$okey\": Ignoring\n");
	    }
	}

	# copy in the columns

	# make a copy of this subhash
	my @cols = keys %{ $$data{$okey} };
	foreach my $col (@cols) {
	    
	    # copy in the new key/col pair
	    $ndata{$nkey}{$col} = $$data{$okey}{$col};

	    if ($delete) {delete($$data{$okey}{$col})}
	} 
	if ($delete) {delete($$data{$okey})}

    }

    return %ndata;

}

sub sort_hoh_bycolumn {

    my $narg = @_;

    my $hash = shift;
    my $columns = shift;
    
    my $naturally = 0;
    if ($narg > 2) {
	my $config = shift;
	$naturally = $$config{naturally};
    }

    # re-key by dash-separated column values
    my %newkeyhash;

    foreach my $key (keys %$hash) {

	my $newkey="";
	foreach my $col (@$columns) {
	    # column must exist
	    if ( exists($$hash{$key}{$col}) ) {
		if ($newkey eq "") {
		    $newkey = $$hash{$key}{$col};
		} else {
		    $newkey = "${newkey}-$$hash{$key}{$col}";
		}
		$newkeyhash{$newkey} = $key;
	    } else {
		printf(STDERR "SORT_HOH_BYCOLUMN: column \"$col\" ");
		printf(STDERR "not found at key \"$key\": Ignoring\n");
	    }
	}
    }

    # Sort the keys naturally?
    my @skeys;
    if ($naturally) {
	use Sort::Naturally;
	@skeys = nsort keys %newkeyhash;
    } else {
	@skeys = sort keys %newkeyhash;
    }

    # Now we must return the original keys, sorted by
    # the new ones
    my @newkeys;
    foreach my $key (@skeys) {
	push @newkeys, $newkeyhash{$key};
    }

    return @newkeys;


}

sub remove_keys {

    my $hash = shift;
    my $keys = shift;

    foreach my $key (@$keys) {
	delete $$hash{$key};
    }

}

sub rename_keys {

    ## the values of this hash are the old keys
    my $hash = $_[0];
    my $newkey_hash = $_[1];

    my @newkeys = keys %$newkey_hash;
    my @oldkeys = keys %$hash;

    if (@newkeys != @newkeys) {
	die("keys must be same length as hash");
    }

    foreach my $key (@newkeys) {
	my $oldkey = $$newkey_hash{$key};
	$$hash{$key} = $$hash{ $oldkey };

	delete $$hash{ $oldkey };
    }

}

1
