#!/usr/bin/perl -w

# HAPPEX prompt analysis script

# usage:
#  prompt.pl [-f] [run]
#  prompt.pl [-f] -b [list of runs]

# Option -f forces Pan analysis of the CODA file even if a ROOT file
# already exists; otherwise Pan analysis is skipped.  Then we run 
# prompt.macro from within Pan.

# Option -b means batch mode: no graphics windows are opened,
# prompt.macro is called with nonzero second argument, and Pan exits
# after prompt.macro completes.  Otherwise prompt is called with zero
# second argument and Pan stays active so user can inspect plots.

# If no runs are specified in interactive mode the user is prompted for one.

use FindBin; 
use lib $FindBin::Bin;
use TaFileName;  # put the directory containing this (./utils) in PERL5LIB environment variable
                 # if you want to run this from a different directory

$force = "";
$batch = 0;
$opts = "";
while ($#ARGV >= 0 && $ARGV[0] =~ /^-/)
{
    if ($ARGV[0] eq '-f')
    {
	shift;
	$force = 1;
    }
    elsif ($ARGV[0] eq '-b')
    {
	shift;
	$batch = 1;
	$opts = "-b -q";
    }
    else
    {
	die &usage();
    }
}

if (!$batch && $#ARGV < 0)
{
    print "Run: ";
    $line = <STDIN>;
    @runs = $line =~ /([0-9]+)/;
}
else
{
    @runs = @ARGV;
}

die "No runs to analyze" if $#runs < 0;
die "Multiple runs without -b option" if !$batch && $#runs > 0;

print "Analyzing run" . ($#runs > 0 ? "s" : "") . " " . (join ", ", @runs) . "\n";
foreach $run (@runs)
{
    if ($run =~ m/[^0-9]/)
    {
	warn "\n'$run' is not a valid run number, skipping\n";
	next;
    }
    warn "\n============ Attempting Prompt analysis for run $run\n";
    $codafn = (MakeTaFileName2 ($run, "", "coda", "", ""));
    $rootfn = (MakeTaFileName2 ($run, "standard", "root", "", ""));
    if ($force || !-e $rootfn)
    { 
	if (!-e $codafn)
	{
	    warn "CODA file $codafn not found, skipping\n";
	    next;
	}
	system "echo './pan -r $run'";
	system "./pan -r $run";
    }
    else
    {
	warn "ROOT file $rootfn already exists\n";
    }
    if (!-e $rootfn)
    {
	warn "ROOT file $rootfn not found, skipping\n";
	next;
    }
    system "echo \"./pan $opts 'prompt.macro($run, $batch)'\"";
    system "./pan $opts 'prompt.macro($run, $batch)'";
}

sub usage
{
    return "Usage: prompt.pl [-f] -b [list of runs]\n       prompt.pl [-f] [run]\n";
}
