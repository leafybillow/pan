#!/usr/bin/perl -w

# HAPPEX prompt analysis script

# usage:
#  prompt.pl [option] [run]
#  prompt.pl [option] --batch [list of runs]

# Options are
#
#   --forcepan
#   --forcereg
#   --forcedit
#   --nopan
#   --noreg
#   --nodit
#   --nomacro

# Normally Pan analysis of the CODA file is skipped if a Pan ROOT file
# already exists (in the $PAN_ROOT_FILE_PATH), regression and
# dithering analyses are skipped if a Redana ROOT file already exists,
# and prompt.macro is run (from inside ROOT).  The above options
# force Pan analysis, regression analysis, or dithering analysis to be
# done in any case (--forcexxxx) or force these or prompt.macro
# execution to be skipped (--noxxxx).

# Option --batch means batch mode: no graphics windows are opened,
# prompt.macro is called with nonzero second argument, and Pan exits
# after prompt.macro completes.  Otherwise prompt is called with zero
# second argument and Pan stays active so user can inspect plots.

# If no runs are specified in interactive mode the user is prompted for one.

use FindBin; 
use lib $FindBin::Bin;
use TaFileName;  # put the directory containing this (./utils) in PERL5LIB environment variable
                 # if you want to run this from a different directory

$DEBUG = 0;

$forcepan = "";
$forcereg = "";
$forcedit = "";
$nopan = "";
$noreg = "";
$nodit = "";
$nomacro = "";
$batch = 0;
$opts = "-l";

# The following greatly evil code just gets defined options off the
# command line and sets the corresponding variables to 1.

%opts = qw / forcepan x forcereg x forcedit x nopan x noreg x nodit x nomacro x batch x /;
eval "\$$1=1" while $#ARGV > -1 && $ARGV[0] =~ /^--(\w+)/ && $opts{$1} && shift; 
die &usage() if $#ARGV > -1 && $ARGV[0] =~ /^-/;


if ($batch)
{
    $opts = "-b -q";
}
else
{
    $batch = 0;
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
die "Multiple runs without --batch option" if !$batch && $#runs > 0;

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
    $rrootfn = (MakeTaFileName2 ($run, "red", "root", "", ""));

# Pan analysis =======================

    if (!$nopan)
    {
	if ($forcepan || !-e $rootfn)
	{ 
	    if (!-e $codafn)
	    {
		warn "CODA file $codafn not found, Pan analysis not done\n";
	    }
	    else	
	    {	    
		system "echo './pan -r $run'";
		system "./pan -r $run" if !$DEBUG;
	    }
	}
	else
	{
	    warn "Pan ROOT file $rootfn already exists, Pan analysis not done\n";
	}
    }

# Regression analysis =======================

    $redexists = -e $rrootfn
    if (!$noreg) 
    { 
	if ($forcereg || !$redexists) 
	{ 
	    if (!-e $rootfn) 
	    {
		warn "Pan ROOT file $rootfn not found, regression analysis not done\n"; 
	    } 
	    else
	    {
		# Note that the following system commands contain a shell
		# metacharacter (;) so are run in sh.  A simple command
		# with no metacharacter would be handled differently!!
		
		system "echo 'export PAN_CONFIG_FILE_SUFFIX=conf_reg; ./redana -r $run'";
		system "export PAN_CONFIG_FILE_SUFFIX=conf_reg; ./redana -r $run" if !$DEBUG;
	    }
	}
	else
	{
	    warn "Redana ROOT file $rrootfn already exists, regression analysis not done\n";
	}
    }

# Dithering analysis =======================

    if (!$nodit) 
    { 
	if ($forcedit || !$redexists) 
	{ 
	    if (!-e $rootfn) 
	    {
		warn "Pan ROOT file $rootfn not found, dithering analysis not done\n";
	    } 
	    else
	    {
		# Note that the following system commands contain a shell
		# metacharacter (;) so are run in sh.  A simple command
		# with no metacharacter would be handled differently!!
		
		system "echo 'export PAN_CONFIG_FILE_SUFFIX=conf_dit; ./redana -r $run'";
		system "export PAN_CONFIG_FILE_SUFFIX=conf_dit; ./redana -r $run" if !$DEBUG;
	    }
	}
	else
	{
	    warn "Redana ROOT file $rrootfn already exists, dithering analysis not done\n";
	}
    }


# prompt.macro =======================

    if (!$nomacro) 
    { 
	system "echo \"root $opts 'prompt.macro($run, $batch)'\"";
	system "root $opts 'macro/prompt.macro($run, $batch)'" if !$DEBUG;
    }
}

sub usage
{
    return <<USAGE;
Usage: prompt.pl [options] --batch [list of runs]
       prompt.pl [options] [run]
Options are 

       --forcepan --forcereg --forcedit  force Pan, regression,
                                           or dither analyses;
       --nopan --noreg --nodit           skip them;
       --nomacro                         skip prompt.macro
USAGE
}
