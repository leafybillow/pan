//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaStandardAna.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Standard analysis.  This class derives from VaAnalysis.  It
//    simply puts differences and asymmetries of beam monitors and
//    detectors into the output root file using the AutoPairAna lists,
//    and prints statistics on these quantities periodically.
//
////////////////////////////////////////////////////////////////////////

#include "TaStandardAna.hh"
#include "VaEvent.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"
#include "TaDevice.hh"

#ifndef NODICT
ClassImp(TaStandardAna)
#endif

// Constructors/destructors/operators

TaStandardAna::TaStandardAna():VaAnalysis()
{
}

TaStandardAna::~TaStandardAna(){}


// Private member functions

// We should not need to copy or assign an analysis, so copy
// constructor and operator= are private.

TaStandardAna::TaStandardAna (const TaStandardAna& copy) 
{
}


TaStandardAna& TaStandardAna::operator=( const TaStandardAna& assign) 
{ 
  return *this; 
}


void TaStandardAna::EventAnalysis()
{
  // Event analysis.

  fEvt->AddResult ( TaLabelledQuantity ( "bcm1",
					 fEvt->GetData(IBCM1), 
					 "chan" ) );
  if (fRun->GetDevices().IsUsed(IBCM2))
    {
      fEvt->AddResult ( TaLabelledQuantity ( "bcm2",
					     fEvt->GetData(IBCM2), 
					     "chan" ) );
    }
  if (fRun->GetDevices().IsUsed(IDET1R))
    {
      fEvt->AddResult ( TaLabelledQuantity ( "det1",
					     fEvt->GetData(IDET1), 
					     "chan" ) );
    }
  if (fRun->GetDevices().IsUsed(IDET2R))
    {
      fEvt->AddResult ( TaLabelledQuantity ( "det2",
					     fEvt->GetData(IDET2), 
					     "chan" ) );
    }
  if (fRun->GetDevices().IsUsed(IDET3R))
    {
      fEvt->AddResult ( TaLabelledQuantity ( "det3",
					     fEvt->GetData(IDET3), 
					     "chan" ) );
    }
  if (fRun->GetDevices().IsUsed(IDET4R))
    {
      fEvt->AddResult ( TaLabelledQuantity ( "det4",
					     fEvt->GetData(IDET4), 
					     "chan" ) );
    }
}

void TaStandardAna::PairAnalysis()
{ 
  // Pair analysis
  // All we have here is a call to AutoPairAna.

  AutoPairAna();
}


