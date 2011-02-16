
sub ascii2html {

    # ASCII-to-HTML converter

    open(IN,$_[0]) || die "can't open $_[0]\n";

    print "<html>\n" . '<body bgcolor="#ffffff" link="#0066ff" vlink="#009999" text="#000000">' . "\n<p>";

    foreach $line (<IN>) {
	chomp($line);
	
	$line =~ s/(\s{2,})/"&nbsp;" x length($1)/ge;
	
	$line =~ s/</&lt;/g;
	$line =~ s/>/&gt;/g;
	
	print "\n<p>" if ($line eq '');
	
	print "$line<br>\n";
    }
    
    print "</body>\n</html>\n";

}

sub asciitable2html {
    
    # first line must contain headers
    # following lines that do not have same number of
    # columns are ignored

    ## reprint header after this many lines
    my $linemod = 25;

    open(IN,$_[0]) || die "can't open $_[0]: $!\n";
    print "<html>\n" . '<body bgcolor="#ffffff" link="#0066ff" vlink="#009999" text="#000000">' . "\n<p>";
    
    chomp(my $date = `date`);

    my @colsplit;
    my @hdrsplit;
    my $numcols;
    my $firstline=1;
    my $linenum=1;
    foreach $line (<IN>) {
	if ($firstline) {
	    # get headers
	    @hdrsplit = split " ", $line;
	    $numcols = @hdrsplit;

	    print "<table border=1>\n";
	    print "<tr>";
	    foreach my $el (@hdrsplit) {
		print "<th>$el</th>";
	    }
	    print "</tr>\n";
	    $firstline=0;
	} else {
	    @colsplit = split " ", $line;
	    my $tnumcols = @colsplit;
	    
	    if (@colsplit == $numcols) {
		print "<tr>";
		foreach my $el (@colsplit) {
		    print "<td nowrap align=left>$el</td>";
		}
		print "</tr>\n";
	    }
	}
	## gets confusing if too many elements without a
	## reminder of headers!
	if ( ($linenum % $linemod) == 0) {
	    print "<tr>";
	    foreach my $el (@hdrsplit) {
		print "<th>$el</th>";
	    }
	    print "</tr>\n";
	}

	$linenum++;

    }
    print "</table>\n";
    print "<hr>\n";
    print "<b>Email: esheldon at cfcp.uchicago.edu</b>\n";
    print "<!-- hhmts start --> Last modified: $date <!-- hhmts end -->\n";
    print "</body>\n";
    print "</html>\n";

}


1
