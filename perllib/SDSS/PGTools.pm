package SDSS::PGTools;
require Exporter;

use strict;
use DBI;

our @ISA = qw(Exporter);
our @EXPORT = qw(write_runlist);

our $VERSION = 0.1;

# we input an ordinary hash of hashes, not references are 
# are returned by postgres.
sub write_runlist {

    my $RI = shift;
    
    my $dbname="sdss";
    my $user = "sdss";
    my $table = "runlist";
    my $dbh = DBI->connect("dbi:Pg:dbname=$dbname", "$user", "");

    # get column names
#    my $sth = $dbh->column_info("","",$table,"");
#    my @column_names;
#    while (my $href=$sth->fetchrow_hashref()) {
#	push @column_names,$href->{COLUMN_NAME};
#    }


    # read all current data from the table
    my $st = "select * from $table";
    my $RIold = $dbh->selectall_hashref($st,"column_id");
    
    # prepare an INSERT and an UPDATE statement
    my $insert_statement = 
	"INSERT INTO $table " .
	"(column_id, run, rerun, camcol, stripe, strip," .
	"n_tsObj, n_tsField, n_fpAtlas, n_fpM, n_psField, n_adatc," .
	"tsObj_photo_v, fpAtlas_photo_v, adatc_photo_v, baye_ver, phtz_ver," .
	"imagingDir, adatcDir, host) VALUES " . 
	"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    # currently we update all, but the question is which is faster, the database
    # operation or checking each value in perl?  This is less coding.
    my $update_statement = 
	"UPDATE $table SET" .
	" run=?, rerun=?, camcol=?, stripe=?, strip=?," .
	" n_tsObj=?, n_tsField=?, n_fpAtlas=?, n_fpM=?, n_psField=?, n_adatc=?," .
	" tsObj_photo_v=?, fpAtlas_photo_v=?, adatc_photo_v=?, baye_ver=?, phtz_ver=?," .
	" imagingDir=?, adatcDir=?, host=? WHERE column_id=? ";
    
    my $insert = $dbh->prepare($insert_statement);
    my $update = $dbh->prepare($update_statement);

    # loop over the keys of *outer* hash: these are the rows
    # the key should be same as column_id primary key 
    foreach my $key (keys %$RI) {

	# simplify the notation by making a copy
	my %TRI = %{ $$RI{$key} };

	# does this primary key already exist?  If not, to an INSERT
	if (! exists($RIold->{$key}) ) {

	    my $numrows = 
		$insert->execute($key, $TRI{run}, $TRI{rerun}, $TRI{camcol}, $TRI{stripe}, $TRI{strip}, 
				 $TRI{n_tsObj}, $TRI{n_tsField}, $TRI{n_fpAtlas}, $TRI{n_fpM}, $TRI{n_psField}, $TRI{n_adatc}, 
				 $TRI{tsObj_photo_v}, $TRI{fpAtlas_photo_v}, $TRI{adatc_photo_v}, 
				 $TRI{baye_ver}, $TRI{phtz_ver},
				 $TRI{imagingDir}, $TRI{adatcDir}, $TRI{host});
	    # this should be unneccesary
	    if (!$numrows) {
		die "Insert failed: ".$dbh->errstr."\n";
	    } 

	    
	} else { 

	    my $numrows = 
		$update->execute($TRI{run}, $TRI{rerun}, $TRI{camcol}, $TRI{stripe}, $TRI{strip}, 
				 $TRI{n_tsObj}, $TRI{n_tsField}, $TRI{n_fpAtlas}, $TRI{n_fpM}, $TRI{n_psField}, $TRI{n_adatc}, 
				 $TRI{tsObj_photo_v}, $TRI{fpAtlas_photo_v}, $TRI{adatc_photo_v}, 
				 $TRI{baye_ver}, $TRI{phtz_ver},
				 $TRI{imagingDir}, $TRI{adatcDir}, $TRI{host}, $key);

	    # should not be neccesary
	    if (!$numrows) {
		die "Update failed: ".$dbh->errstr."\n";
	    }

	}

    } # loop over primary keys

    $insert->finish();
    $update->finish();
    $dbh->disconnect();

}


1
