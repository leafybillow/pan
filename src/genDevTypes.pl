#!/usr/bin/perl -w

# Script to generate DevTypes.hh
# R. Holmes Aug 2002
#
# Inputs: none
# Output to stdout.
#
# To add a new device of an existing type to DevTypes.hh, add its
# (unique) name to the appropriate list (or, for a new ADC or scaler,
# increase $adcnum or $scanum). 
#
# To add a new type of device, add a list of device names and a sub
# do_<whatever> modelled on do_striplines, do_bcms, etc., and call it
# along with the rest of the do_<whatever> subroutines below.
#
# Note that in either case you must still make appropriate changes to
# e.g. TaDevice.cc in addition.

# Hall A stripline BPMs
@halla_strlist = qw / IBPM8 IBPM10 IBPM12 IBPM4A IBPM4B /;
# Injector stripline BPMs
@inj_strlist =   qw / IBPMIN1 IBPMIN2 /;
# Cavity BPMs
@cavlist =       qw / IBPMCAV1 IBPMCAV2 IBPMCAV3 IBPMCAV4 /;
# Old (HAPPEX-I era) current monitors
@old_bcmlist =   qw / IBCM1 IBCM2 IBCM3 /;
# G0 current monitors
@g0_bcmlist =    qw / IBCMCAV1 IBCMCAV2 IBCMCAV3 IBCMCAV4 /;
# Batteries
@battlist =      qw / IBATT1 IBATT2 IBATT3 IBATT4 IBATT5 /;
# Detectors
@detlist =       qw / IDET1 IDET2 IDET3 IDET4 /;
# Lumi detectors
@lumilist =      qw / ILUMI1 ILUMI2 ILUMI3 ILUMI4 /;
# V2F clocks
@v2fclocklist =  qw / IV2F_CLK0 IV2F_CLK1 IV2F_CLK2 IV2F_CLK3 /;
# Number of ADC modules, scaler modules, and crates
$adcnum = 15;
$scanum = 4;
$ncrates = 4;

$p = 1;      # next value to be assigned
$out = "";   # output being built

&do_striplines();
&do_cavities();
&do_bcms();
&do_batts();
&do_dets();
&do_adcs();
&do_scalers();
&do_tirs();
&do_timeboards();
&do_lumis();
&do_v2fclocks();

print << "END";
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           DevTypes.hh  (definition file)
//           ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//  Defines arbitrary integer "keys" used to access data (both raw 
//  and calibrated) from fData array in TaEvent class.   
//  The keys have easy-to-remember names.  
//  Restrictions: 
//    1. the keys must be unique.
//    2. MAXKEYS must be > largest key.
//    3. ordering of keys affects TaEvent::Decode().
//
//////////////////////////////////////////////////////////////////////////
 
\#define   MAXKEYS        $p
\#define   ADCREADOUT     1
\#define   SCALREADOUT    2

// Keys for getting data from devices.
// They keys are mapped to devices automatically.
// Use these keys for  TaEvent::GetData(Int_t key)

END

print $out;

sub do_striplines
{
# Stripline BPMs

    $stroff = $p;
    $strnum = scalar (@halla_strlist) + scalar (@inj_strlist);
    $out1 = "// Hall A striplines\n";
    foreach $str (@halla_strlist)
    {
	$out1 .= &add_str ($str);
    }
    $out1 .= "// Injector striplines\n";
    foreach $str (@inj_strlist)
    {
	$out1 .= &add_str ($str);
    }
    
    $out .= << "ENDSTRCOM";
// Stripline BPMs

\#define   STROFF     $stroff     // Stripline BPMs start here
\#define   STRNUM     $strnum     // number of striplines defined below

// XP, XM, YP, YM = antennas; X, Y = calibrated position;
// XWS, YWS, WS = X, Y, and total wiresum

$out1
ENDSTRCOM
}

