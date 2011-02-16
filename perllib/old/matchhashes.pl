sub matchhashes_bykey {
    
    if (@_ < 2) {
	print "At least two arguments must be sent to matchhashes\n";
	&matchhashes_syntax;
	die;
    }

    my $doret=0;

    if (@_ > 2) {
	$doret=1;
    }

    my $hash1 = $_[0];
    my $hash2 = $_[1];
    my $matchnum = 0;

    # match on smallest hash
    my @keys1 = keys %$hash1;
    my @keys2 = keys %$hash2;

    my $num1 = @keys1;
    my $num2 = @keys2;

    my @matching_keys;
    

    if ($num1 < $num2) {
	
	foreach my $key (@keys1) {
	    if ( exists($$hash2{$key}) ) {
		push @matching_keys, $key;
		$matchnum +=1;
	    }
	}

    } else {
	foreach my $key (@keys2) {
	    if ( exists($$hash1{$key}) ) {
		push @matching_keys, $key;
		$matchnum +=1;
	    }
	}
    }

    if ($doret) {
	$_[2] = $matchnum;
    }
    return @matching_keys;
}

sub diffhashes_bykey {

    ## here treat first as the "truth" 
    if (@_ < 2) {
	print "At least two arguments must be sent to diffhashes\n";
	&diffhashes_syntax;
	die;
    }
    my $doret=0;

    if (@_ > 2) {
	$doret=1;
    }

    my $hash1 = $_[0];
    my $hash2 = $_[1];
    my $missnum = 0;

    my @missing_keys;
    	
    foreach my $key (keys %$hash1) {
	if ( !exists($$hash2{$key}) ) {
	    push @missing_keys, $key;
	    $missnum +=1;
	}
    }


    if ($doret) {
	$_[2] = $missnum;
    }
    return @missing_keys;
}

sub diffhashes_syntax {
    print '--Syntax: @result = &diffhashes(\%hash1, \%hash2 [, $nmiss])'."\n";
}
sub matchhashes_syntax {
    print '--Syntax: @result = &matchhashes(\%hash1, \%hash2 [, $nmatch])'."\n";
}


1
