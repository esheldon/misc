
package    SDSS::DBMTools;
require    Exporter;

use strict;
use SDSS::StringOp;

our @ISA = qw(Exporter);
our @EXPORT = qw(write_rundb read_rundb read_rundb_byrun read_imagingRoot
		 read_steves_runlist);
our $VERSION = 1.0;

# These are always by camcol, and we just read them differently
# run/rerun/stripe/strip/camcol/ntsobj/natlas/nfpm/npsfield/cdir/odir/host

my $rundb_format = "s s c s A s s s s s s A10 A10 A10 A10 A10 A80 A80 A40";

#########################################################
# write a run database
#########################################################

sub write_rundb {

    my $RI = shift;
    
    my $dbm_dir;
    if ( exists($ENV{"DBM_DIR"}) ) {
	$dbm_dir = $ENV{"DBM_DIR"};
    } else {
	die "Environment variable DBM_DIR is not set";
    }

    my $dbm_name = "${dbm_dir}/rundb/runlist";

    my %RUNDB;
    dbmopen(%RUNDB, $dbm_name, 0644)
	or die "Can't create/open $dbm_name: $!";


    # loop over the keys of *outer* hash
    foreach my $key (keys %$RI) {

	# simplify the notation by making a copy
	my %TRI = %{ $$RI{$key} };

	$RUNDB{$key} = pack($rundb_format, 
			    $TRI{run},
			    $TRI{rerun},
			    $TRI{camcol},
			    $TRI{stripe},
			    $TRI{strip},
			    
			    $TRI{n_tsObj},
			    $TRI{n_tsField},
			    $TRI{n_fpAtlas},
			    $TRI{n_fpM},
			    $TRI{n_psField},
			    $TRI{n_adatc},
			    
			    $TRI{tsObj_photo_v},
			    $TRI{fpAtlas_photo_v},
			    $TRI{adatc_photo_v},
			    $TRI{baye_ver},
			    $TRI{phtz_ver},
			    
			    $TRI{imagingDir},
			    $TRI{adatcDir},
			    
			    $TRI{host});

    }

    dbmclose(%RUNDB);
}



#########################################################
# read a run database
#########################################################


sub read_rundb {
    
    my $nargs=@_;

    my $init=0;
    if ($nargs > 0) {
	my $config = shift;
	if ( exists($$config{init}) ) {
	    $init = $$config{init};
	}
    }

    my $dbm_dir;
    if ( exists($ENV{"DBM_DIR"}) ) {
	$dbm_dir = $ENV{"DBM_DIR"};
    } else {
	die "Environment variable DBM_DIR is not set";
    }
    my $dbm_name = "${dbm_dir}/rundb/runlist";
    
    # Should we re-create the file?
    if ($init) {
	printf(STDERR "Initializing file $dbm_name\n");
	my $tmp = `rm ${dbm_name}.*`;
	printf(STDERR "$tmp");
    }

    my %RUNDB;
    dbmopen(%RUNDB, $dbm_name, 0644)
	or die "Can't create/open $dbm_name: $!";

    my %RI;

    foreach my $key (keys %RUNDB) {
	(
	 $RI{$key}{run},
	 $RI{$key}{rerun},
	 $RI{$key}{camcol},
	 $RI{$key}{stripe},
	 $RI{$key}{strip},

	 $RI{$key}{n_tsObj},
	 $RI{$key}{n_tsField},
	 $RI{$key}{n_fpAtlas},
	 $RI{$key}{n_fpM},
	 $RI{$key}{n_psField},
	 $RI{$key}{n_adatc},

	 $RI{$key}{tsObj_photo_v},
	 $RI{$key}{fpAtlas_photo_v},
	 $RI{$key}{adatc_photo_v},
	 $RI{$key}{baye_ver},
	 $RI{$key}{phtz_ver},

	 $RI{$key}{imagingDir},
	 $RI{$key}{adatcDir},

	 $RI{$key}{host}
	 ) = unpack($rundb_format, $RUNDB{$key});
    }

    dbmclose(%RUNDB);
    return %RI;
}