void
TaStandardAna::InitChanLists ()
{
  // Initialize the lists used by InitTree and AutoPairAna.
  // TaStandardAna's implementation puts channels associated with beam
  // devices and electron detectors, into the lists.

  // Initialize the lists of devices to analyze
  vector<AnaList> f;


  // Channels for which to store left and right values
  fTreeList = ChanListFromName ("helicity", "", fgNO_STATS + fgCOPY);
  f = ChanListFromName ("pairsynch", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("quadsynch", "", fgNO_STATS + fgCOPY);
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
  f = ChanList ("lumi", "~", "chan", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanListFromName ("bmw", "", fgNO_STATS + fgCOPY);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store differences
  f = ChanList ("batt", "~", "mchan", fgDIFF + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~x", "um", fgDIFF + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bpm", "~y", "um", fgDIFF + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("qpd", "~x", "um", fgDIFF + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("qpd", "~y", "um", fgDIFF + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store asymmetries
//    f = ChanList ("bpm", "~xws", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~yws", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
//    f = ChanList ("bpm", "~ws", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
//    fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("bcm", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
  for (vector<AnaList>::iterator i_f = f.begin();
       i_f < f.end();
       ++i_f)
    {
      if (i_f->fVarStr == "bcm1")
	i_f->fFlagInt += fgORDERED;
      if (i_f->fVarStr == "bcm10")
	i_f->fFlagInt = fgNO_BEAM_C_NO_ASY + fgASY + fgBLINDSIGN;
    }
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("lumi", "~", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());
  f = ChanList ("qpd", "~sum", "ppm", fgNO_BEAM_NO_ASY + fgASY + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  // Channels for which to store normalized asymmetries
  f = ChanList ("lumi", "~", "ppm", fgNO_BEAM_NO_ASY + fgASYN + fgBLINDSIGN);
  fTreeList.insert (fTreeList.end(), f.begin(), f.end());

  f = ChanList ("det", "~", "ppm", fgNO_BEAM_NO_ASY + fgASYN + fgBLIND);
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
  vector<Double_t> wtsa(0);

  if (fRun->GetDevices().IsUsed(IBLUMI1R) &&
      fRun->GetDevices().IsUsed(IBLUMI2R) &&
      fRun->GetDevices().IsUsed(IBLUMI3R) &&
      fRun->GetDevices().IsUsed(IBLUMI4R) &&
      fRun->GetDevices().IsUsed(IBLUMI5R) &&
      fRun->GetDevices().IsUsed(IBLUMI6R) &&
      fRun->GetDevices().IsUsed(IBLUMI7R) &&
      fRun->GetDevices().IsUsed(IBLUMI8R))
    {
      keys.clear(); keys.push_back(IBLUMI1);  keys.push_back(IBLUMI2);
      keys.push_back(IBLUMI3);  keys.push_back(IBLUMI4);
      keys.push_back(IBLUMI5);  keys.push_back(IBLUMI6);
      keys.push_back(IBLUMI7);  keys.push_back(IBLUMI8);
      wts = fRun->GetDataBase().GetBlumiWts();
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: blumi weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("blumi_sum", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLINDSIGN));
      fTreeList.push_back (AnaList ("blumi_ave", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgAVE + fgBLINDSIGN));
    }
  if (fRun->GetDevices().IsUsed(IFLUMI1R) &&
      fRun->GetDevices().IsUsed(IFLUMI2R))
    {
      keys.clear(); keys.push_back(IFLUMI1);  keys.push_back(IFLUMI2);
      wts = fRun->GetDataBase().GetFlumiWts();
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: flumi weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("flumi_sum", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLINDSIGN));
      fTreeList.push_back (AnaList ("flumi_ave", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgAVE + fgBLINDSIGN));
    }
  if (fRun->GetDevices().IsUsed(IDET1R) &&
      fRun->GetDevices().IsUsed(IDET2R))
    {
      keys.clear(); keys.push_back(IDET1);  keys.push_back(IDET2);
      wtsa = fRun->GetDataBase().GetDetWts();
      wts.clear(); wts.push_back (wtsa[0]); wts.push_back (wtsa[1]); 
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: det1/2 weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("det_l", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLIND));
    }
  if (fRun->GetDevices().IsUsed(IDET3R) &&
      fRun->GetDevices().IsUsed(IDET4R))
    {
      keys.clear(); keys.push_back(IDET3);  keys.push_back(IDET4);
      wtsa = fRun->GetDataBase().GetDetWts();
      wts.clear(); wts.push_back (wtsa[2]); wts.push_back (wtsa[3]); 
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: det3/4 weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("det_r", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLIND));
    }
  if (fRun->GetDevices().IsUsed(IDET1R) &&
      fRun->GetDevices().IsUsed(IDET3R))
    {
      keys.clear(); keys.push_back(IDET1);  keys.push_back(IDET3);
      wtsa = fRun->GetDataBase().GetDetWts();
      wts.clear(); wts.push_back (wtsa[0]); wts.push_back (wtsa[2]); 
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: det1/3 weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("det_lo", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLIND));
    }
  if (fRun->GetDevices().IsUsed(IDET2R) &&
      fRun->GetDevices().IsUsed(IDET4R))
    {
      keys.clear(); keys.push_back(IDET2);  keys.push_back(IDET4);
      wtsa = fRun->GetDataBase().GetDetWts();
      wts.clear(); wts.push_back (wtsa[1]); wts.push_back (wtsa[3]); 
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: det2/4 weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("det_hi", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLIND));
    }
  if (fRun->GetDevices().IsUsed(IDET1R) &&
      fRun->GetDevices().IsUsed(IDET2R) &&
      fRun->GetDevices().IsUsed(IDET3R) &&
      fRun->GetDevices().IsUsed(IDET4R))
    {
      keys.clear(); keys.push_back(IDET1);  keys.push_back(IDET2);
      keys.push_back(IDET3);  keys.push_back(IDET4);
      wts = fRun->GetDataBase().GetDetWts();
      if (!WtsOK (wts)) 
	{
	  cerr << "TaStandardAna::InitChanLists WARNING: det1-4 weights bad, ignored" << endl;
	  wts.clear();
	}
      fTreeList.push_back (AnaList ("det_all", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgBLIND));
      fTreeList.push_back (AnaList ("det_ave", keys, wts, "ppm", 
				    fgNO_BEAM_NO_ASY + fgASYN + fgAVE + fgBLIND));
    }
}
