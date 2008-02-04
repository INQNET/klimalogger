#!/usr/bin/perl -w
use strict;
my %config;

#### CONFIG SECTION ####

$config{'in'} = "Serverraum";
$config{'1'}  = "Raum ITS";
$config{'2'}  = "Raum X";
$config{'3'}  = "Raum FL";

$config{'datadir'}   = "/srv/klimalogger/data/";
$config{'outputdir'} = "/srv/klimalogger/web/out/";

#### END CONFIG     ####



while ( my $db = <$config{'datadir'}/sensor_*.rrd> ) {
	$db =~ s/.*sensor_([\da-z]+)\.rrd.*/$1/g;
	graph($db, 'Day');
	graph($db, 'Week');
	graph($db, 'Month');
	graph($db, 'Year');
}




sub graph {
	my ($db, $time ) = ( @_ );
	my $start;
	my $output_temp = $config{'outputdir'} . "/graph_${db}_${time}_Temperature.png";
	my $output_hum  = $config{'outputdir'} . "/graph_${db}_${time}_Humidity.png";


	if ( $time eq 'Day' ) {
		$start = 'end-1d';
	} elsif ( $time eq 'Week' ) {
		$start = 'end-1w';
	} elsif ( $time eq 'Month' ) {
		$start = 'end-1m';
	} elsif ( $time eq 'Year' ) {
		$start = 'end-1y';
	}

	my $dbalias;
	if ( defined $config{$db} ) {
		$dbalias = $config{$db};
	} else {
		$dbalias = $db;
	}

	my $cmd = "rrdtool graph $output_temp \\
	        -w 650 \\
	        -h 160 \\
	        --start $start \\
		--vertical-label=\"Degree Celsius\" \\
	        DEF:temp=$config{'datadir'}/sensor_${db}.rrd:temp:AVERAGE \\
	        LINE2:temp#FF0000:\"Temperature\" \\
	        GPRINT:temp:MIN:\"  Min\\: %4.1lf\" \\
	        GPRINT:temp:MAX:\" Max\\: %4.1lf\" \\
	        GPRINT:temp:AVERAGE:\" Avg\\: %4.1lf\" \\
	        -t \" $time - Sensor $dbalias (Temperature) \" ";

	#print $cmd;
	system( $cmd . " > /dev/null");


	$cmd = "rrdtool graph $output_hum \\
	        -w 650 \\
	        -h 160 \\
	        --start $start \\
		--vertical-label=\"%\" \\
	        DEF:hum=$config{'datadir'}/sensor_${db}.rrd:humidity:AVERAGE \\
	        LINE2:hum#00FF00:\"Humidity\" \\
	        GPRINT:hum:MIN:\"  Min\\: %4.1lf\" \\
	        GPRINT:hum:MAX:\" Max\\: %4.1lf\" \\
	        GPRINT:hum:AVERAGE:\" Avg\\: %4.1lf\" \\
	        -t \" $time - Sensor $dbalias (Humidity) \" ";

	#print $cmd;
	system( $cmd . " > /dev/null");

}

