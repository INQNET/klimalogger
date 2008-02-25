#!/usr/bin/perl -w
use strict;
my %config;

#### CONFIG SECTION ####

$config{'in'} = "Serverraum";
$config{'1'}  = "Raum ITS";
$config{'2'}  = "Raum Operative Leitung";
$config{'3'}  = "Raum FL";

$config{'datadir'}   = "/srv/klimalogger/data/";
$config{'outputdir'} = "/srv/klimalogger/web/out/";

#### END CONFIG     ####

my $sensor = 'in';

while ( my $db = <$config{'datadir'}/sensor_$sensor.rrd> ) {
	$db =~ s/.*sensor_([\da-z]+)\.rrd.*/$1/g;
	graph($db, '48h');
}

sub graph {
	my ($db, $time ) = ( @_ );
	my $start;
	my $output_temp = $config{'outputdir'} . "/graph_${db}_${time}_Temperature.png";

	if ( $time eq '48h' ) {
		$start = 'end-48h';
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
		--end now \\
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

}