sub read_rundb_byrun {
    
    my $dbm_dir;
    if ( exists($ENV{"DBM_DIR"}) ) {
	$dbm_dir = $ENV{"DBM_DIR"};
    } else {
	die "Environment variable DBM_DIR is not set";
    }
    my $dbm_name = "${dbm_dir}/rundb/runlist";

    my %RUNDB;
    dbmopen(%RUNDB, $dbm_name, 0644)
	or die "Can't create/open $dbm_name: $!";

    my %RI;

    # temporary variablesx
    my $trun;
    my $trerun;
    my $tcamcol;
    my $tstripe;
    my $tstrip;

    my $tn_tsObj;
    my $tn_tsField;
    my $tn_fpAtlas;
    my $tn_fpM;
    my $tn_psField;
    my $tn_adatc;

    my $ttsObj_photo_v;
    my $tfpAtlas_photo_v;
    my $tadatc_photo_v;
    my $tbaye_ver;
    my $tphtz_ver;

    my $timagingDir;
    my $tadatcDir;

    my $thost;

    # read from the database
    foreach my $key (keys %RUNDB) {

	(
	 $trun, 
	 $trerun,
	 $tcamcol,
	 $tstripe,
	 $tstrip,

	 $tn_tsObj,
	 $tn_tsField,
	 $tn_fpAtlas,
	 $tn_fpM,
	 $tn_psField,
	 $tn_adatc,

	 $ttsObj_photo_v,
	 $tfpAtlas_photo_v,
	 $tadatc_photo_v,
	 $tbaye_ver,
	 $tphtz_ver,

	 $timagingDir,
	 $tadatcDir,

	 $thost
	 ) = unpack($rundb_format, $RUNDB{$key});

	# Now create a run-rerun key rather than 
	# run-rerun-camcol key

	my $runstring = &run2string($trun);
	my $newkey = "$runstring-$trerun";

	$RI{$newkey}{run} = $trun;
	$RI{$newkey}{rerun} = $trerun;
	$RI{$newkey}{stripe} = $tstripe;
	$RI{$newkey}{strip} = $tstrip;
	
	if (!exists($RI{$newkey}{n_tsObj})) {
	    $RI{$newkey}{n_tsObj} = $tn_tsObj;
	    $RI{$newkey}{n_tsField} = $tn_tsField;
	    $RI{$newkey}{n_fpAtlas} = $tn_fpAtlas;
	    $RI{$newkey}{n_fpM} = $tn_fpM;
	    $RI{$newkey}{n_psField} = $tn_psField;
	    $RI{$newkey}{n_adatc} = $tn_adatc;
	} else {
	    
	    # Copy in the *least* in order to indicate
	    # problems
	    if ( $RI{$newkey}{n_tsObj} > $tn_tsObj ) {
		$RI{$newkey}{n_tsObj} = $tn_tsObj;
	    }

	    if ( $RI{$newkey}{n_tsField} > $tn_tsField ) {
		$RI{$newkey}{n_tsField} = $tn_tsField;
	    }

	    if ( $RI{$newkey}{n_fpAtlas} > $tn_fpAtlas ) {
		$RI{$newkey}{n_fpAtlas} = $tn_fpAtlas;
	    }

	    if ( $RI{$newkey}{n_fpM} > $tn_fpM ) {
		$RI{$newkey}{n_fpM} = $tn_fpM;
	    }
	    
	    if ( $RI{$newkey}{n_psField} > $tn_psField ) {
		$RI{$newkey}{n_psField} = $tn_psField;
	    }

	    if ( $RI{$newkey}{n_adatc} > $tn_adatc ) {
		$RI{$newkey}{n_adatc} = $tn_adatc;
	    }

	}

	$RI{$newkey}{tot_tsObj} += $tn_tsObj;
	$RI{$newkey}{tot_tsField} += $tn_tsField;
	$RI{$newkey}{tot_fpAtlas} += $tn_fpAtlas;
	$RI{$newkey}{tot_fpM} += $tn_fpM;
	$RI{$newkey}{tot_psField} += $tn_psField;
	$RI{$newkey}{tot_adatc} += $tn_adatc;

	$RI{$newkey}{tsObj_photo_v} = $ttsObj_photo_v;
	$RI{$newkey}{fpAtlas_photo_v} = $tfpAtlas_photo_v;
	$RI{$newkey}{adatc_photo_v} = $tadatc_photo_v;
	$RI{$newkey}{baye_ver} = $tbaye_ver;
	$RI{$newkey}{phtz_ver} = $tphtz_ver;

	$RI{$newkey}{imagingDir} = $timagingDir;
	$RI{$newkey}{adatcDir} = $tadatcDir;

	$RI{$newkey}{host} = $thost;

    }

    dbmclose(%RUNDB);
    return %RI;

}


