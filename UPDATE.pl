#!/usr/bin/perl -w
use strict;
use Time::Local;

my %config;
my %input;
my %last_update;

my $data_dir = '/srv/klimalogger/data/';


# retrieve last update timestamps of existing DBs
while ( my $file = <$data_dir/sensor_*.rrd> ) {
	open(FH, "rrdtool info $file | grep last_update |");
	my $output = <FH>;
	close FH;

	my ( $db )   = ( $file =~ /sensor_([a-z\d]+)\.rrd/i );
	my ( $time ) = ( $output =~ /last_update = (\d+)/ );
	$last_update{$db} = $time;
}


# store decoded data in %input
open (FH, "/srv/klimalogger/bin/decode_tfa $ARGV[0] 2>/dev/null |");
while ( <FH> ) {

	if ( /^\d+ (\d+).(\d+).(\d+) (\d+):(\d+) (.*)/ ) { 
		my $timestamp;
		my ( $sec, $min, $hour, $mday, $mon, $year ) =
			( 0, $5, $4, $1, $2, $3 );
		my $data = $6;

		$timestamp =  timelocal( $sec, $min, $hour, $mday,
				      $mon - 1, $year);

		$input{$timestamp} = $data;
	}

}
close FH;


# rrdtool likes its timestamps ascending
foreach my $key ( sort(keys %input) ) {
	my ( @tmp_temp, @tmp_hum, @tmp_pairs );

	@tmp_pairs = ( $input{$key} =~ m/[TH][in\d]+: [\d\.]+/g );

	foreach ( @tmp_pairs ) {
		my ( $dtype, $sens, $value ) =
			( /([TH])([in\d]+): ([\d\.]+)/ );

		if ( $dtype eq 'T' ) {
			$tmp_temp[ $sens eq 'in' ? 0 : $sens ] = $value;
		} else {
			$tmp_hum [ $sens eq 'in' ? 0 : $sens ] = $value;
		}
	}

	for ( my $i=0; $i < @tmp_temp or $i < @tmp_hum; $i++ ) {
		my $sens = ( $i == 0 ? 'in' : $i );
		my $temp = defined $tmp_temp[$i] ? $tmp_temp[$i] : 'U';
		my $hum  = defined $tmp_hum[$i]  ? $tmp_hum[$i]  : 'U';

		# no need to add UNKNOWN values
		next if $temp eq $hum and $temp eq 'U';

		# no need to add old values
		next if defined $last_update{$sens} and
		        $key <= $last_update{$sens};
	

		if ( ! -f "$data_dir/sensor_$sens.rrd" ) {
			create_db($sens, $key);
		}

		system( "rrdtool update $data_dir/sensor_$sens.rrd $key:$temp:$hum" )
	}
}




sub create_db {
	my ( $dbname, $start ) = ( @_ );
	$start--;

	print "Creating DB $dbname...\n";

	my $cmd = "rrdtool create $data_dir/sensor_$dbname.rrd \\
	             --start $start \\
	             --step 60 \\
	             DS:temp:GAUGE:3600:-273:U \\
	             DS:humidity:GAUGE:3600:0:100 \\
	             RRA:AVERAGE:0.6:1:1440 \\
	             RRA:AVERAGE:0.5:60:730 \\
	             RRA:AVERAGE:0.5:1440:365";

	system( $cmd );# or die "Creation of DB $dbname failed: $?";
}
