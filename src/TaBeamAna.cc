//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBeamAna.cc  (implementation)
//           ^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam quality analysis.
//
//////////////////////////////////////////////////////////////////////////

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
  // put everything needed to analyze one event 
  // Just for demo purposes for now we hard code four results, the
  // values of bcm1 and bcm2 (raw and corrected).

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
  // analyze a pair and compute everything needed for a beam analysis
  // we compute bcms asymmetries and bpms differences 

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




