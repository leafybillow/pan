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

#ifdef DICT
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
  vector<AnaList* > f;

  // List of channels for which to store left and right values
  fCopyList = ChanList ("tir", "helicity", "");
  f = ChanList ("tir", "pairsynch", "");
  fCopyList.insert (fCopyList.end(), f.begin(), f.end());

  // List of channels for which to store differences
  fDiffList = ChanList ("bpm", "~x", "um");
  f = ChanList ("bpm", "~y", "um");
  fDiffList.insert (fDiffList.end(), f.begin(), f.end());

  // List of channels for which to store asymmetries
  fAsymList = ChanList ("bcm", "~", "ppm");

}
