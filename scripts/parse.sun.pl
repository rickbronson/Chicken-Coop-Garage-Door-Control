#!/usr/bin/env perl
# parse http://aa.usno.navy.mil/data/docs/RS_OneYear.php

use Date::Manip;
use Time::Piece;
use POSIX qw/strftime/;

if ($ARGV[0] ne "") {
  $infile=$ARGV[0];     # use user-defined file if one is supplied
} else {
  $infile="sun.txt"; # default input file to read (the disk image file)
}

$outfile="/home/rick/boards/stm8/time.h";
open(OUT,"> $outfile") or die "couldn't open $outfile";
($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();
if ($isdst)
	{    # kill leading zero using "-"
		printf OUT "#define SEC_INIT %d\n", strftime "%-S", $sec, $min, $hour - 1, $mday, $mon, $year;
		printf OUT "#define MIN_INIT %d\n", strftime "%-M", $sec, $min, $hour - 1, $mday, $mon, $year;
		printf OUT "#define DOY_INIT %d\n", strftime "%-j", $sec, $min, $hour - 1, $mday, $mon, $year;
		printf OUT "#define HR_INIT %d\n",  strftime "%-H", $sec, $min, $hour - 1, $mday, $mon, $year;
		printf OUT "#define YR_INIT %d\n",  strftime "%-g", $sec, $min, $hour - 1, $mday, $mon, $year;
	}
else
	{
		printf OUT "#define SEC_INIT %d\n", strftime "%-S", $sec, $min, $hour, $mday, $mon, $year;
		printf OUT "#define MIN_INIT %d\n", strftime "%-M", $sec, $min, $hour, $mday, $mon, $year;
		printf OUT "#define DOY_INIT %d\n", strftime "%-j", $sec, $min, $hour, $mday, $mon, $year;
		printf OUT "#define HR_INIT %d\n",  strftime "%-H", $sec, $min, $hour, $mday, $mon, $year;
		printf OUT "#define YR_INIT %d\n",  strftime "%-g", $sec, $min, $hour, $mday, $mon, $year;
	}

open(INF,"< $infile") or die "couldn't open $infile";
binmode(INF);
$cntr = 0;
while(<INF>) {
	chomp;
  push @line, [ split ' ' ];

	if ($line[$cntr][0] =~ m/^01$/ )
		{
			$save_cntr = $cntr;
#			printf("one entry %s\n", $line[$cntr][0]);
		}
	$cntr++;
}
$todaymonth = localtime->strftime('%-m');  # kill leading zero
$todayday = localtime->strftime('%-d');
printf("$todaymonth $todayday");
$lut = "static const char sunrise_lut[] = {\n";
$oldvalue = 0;
# do sunrise
$veryfirst = 1;
for($month = 1; $month <= 12; $month++)
	{
		$day = 1;
		foreach $row ($save_cntr..$save_cntr+31)
			{
				$value = $line[$row][($month - 1) * 2 + 1];
				if ($value && $value ne "NA")
					{
						$value =~ m/(\d\d)(\d\d)/;
						$hour = $1;
						$min = $2;
						$value =~ s/(\d\d)(\d\d)/$1:$2/;
						$diff = &DateCalc(&ParseDate("$oldvalue"), &ParseDate("$value"), \$err, 1);

						$oldvalue = $value;
#						printf ("$value, %s ", Delta_Format($diff, "%mv"));
						$lut .= sprintf ("%s, ", Delta_Format($diff, "%mv")) if !$veryfirst;
						$veryfirst = 0;
						if ($month == $todaymonth && $day == $todayday)
							{
								printf OUT "#define SUNRISE_MINS_INIT %d // %s\n", $hour * 60 + $min, $value;
							}
						if ($month == 1 && $day == 1)
							{
								printf OUT "#define SUNRISE_JAN1_MINS_INIT %d // %s\n", $hour * 60 + $min, $value;
							}
					}
				else
					{
						$lut .= sprintf " /* %d */\n", $month;
						last;
					}
				$day++;
			}
	}
$lut .= sprintf ("};\n\n");

# do sunset
$lut .= "static const char sunset_lut[] = {\n";
$veryfirst = 1;
for($month = 1; $month <= 12; $month++)
	{
		$day = 1;
		foreach $row ($save_cntr..$save_cntr+31)
			{
				$value = $line[$row][($month - 1) * 2 + 2];
				if ($value && $value ne "NA")
					{
						$value =~ m/(\d\d)(\d\d)/;
						$hour = $1;
						$min = $2;
						$value =~ s/(\d\d)(\d\d)/$1:$2/;
						$diff = &DateCalc(&ParseDate("$oldvalue"), &ParseDate("$value"), \$err, 1);

						$oldvalue = $value;
#						printf ("$value, %s ", Delta_Format($diff, "%mv"));
						$lut .= sprintf ("%s, ", Delta_Format($diff, "%mv")) if !$veryfirst;
						$veryfirst = 0;
						if ($month == $todaymonth && $day == $todayday)
							{
								printf OUT "#define SUNSET_MINS_INIT %d // %s\n", $hour * 60 + $min, $value;
							}
						if ($month == 1 && $day == 1)
							{
								printf OUT "#define SUNSET_JAN1_MINS_INIT %d // %s\n", $hour * 60 + $min, $value;
							}
					}
				else
					{
						$lut .= sprintf " /* %d */\n", $month;
						last;
					}
				$day++;
			}
	}
$lut .= sprintf ("};");
printf OUT ($lut);

close OUT;
close INF;