sub add_str
{
    my ($str) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${str}XP    $p\n"; $p++;
    $ret .= "\#define   ${str}XM    $p\n"; $p++;
    $ret .= "\#define   ${str}YP    $p\n"; $p++;
    $ret .= "\#define   ${str}YM    $p\n"; $p++;
    $ret .= "\#define   ${str}X     $p\n"; $p++;
    $ret .= "\#define   ${str}Y     $p\n"; $p++;
    $ret .= "\#define   ${str}XWS   $p\n"; $p++;
    $ret .= "\#define   ${str}YWS   $p\n"; $p++;
    $ret .= "\#define   ${str}WS    $p\n"; $p++;
    $ret .= "\n";
    return $ret;
}

sub do_cavities
{
# Cavity BPMs

    $cavoff = $p;
    $cavnum = scalar (@cavlist);
    $out1 = "";
    foreach $cav (@cavlist)
    {
	$out1 .= &add_cav ($cav);
    }
    
    $out .= << "ENDCAVCOM";
// Cavity BPMs
\#define   CAVOFF     $cavoff     // Cavity BPMs start here
\#define   CAVNUM     $cavnum     // number of cavities defined below

// XR, YR = raw data; X, Y = calibrated position

$out1
ENDCAVCOM
}

sub add_cav
{
    my ($cav) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${cav}XR    $p\n"; $p++;
    $ret .= "\#define   ${cav}YR    $p\n"; $p++;
    $ret .= "\#define   ${cav}X     $p\n"; $p++;
    $ret .= "\#define   ${cav}Y     $p\n"; $p++;
    $ret .= "\n";
    return $ret;
}

sub do_bcms
{
# BCMs

    $bcmoff = $p;
    $bcmnum = scalar (@old_bcmlist);
    $out1 = "";
    foreach $bcm (@old_bcmlist)
    {
	$out1 .= &add_bcm ($bcm);
    }
    $out .= << "ENDBCMCOM";
// Old (HAPPEX-I era) BCMs

\#define   BCMOFF     $bcmoff     // Old BCMs start here
\#define   BCMNUM     $bcmnum     // number of old BCMs defined below

// R = raw data; "" = calibrated data

$out1
ENDBCMCOM

    $ccmoff = $p;
    $ccmnum = scalar (@g0_bcmlist);
    $out1 = "";
    foreach $bcm (@g0_bcmlist)
    {
	$out1 .= &add_bcm ($bcm);
    }
    
    $out .= << "ENDCCMCOM";
// G0 BCMs

\#define   CCMOFF     $ccmoff     // G0 BCMs start here
\#define   CCMNUM     $ccmnum     // number of G0 BCMs defined below

// R = raw data; "" = calibrated data

$out1
ENDCCMCOM
}

sub add_bcm
{
    my ($bcm) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${bcm}R     $p\n"; $p++;
    $ret .= "\#define   ${bcm}      $p\n"; $p++;
    $ret .= "\n";
    return $ret;
}

sub do_batts
{
# Batteries

    $batoff = $p;
    $batnum = scalar (@battlist);
    $out1 = "";
    foreach $batt (@battlist)
    {
	$out1 .= &add_batt ($batt);
    }
    $out .= << "ENDBATTCOM";
// Batteries

\#define   BATOFF     $batoff     // Batteries start here
\#define   BATNUM     $batnum     // number of batteries defined below

$out1
ENDBATTCOM
}

sub add_batt
{
    my ($batt) = @_;
    my ($ret);
    $ret = "\#define   ${batt}      $p\n"; $p++;
    return $ret;
}

sub do_dets
{
# Detectors

    $detoff = $p;
    $detnum = scalar (@detlist);
    $out1 = "";
    foreach $det (@detlist)
    {
	$out1 .= &add_det ($det);
    }
    $out .= << "ENDDETCOM";
// Detectors

\#define   DETOFF     $detoff     // Detectorss start here
\#define   DETNUM     $detnum     // number of detectors defined below

// R = raw data; "" = calibrated data

$out1
ENDDETCOM
}

sub add_det
{
    my ($det) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${det}R     $p\n"; $p++;
    $ret .= "\#define   ${det}      $p\n"; $p++;
    $ret .= "\n";
    return $ret;
}

