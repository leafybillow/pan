//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBeamAna.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Beam quality analysis.  This class derives from VaAnalysis.
//    It simply puts differences and asymmetries of beam monitors
//    into the output root file using the AutoPairAna lists, and prints
//    statistics on these quantities periodically.
//
////////////////////////////////////////////////////////////////////////

#include "TaBeamAna.hh"
#include "TaEvent.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"

#ifndef NODICT
ClassImp(TaBeamAna)
#endif

// Constructors/destructors/operators

TaBeamAna::TaBeamAna():VaAnalysis()
{
}

TaBeamAna::~TaBeamAna(){}


// Private member functions

// We should not need to copy or assign an analysis, so copy
// constructor and operator= are private.

TaBeamAna::TaBeamAna (const TaBeamAna& copy) 
{
}


TaBeamAna& TaBeamAna::operator=( const TaBeamAna& assign) 
{ 
  return *this; 
}


void TaBeamAna::EventAnalysis()
{
  // Event analysis.
  // For now we hard code four results, the values of bcm1 and bcm2
  // (raw and corrected).  A somewhat more intelligent analysis 
  // should be concocted eventually.

  fEvt->AddResult ( TaLabelledQuantity ( "bcm1 raw",
					 fEvt->GetData(IBCM1R), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm1",
					 fEvt->GetData(IBCM1), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm2 raw",
					 fEvt->GetData(IBCM2R), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm2",
					 fEvt->GetData(IBCM2), 
					 "chan" ) );
}

void TaBeamAna::PairAnalysis()
{ 
  // Pair analysis
  // All we have here is a call to AutoPairAna.

  AutoPairAna();
}


void
TaBeamAna::InitChanLists ()
{
  // Initialize the lists used by InitTree and AutoPairAna.
  // TaBeamAna's implementation puts channels associated with beam
  // devices, but not detectors, into the lists.

  // Initialize the lists of devices to analyze
  vector<AnaList> f;

  // Channels for which to store left and right values
  fTreeList = ChanListFromName ("helicity", "", fgNO_STATS + fgCOPY);
  f = ChanListFromName ("quadsynch", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("pairsynch", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("quadsynch", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("timeslot", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

//    f = ChanList("timeboard","oversample","cnts", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("timeboard", "integtime","cnts",fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("timeboard", "rampdelay","cnts",fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("timeboard", "pitadac","cnts",fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("adc", "~_0", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("adc", "~_1", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("adc", "~_2", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("adc", "~_3", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

//    f = ChanList ("scaler", "~_17", "cnts", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("v2f", "~_clk", "cnts", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());

    f = ChanList ("adc_cal", "~_0", "chan", fgNO_STATS + fgCOPY);
    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_1_cal", "chan", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_2_cal", "chan", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_3_cal", "chan", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());

//    f = ChanList ("batt", "~", "chan", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());

//    f = ChanList ("bcm", "~r", "chan", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~xm", "mm", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~ym", "mm", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~xp", "mm", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~yp", "mm", fgNO_STATS + fgCOPY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());


  f = ChanList ("bpm", "~x", "mm", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~y", "mm", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("bcm", "~", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("bpm", "~xws", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~yws", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~ws", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("lumi", "~", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("bpmcav", "~x", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpmcav", "~y", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store differences

//    f = ChanList ("adc", "~_0", "chan", fgDIFF);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_1", "chan", fgDIFF);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_2", "chan", fgDIFF);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("adc", "~_3", "chan", fgDIFF);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("bpm", "~x", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~y", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("bpmcav", "~x", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpmcav", "~y", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store asymmetries
  f = ChanList ("batt", "~", "ppm", fgASY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bcm", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("lumi", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("det", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

//    f = ChanList ("bpm", "~xws", "ppm", fgNO_BEAM_NO_ASY + fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~yws", "ppm", fgNO_BEAM_NO_ASY + fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~ws", "ppm", fgNO_BEAM_NO_ASY + fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());


//    f = ChanListFromName ("adc2_0", "", fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanListFromName ("adc2_1", "", fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanListFromName ("adc2_2", "", fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanListFromName ("adc2_3", "", fgASY);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());



}
