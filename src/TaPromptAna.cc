//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPromptAna.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Prompt data analysis.  This class derives from VaAnalysis.  It
//    simply puts differences and asymmetries of beam monitors and
//    detectors into the output root file using the AutoPairAna lists,
//    and prints statistics on these quantities periodically.
//
////////////////////////////////////////////////////////////////////////

#include "TaPromptAna.hh"
#include "VaEvent.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"

#ifndef NODICT
ClassImp(TaPromptAna)
#endif

// Constructors/destructors/operators

TaPromptAna::TaPromptAna():VaAnalysis()
{
}

TaPromptAna::~TaPromptAna(){}


// Private member functions

// We should not need to copy or assign an analysis, so copy
// constructor and operator= are private.

TaPromptAna::TaPromptAna (const TaPromptAna& copy) 
{
}


TaPromptAna& TaPromptAna::operator=( const TaPromptAna& assign) 
{ 
  return *this; 
}


void TaPromptAna::EventAnalysis()
{
  // Event analysis.

  fEvt->AddResult ( TaLabelledQuantity ( "bcm1",
					 fEvt->GetData(IBCM1), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm2",
					 fEvt->GetData(IBCM2), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "det1",
					 fEvt->GetData(IDET1), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "det2",
					 fEvt->GetData(IDET2), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "det3",
					 fEvt->GetData(IDET3), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "det4",
					 fEvt->GetData(IDET4), 
					 "chan" ) );
}

void TaPromptAna::PairAnalysis()
{ 
  // Pair analysis
  // All we have here is a call to AutoPairAna.

  AutoPairAna();
}


void
TaPromptAna::InitChanLists ()
{
  // Initialize the lists used by InitTree and AutoPairAna.
  // TaPromptAna's implementation puts channels associated with beam
  // devices and electron detectors, into the lists.

  // Initialize the lists of devices to analyze
  vector<AnaList> f;

  // Channels for which to store left and right values
  fTreeList = ChanListFromName ("helicity", "", fgNO_STATS + fgCOPY);
  f = ChanListFromName ("pairsynch", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("timeslot", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("pitadac", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("batt", "~", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~x", "mm", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~y", "mm", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bcm", "~", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("det", "~", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store differences
  f = ChanList ("batt", "~", "mchan", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~x", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~y", "um", fgDIFF);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store asymmetries
  f = ChanList ("bcm", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store normalized asymmetries
  f = ChanList ("det", "~", "ppm", fgNO_BEAM_NO_ASY + fgASYN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // multi-detector asymmetries

  // Here we assume there are four detector signals:
  //   det1 = left spectrometer,  low Q^2
  //   det2 = left spectrometer,  high Q^2
  //   det3 = right spectrometer, low Q^2
  //   det4 = right spectrometer, high Q^2
  // We do sums of left and right, sums of high and low, sum of all,
  // and average of all.

  // (Unfortunate terminology: here "right" and "left" refer to which
  // spectrometer, not which helicity.)

  vector<Int_t> keys(0);
  vector<Double_t> wts(0);

  keys.push_back(IDET1);  keys.push_back(IDET2);
  fTreeList.push_back (AnaList ("det_l", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN));
  keys.clear(); keys.push_back(IDET3);  keys.push_back(IDET4);
  fTreeList.push_back (AnaList ("det_r", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN));
  keys.clear(); keys.push_back(IDET1);  keys.push_back(IDET3);
  fTreeList.push_back (AnaList ("det_lo", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN));
  keys.clear(); keys.push_back(IDET2);  keys.push_back(IDET4);
  fTreeList.push_back (AnaList ("det_hi", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN));
  keys.clear(); keys.push_back(IDET1);  keys.push_back(IDET2);
  keys.push_back(IDET3);  keys.push_back(IDET4);
  fTreeList.push_back (AnaList ("det_all", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN));
  fTreeList.push_back (AnaList ("det_ave", keys, wts, "ppm", 
				fgNO_BEAM_NO_ASY + fgASYN + fgAVE));

}
