#!/usr/bin/perl -w

use strict;
<<<<<<< test.pl
=======
use Ascii::Read;
use Hash::Print;
use Sort::Naturally;
use Getopt::Long;

use DBI;
use SDSS::PGTools;
use Data::Dumper;

my %RI;
my $key = "000756-44-2";
$RI{$key}{run} = 756;
$RI{$key}{rerun} = 44;
$RI{$key}{camcol} = 2;
$RI{$key}{stripe} = 10;
$RI{$key}{strip} = 'N';

$RI{$key}{n_tsObj} = 555;
$RI{$key}{n_tsField} = 500;
$RI{$key}{n_fpAtlas} = 500;
$RI{$key}{n_fpM} = 500;
$RI{$key}{n_psField} = 500;
$RI{$key}{n_adatc} = 500;

$RI{$key}{tsObj_photo_v} = 'v5_4_4';
$RI{$key}{fpAtlas_photo_v} = 'v5_4_4';
$RI{$key}{adatc_photo_v} = 'v5_4_4';

$RI{$key}{baye_ver} = 'v1_3';
$RI{$key}{phtz_ver} = 'v2_6';

$RI{$key}{imagingDir} = '/blah/blah';
$RI{$key}{adatcDir} = '/blah/stuff';

$RI{$key}{host} = 'cheops1';

&write_runlist(\%RI);


exit;

my $dbh = DBI->connect("dbi:Pg:dbname=sdss", "sdss", "");

if (!$dbh) {
    die "Error: Couldn't open connection: ".$DBI::errstr."\n";
}

my $sth = $dbh->column_info("","","test","");
my @column_names;
while (my $href=$sth->fetchrow_hashref) {
    push @column_names,$href->{COLUMN_NAME};
}

# first get all the known entries to test against
my $select_statement = "SELECT * FROM test";
my $select_hash = $dbh->selectall_hashref($select_statement, "id");

# insert some values
my $insert_st = "INSERT INTO test VALUES (?, ?)";
my $update_st = "UPDATE test SET name=? WHERE id=?";

my $insert_query = $dbh->prepare($insert_st);
my $update_query = $dbh->prepare($update_st);
for (my $i=0; $i<26; $i++) {

    my $id2 = $i+10;
    my $name = "hello-$i";

    # only insert if doesn't already exist, otherwise 
    # check if we should update
    if (! exists($select_hash->{$i} ) ) {
	my $res = $insert_query->execute($i, $name);
	
	# this should be unneccesary
	if (!$res) {
	    die "Insert failed: ".$dbh->errstr."\n";
	} 
    
    } else {
	# should we update?
	if ($select_hash->{$i}->{name} ne $name) {
	    my $res = $update_query->execute($name, $i);
	    if (!$res) {
		die "Update failed: ".$dbh->errstr."\n";
	    }
	}
    }
}

my $st = "SELECT * FROM test";

&print_header(\@column_names);
my $array_ref = $dbh->selectall_arrayref($st);
foreach my $aref (@{$array_ref}) {
    print join(" ",@$aref),"\n";
}
exit;



exit;
my @columns = qw(photoid run rerun camcol field id m_r[2]);

my $statement = "SELECT " . join(", ",@columns) . " FROM adatc LIMIT 10";


# Get it all at once
&print_header(\@columns);
my $ary_ref = $dbh->selectall_arrayref($statement);
foreach my $aref (@{$ary_ref}) {
    print join(" ",@$aref),"\n";
}
exit;

print "\n";
# get it one row at a time
&print_header(\@columns);
my $query = $dbh->prepare($statement);
$query->execute();
while (my @row = $query->fetchrow_array()) {
    print join(" ", @row),"\n";
}

# use dump results
print "\n";
$query = $dbh->prepare($statement);
$query->execute();
&print_header(\@columns);
DBI::dump_results($query);


# Get it all at once as a hash ref to hash refs
my @keyfields = ("run","rerun");
my $hash_ref = $dbh->selectall_hashref($statement, "photoid");

print "\n";
&print_header(\@columns);
foreach my $key (nsort keys %$hash_ref) {
    print "key = $key photoid = $hash_ref->{$key}->{photoid} run = $hash_ref->{$key}->{run}\n";
}


$dbh->disconnect();

sub print_header {

    my $columns = shift;

    my $header = join(" ",@$columns);
    my $hlen = length($header);
    print $header,"\n";
    for (my $i=0;$i<$hlen+1;$i++){ print "-"; }
    print "\n";
>>>>>>> 1.5

<<<<<<< test.pl
use strict;

my $s = "things";

if ($s =~ /(hello|goodbye|stuff)/) {
    print "yes\n";
=======
>>>>>>> 1.5
}

