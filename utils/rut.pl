#!/usr/bin/perl -w

# Results UTility: Utility to examine Pan result files
# Rich Holmes 3/03

use FindBin;
use lib $FindBin::Bin;
use TaFileName;

use Getopt::Long;

# Get command line options

$anadef = "standard";
$comdef = "";

&dieusage() if @ARGV == 0;

GetOptions (
	    "a|anatype:s" => \$anatype,
	    "c|comment:s" => \$com,
	    "e|expr=s" => \$re,
	    "h|help" => \$givehelp,
	    "r|run=i" => \$run,
	    );

&dieusage() if $givehelp;

# Handle arguments required if no input files specified

if (@ARGV == 0)
{
    $anatype = $anadef if !defined $anatype || $anatype eq "";
    $com = $comdef if !defined $com;
    $re = "" if !defined $re;
    
    if (!defined $run || $run eq "")
    {
	die "Must specify --run=<run number>\n";
    }

    @fn = (MakeTaFileName2 ($run, $anatype, "result", $com, ""));
}
else
{
    @fn = @ARGV;
}

foreach $fn (@fn)
{
    open (FILE, $fn) or die "Can't open file $fn";
    warn "Opened $fn for input\n";
    
    $foundheader = 0;
    while (<FILE>)
    {
	next if /^\s*$/ || /^\#/;  # skip blanks and comments
	if (!$foundheader)
	{
	    $foundheader = 1;
	    next; # skip header
	}
	
	if (/$re/)
	{
	    ($stage, $qty, $stat, $rest) = /(.*?) ([^ ]*)_(.*?) (.*)/;
	    
	    ($val, $err, $min, $max, $units) = split (" ", $rest, 5);
	    
	    $val = sprintf "%f", $val;
	    $err = sprintf "%f", $err;
	    $key = "${stage}~${qty}~${min}~$max";
	    
	    if ($stat eq "mean")
	    {
		$mean{$key} = (sprintf "%f", $val) . (sprintf "%f", $err);
		$units{$key} = "$units";
	    }
	    elsif ($stat eq "RMS")
	    {
		$mean{$key} = sprintf "%f", $val;
		$rms{$key} = $val;
	    }
	    elsif ($stat eq "Neff")
	    {
		$neff{$key} = sprintf "%.1f", $val;
	    }
	}
    }
    
    foreach $k (sort keys %mean)
    {
	($stage, $qty, $min, $max) = split ("~", $k);
	print "From ${stage}: $qty in evts $min to $max: ";
	print "  mean $mean{$k}";
	print " RMS $rms{$k}" if defined $rms{$k};
	print " $units{$k}";
	print " with N_eff = $neff{$k}" if defined $neff{$k};
	print "\n";
    }
}

sub dieusage
{
    die << "USE"
usage: rut [options] [files]
   Options to locate result files:
	 --a=s or --anatype=s   analysis type (defaults to "$anadef")
	 --c=s or --comment=s   comment (defaults to "$comdef")
	 --r=i or --run=i       run number (required if no files specified)

   Other options:
	 --e=s or --expr=s      regular expression to look for
	 --h   or --help        print this message
USE
}
