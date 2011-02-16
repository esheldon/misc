 
package    SDSS::ReadYanny;
require    Exporter;

use strict;
use String::Between;

our @ISA = qw(Exporter);
our @EXPORT = qw(read_yanny);
our $VERSION = 0.1;

####################################################################
# Reads a "Yanny parameter file", which is an extremely flexible 
# ascii file format used by the SDSS.
#
# ignores enum definitions since they are unneeded in perl
# This is a problem if we want to WRITE rewrite the file....
####################################################################

sub read_yanny {

    my $nargs = @_;

    # Hash of hashes will contain all our data
    my %data;

    if ($nargs < 1) {
	printf(STDERR "Error (read_yanny): No file input!\n");
	return %data;
    }
    my $file = shift;

    # contains structure definitions
    # data will also contain a key called structs
    my %structs;
    my $in_structdef=0;
    my $in_enum=0;

    my @ln;

    open(IN, $file) || die "Error (read_yanny): Can't open $file: $!\n";

    ##################################################
    # Read file line-by-line
    ##################################################

    my $line;
    while ($line = <IN>) {

	chomp($line);

	###########################################################
	# only consider parts of the line *before* any comments: #
	###########################################################

	my $comm_index = index($line, "#");
	if ($comm_index != -1) {
	    $line = substr($line, 0, $comm_index);
	}

	###########################################################
	# Deal with possible continuation of lines
	###########################################################

	my $docont=0;
	my $cont_index = index($line, "\\");
	if ($cont_index != -1) {
	    $docont=1;
	    $line = substr($line, 0, $cont_index);
	}
	while ($docont) {
	    # This line continues onto the next

	    my $tline = <IN>;
	    chomp($tline);

	    # only consider parts of the line *before* any comments: #
	    my $comm_index = index($tline, "#");
	    if ($comm_index != -1) {
		$tline = substr($tline, 0, $comm_index);
	    }

	    # look for another continuation
	    $cont_index = index($tline, "\\");
	    if ($cont_index != -1) {
		$tline = substr($tline, 0, $cont_index);
	    } else {
		$docont=0;
	    }
	    $line = $line . $tline;
	}

	#################################
	# Split the line by white space
	#################################

	if(@ln = split " ",$line) {

	    my $firstword = $ln[0];
	    
	    # make lower case
	    $firstword =~ tr/[A-Z]/[a-z]/;
	    
	    if ($firstword eq "typedef") {
		
		################################################
		# Here begins a typedef statement. Which kind?
		################################################
		
		if ($ln[1] eq "struct") {
		    $in_structdef = 1;
		} elsif ($ln[1] eq "enum") {
		    $in_enum=1;
		} else {
		    printf(STDERR "Error (read_yanny): ");
		    printf(STDERR "Unknown typedef $ln[1]\n");
		}
		
		if ($in_structdef) {

		    ###############################################
		    # We will read in the typedef fully before 
		    # parsing. i.e. until we see the closing brace
		    ###############################################
		
		    while ($line !~ /\}/) {
			my $tline = <IN>;
			chomp($tline);
			$line = $line . $tline;
		    }
		    
		    # Isolate the typedef pairs
		    my $struct_def = &between($line, "{","}"); 

		    # get the struct name
		    my $w2 = index($line, "}");
		    my $struct_name = substr($line, $w2+1);

		    # remove white space
		    $struct_name =~ s/\s//g;
		    # remove the semicolon
		    $struct_name =~ s/\;//;

		    # make lower case
		    $struct_name =~ tr/[A-Z]/[a-z]/;
		    
		    ###################################################
		    # Now split the typedef string by the semicolons
		    # and then into pairs by white space
		    ###################################################

		    my @typedef_pairs = split ";", $struct_def;

		    my @coltype;
		    my @columns;
		    my @lengths;
		    foreach my $tdp (@typedef_pairs) {
			my @pair = split " ",$tdp;

			if (@pair == 2) {
			    my $ctype = $pair[0];
			    my $cname = $pair[1];
			    my $clen = 1;

			    my @tmp = split /\[/,$cname;
			    if (@tmp == 2) {
				# this is it
				$clen = $tmp[1];
				$clen =~ s/\]//;
			    }
			    $cname = $tmp[0];
			    @tmp = split /\</, $cname;
			    if (@tmp == 2) {
				# this is it
				$clen = $tmp[1];
				$clen =~ s/\]//;
			    }
			    $cname = $tmp[0];
			    
			    push @coltype, $ctype;
			    push @columns, $cname;
			    push @lengths, $clen;
			    
			}
		    }
		    
		    # Copy into the data and structs hashes

		    $structs{$struct_name}{types} = [@coltype];
		    $structs{$struct_name}{columns} = [@columns];
		    $structs{$struct_name}{lengths} = [@lengths];
		    $structs{$struct_name}{rownum} = 0;
		    
		    $data{structs}{$struct_name}{types} = [@coltype];
		    $data{structs}{$struct_name}{columns} = [@columns];
		    $data{structs}{$struct_name}{lengths} = [@lengths];

		} else {

		    ###################################################
		    # We are in an enum typedef or something unknown. 
		    # We will just skip it for now
		    ###################################################

		    while ($line !~ /\}/) {
			$line = <IN>;
		    }

		}
	    } elsif ( exists($structs{$firstword}) ) {

		#################################################
		# This is part of one of our data structures
		# Read the columns into the appropriate hash
		# no need to keep element zero, the struct name
		#################################################
		
		my $struct_name = $firstword;
		
		my @columns = @{ $structs{$struct_name}{columns} };
		my @types = @{ $structs{$struct_name}{types} };
		
		my $colnum=1;
		my $rownum = $structs{$struct_name}{rownum};
		
		#######################################################
		# There must be at least as many elements as there
		# are columns, although, due to strings and arrays,
		# this check may not always get rid of corrupt lines
		#######################################################

		if (@ln >= @columns) {

		    foreach my $col (@columns) {
		    
			my $type = shift @types;
			
			###################################################
			# need to do something special with char or array
			# For char, can either be between quotes "" or 
			# curly braces {}, but arrays are always between 
			# braces
			###################################################
    
			if ( $ln[$colnum] =~ /\{/ ) {
			    
			    ###############################################
			    # need to take everything between here and the 
			    # next closing }
			    ###############################################
			    
			    if ( $ln[$colnum] =~ /\}/ ) {
				# No spaces so just use it as-is
				$data{$struct_name}{$rownum}{$col} = 
				    $ln[$colnum];
				# remove the braces
				$data{$struct_name}{$rownum}{$col} =~ 
				    s/\{//g;
				$data{$struct_name}{$rownum}{$col} =~ 
				    s/\}//g;
				
			    } else {
				
				###########################################
				# look for closing brace }
				# Two cases: its a string or its an array
				###########################################
				
				if ($type eq "char") {
				    
				    #####################
				    # Its a string
				    #####################
				    
				    # use the first one
				    my $tcol = $ln[$colnum];
				    ++$colnum;
				    
				    # There must be at least one more; add
				    # it.  If it contains } then we are done,
				    # else add the next one and check it
				    
				    $tcol = "${tcol} $ln[$colnum]";
				    while ($ln[$colnum] !~ /\}/) {
					++$colnum;
					$tcol = "${tcol} $ln[$colnum]";
				    }
				    $data{$struct_name}{$rownum}{$col} = $tcol;
				    
				    # remove the braces
				    $data{$struct_name}{$rownum}{$col} =~ 
					s/\{//g;
				    $data{$struct_name}{$rownum}{$col} =~ 
					s/\}//g;
				} else {
				    
				    #####################
				    # Its an array
				    #####################
				    
				    my @tcol = ();
				    
				    my $tmp = $ln[$colnum];
				    if ($tmp ne "{") {
					$tmp =~ s/\{//g;  # remove braces
					push @tcol, $tmp;
				    }
				    ++$colnum;
				    
				    # There must be at least one more; add
				    # it.  If it contains } then we are done,
				    # else add the next one and check it
				    
				    $tmp = $ln[$colnum];
				    if ($tmp ne "{" && $tmp ne "}") {
					$tmp =~ s/\{//g;    # remove braces
					$tmp =~ s/\}//g;
					push @tcol, $tmp;
				    }
				    while ($ln[$colnum] !~ /\}/) {
					++$colnum;
					if ($tmp ne "{" && $tmp ne "}") {
					    $tmp = $ln[$colnum];
					    $tmp =~ s/\{//g;  # remove braces
					    $tmp =~ s/\}//g;
					    push @tcol, $tmp;
					}
				    }
				    $data{$struct_name}{$rownum}{$col} = 
					[@tcol];
				}
			    }
			    
			    if ($data{$struct_name}{$rownum}{$col} eq "") {
				$data{$struct_name}{$rownum}{$col} = "?";
			    }
			} elsif ( $ln[$colnum] =~ /\"/ ) {
			    
			    # take everything from here to next "
			    my $tcol = $ln[$colnum];
			    my $wq = index($tcol, "\"");
			    $tcol = substr($tcol, $wq+1);
			    
			    # look for next quote in this string
			    $wq = index($tcol, "\"");
			    if ( $wq != -1 ) {
				# No spaces so just use it as-is
				$data{$struct_name}{$rownum}{$col} = 
				    substr($tcol, 0, $wq);
			    } else {
				
				# use the first one
				++$colnum;
				
				# There must be at least one more; add
				# it.  If it contains " then we are done,
				# else add the next one and check it
				
				$tcol = "${tcol} $ln[$colnum]";
				while ($ln[$colnum] !~ /\"/) {
				    ++$colnum;
				    $tcol = "${tcol} $ln[$colnum]";
				}
				$data{$struct_name}{$rownum}{$col} = $tcol;
				
				# remove the closing quote
				$data{$struct_name}{$rownum}{$col} =~ 
				    s/\"//g;
			    }
			    
			} else {
			    $data{$struct_name}{$rownum}{$col} = $ln[$colnum];
			}
			++$colnum;
		    } # Loop over columns 
		} # If at least @columns elements in line

		$structs{$struct_name}{rownum} += 1;
	    } else {
		
		#################################
		# Treat this as a parameter line
		#################################
		
		my $par = shift(@ln);
		
		if (@ln > 1) {

		    #############################################
		    # This is an array or string with space
		    # May be contained between quotes or braces
		    #############################################

		    my $firstcar = substr($ln[0],0,1);
		    if ($firstcar eq "\"") {
			# string between quotes
			$data{$par} = &between($line, "\"", "\"");
		    } elsif ($firstcar eq "\{")  {
			# string between braces
			$data{$par} = &between($line, "\{", "\}");
		    } else {
			# Array
			$data{$par} = @ln;
		    }
		} elsif (@ln == 0) {
		    # Nothing there!
		    $data{$par} = "?";
		} else {
		    # this is a scalar
		    $data{$par} = $ln[0];
		    $data{$par} =~ s/\"//g;
		    $data{$par} =~ s/\$//g;
		}
		
	    } # Parameter line
	} #skipping empty lines
    } # Looping over lines

    # Close the file
    close(IN);

    return %data;

}


1