sub do_adcs
{
# ADCs

    $adcoff = $p;
    $out1 = "";
    for $iadc (0..$adcnum-1)
    {
	$adc = "IADC$iadc";
	$out1 .= &add_adc ($adc);
    }
    $out .= << "ENDADCCOM";
// ADC data.  Data are arranged in sequence starting with ADC0_0
// First index is the adc#, second is channel#.  Indices start at 0
// ADC# 0 - 9 are in first crate, 10 in next crate, etc.  

// First the raw data

\#define   ADCOFF     $adcoff     // Raw ADCs start here
\#define   ADCNUM     $adcnum     // number of ADCs defined below

$out1
ENDADCCOM

    $accoff = $p;
    $out1 = "";
    for $iacc (0..$adcnum-1)
    {
	$acc = "IADC_CAL$iacc";
	$out1 .= &add_adc ($acc);
    }
    $out .= << "ENDACCCOM";
// Now the calibrated data

\#define   ACCOFF     $accoff     // Calibrated ADCs start here

$out1
ENDACCCOM

    $dacoff = $p;
    $dacnum = $adcnum;
    $out1 = "";
    for $idac (0..$dacnum-1)
    {
	$dac = "IDAC$idac";
	$out1 .= &add_daccsr ($dac);
    }
    $out .= << "ENDDACCOM";
// DAC noise.

\#define   DACOFF     $dacoff     // DACs start here
\#define   DACNUM     $dacnum     // number of DACs defined below

$out1
ENDDACCOM

    $csroff = $p;
    $csrnum = $adcnum;
    $out1 = "";
    for $icsr (0..$csrnum-1)
    {
	$csr = "ICSR$icsr";
	$out1 .= &add_daccsr ($csr);
    }
    $out .= << "ENDCSRCOM";
// CSRs

\#define   CSROFF     $csroff     // CSRs start here
\#define   CSRNUM     $csrnum     // number of CSRs defined below

$out1
ENDCSRCOM
}

sub add_adc
{
    my ($adc) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${adc}_0     $p\n"; $p++;
    $ret .= "\#define   ${adc}_1     $p\n"; $p++;
    $ret .= "\#define   ${adc}_2     $p\n"; $p++;
    $ret .= "\#define   ${adc}_3     $p\n"; $p++;
    return $ret;
}

sub add_daccsr
{
    my ($adc) = @_;
    my ($ret);
    $ret = "\#define   ${adc}      $p\n"; $p++;
    return $ret;
}

sub do_scalers
{
# Scalers

    $scaoff = $p;
    $out1 = "";
    for $iscaler (0..$scanum-1)
    {
	$scaler = "ISCALER$iscaler";
	$out1 .= &add_scaler ($scaler);
    }
    $out .= << "ENDSCALERCOM";
// Raw scalers

\#define   SCAOFF     $scaoff     // Raw SCALERs start here
\#define   SCANUM     $scanum     // number of SCALERs defined below

$out1
ENDSCALERCOM

    $sccoff = $p;
    $out1 = "";
    for $iscc (0..$scanum-1)
    {
	$scc = "ICALSCA$iscc";
	$out1 .= &add_scaler ($scc);
    }
    $out .= << "ENDSCCCOM";
// Calibrated scalers

\#define   SCCOFF     $sccoff     // Calibrated SCALERs start here

$out1
ENDSCCCOM
}

sub add_scaler
{
    my ($scaler) = @_;
    my ($ret);
    $ret = "";
    for $i (0..31)
    {
	$ret .= "\#define   ${scaler}_$i     $p\n"; $p++;
    }
    $ret .= "\n";
    return $ret;
}