sub read_imagingRoot {

    ############################
    # Read Steve's runlist
    ############################

    my %srl = &read_steves_runlist;

    ############################
    # imagingRoot
    ############################

    my $dbm_dir;
    if ( exists($ENV{"DBM_DIR"}) ) {
	$dbm_dir = $ENV{"DBM_DIR"};
    } else {
	die "Environment variable DBM_DIR is not set";
    }
    my $fermi_imagingRoot_name = "${dbm_dir}/imagingRoot/imagingRoot.dat";

#    print "FERMI IMAGING ROOT: $fermi_imagingRoot_name\n";

    open(IMAGINGROOT, $fermi_imagingRoot_name) || 
	die "can't open file $fermi_imagingRoot_name: $!\n";


    # skip first two lines

    my %FRI;

    my $li=0;

    foreach my $line (<IMAGINGROOT>) {

	if ($li >= 2) {

	    chomp($line);
	    my @ln = split " ", $line;

	    my $trun = $ln[0];
	    my $trerun = $ln[1];

	    my $runstring = &run2string($trun);
	    my $key = "${runstring}-${trerun}";

	    $FRI{$key}{run} = $trun;
	    $FRI{$key}{rerun} = $trerun;
	    $FRI{$key}{n_fpC} = $ln[2];
	    $FRI{$key}{n_fpBIN} = $ln[3];
	    $FRI{$key}{n_fpM} = $ln[4]*5;
	    $FRI{$key}{n_fpObjc} = $ln[5];
	    $FRI{$key}{n_fpFieldStat} = $ln[6];
	    $FRI{$key}{n_fpAtlas} = $ln[7];
	    $FRI{$key}{n_psField} = $ln[8];
	    $FRI{$key}{n_tsField} = $ln[9];
	    $FRI{$key}{n_tsObj} = $ln[10];

	    # get stripe/strip info from steve's runlist
	    if (! exists($srl{$runstring}{stripe}) ) {
		$FRI{$key}{stripe} = -1;
		$FRI{$key}{strip} = "?";
	    } else {
		$FRI{$key}{stripe} = $srl{$runstring}{stripe};
		$FRI{$key}{strip} = $srl{$runstring}{strip};
	    }

	}

	$li = $li + 1;
    }

    close(IMAGINGROOT);

    return %FRI;

}

sub read_steves_runlist {

    my $dbm_dir;
    if ( exists($ENV{"DBM_DIR"}) ) {
	$dbm_dir = $ENV{"DBM_DIR"};
    } else {
	die "Environment variable DBM_DIR is not set";
    }
    my $steves_runlist = "${dbm_dir}/steves_runlist/run.par";

#    print "STEVES RUNLIST: $steves_runlist\n";

    open(RUNLIST, $steves_runlist) || 
	die "can't open file $steves_runlist: $!\n";

    my %srl;
    foreach my $line (<RUNLIST>) {
	
	chomp($line);
	my @ln = split " ", $line;
	
	if (@ln == 12) {
	    
	    my $trun = $ln[1];
	    my $runstring = &run2string($trun);
	    my $skey = "$runstring";

	    $srl{$skey}{run} = $trun;
	    $srl{$skey}{mjd} = $ln[2];
	    $srl{$skey}{stripe} = $ln[3];
	    $srl{$skey}{strip} = $ln[4];
	    
	}
	
    }

    close(RUNLIST);
    
    return %srl;

}

1