sub do_tirs
{
# TIRs

    $tiroff = $p;
    $out1 = &add_tir ("ITIRDATA");
    $out .= << "ENDTIRCOM";
// TIR data from various crates

\#define   TIROFF     $tiroff     // TIRs start here
\#define   TIRNUM     $ncrates     // number of TIRs defined below

$out1
ENDTIRCOM

    $heloff = $p;
    $out1 = &add_tir ("IHELICITY");
    $out .= << "ENDHELCOM";
// Helicity info from various crates

\#define   HELOFF     $heloff     // Helicities start here
\#define   HELNUM     $ncrates     // number of HELs defined below

$out1
ENDHELCOM

    $timoff = $p;
    $out1 = &add_tir ("ITIMESLOT");
    $out .= << "ENDTIMCOM";
// Timeslot info from various crates

\#define   TIMOFF     $timoff     // Timeslots start here
\#define   TIMNUM     $ncrates     // number of timeslots defined below

$out1
ENDTIMCOM

    $paroff = $p;
    $out1 = &add_tir ("IPAIRSYNCH");
    $out .= << "ENDPARCOM";
// Pairsynch info from various crates

\#define   PAROFF     $paroff     // Pairsynchs start here
\#define   PARNUM     $ncrates     // number of pairsynchs defined below

$out1
ENDPARCOM
    $qudoff = $p;
    $out1 = &add_tir ("IQUADSYNCH");
    $out .= << "ENDQUDCOM";
// Quadsynch info from various crates

\#define   QUDOFF     $qudoff     // Quadsynchs start here
\#define   QUDNUM     $ncrates     // number of quadsynchs defined below

$out1
ENDQUDCOM
}

sub do_timeboards
{
# Timeboards

    $tbdoff = $p;
    $out1 = "";
    $out1 .= &add_tir ("ITIMEBOARD");
    $out1 .= "\n";
    $out1 .= &add_tir ("IRAMPDELAY");
    $out1 .= "\n";
    $out1 .= &add_tir ("IINTEGTIME");
    $out1 .= "\n";
    $out1 .= &add_tir ("IOVERSAMPLE");
    $out1 .= "\n";
    $out1 .= &add_tir ("IPRECDAC");
    $out1 .= "\n";
    $out1 .= &add_pitadac ("IPITADAC");

    $out .= << "ENDTBDCOM";
// Timeboard data from various crates

\#define   TBDOFF     $tbdoff     // Timeboards start here
\#define   TBDNUM     $ncrates     // number of timeboards defined below

$out1
ENDTBDCOM
}

sub add_tir
{
    my ($tir) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${tir}        $p\n"; $p++;
    $ret .= "\#define   ${tir}1       $p\n"; $p++;
    $ret .= "\#define   ${tir}2       $p\n"; $p++;
    $ret .= "\#define   ${tir}3       $p\n"; $p++;
    return $ret;
}

sub add_pitadac
{
    my ($pita) = @_;
    my ($ret);
    $ret = "\#define   ${pita}        $p\n"; $p++;
    return $ret;
}

sub do_lumis
{
# Lumi detectors

    $lmioff = $p;
    $lminum = scalar (@lumilist);
    $out1 = "";
    foreach $lumi (@lumilist)
    {
	$out1 .= &add_lumi ($lumi);
    }
    $out .= << "ENDLUMICOM";
// Lumi detectors

\#define   LMIOFF     $lmioff     // Lumis start here
\#define   LMINUM     $lminum     // number of lumis defined below

// R = raw data; "" = calibrated data

$out1
ENDLUMICOM
}

sub add_lumi
{
    my ($lumi) = @_;
    my ($ret);
    $ret = "";
    $ret .= "\#define   ${lumi}R     $p\n"; $p++;
    $ret .= "\#define   ${lumi}      $p\n"; $p++;
    $ret .= "\n";
    return $ret;
}

sub do_v2fclocks
{
# V2F Clocks

    $v2fclkoff = $p;
    $v2fclknum = scalar (@v2fclocklist);
    $out1 = "";
    foreach $v2fclock (@v2fclocklist)
    {
	$out1 .= &add_v2fclock ($v2fclock);
    }
    $out .= << "ENDV2FCLOCKCOM";
// V2f_Clocks

\#define   V2FCLKOFF     $v2fclkoff     // V2F clocks start here
\#define   V2FCLKNUM     $v2fclknum     // number of V2F clocks defined below

$out1
ENDV2FCLOCKCOM
}

sub add_v2fclock
{
    my ($v2fclock) = @_;
    my ($ret);
    $ret = "\#define   ${v2fclock}      $p\n"; $p++;
    return $ret;
}

