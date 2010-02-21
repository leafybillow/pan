//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaMultiplet.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Class for groups of two or more pairs of events.
//
////////////////////////////////////////////////////////////////////////


#include "VaPair.hh"
#include "TaLabelledQuantity.hh"
#include "TaMultiplet.hh" 
#include "TaRun.hh"
#include "TaString.hh"
#include "TaDataBase.hh"

//#define NOISY

#ifndef NODICT
ClassImp(TaMultiplet)
#endif

// Static members
const ErrCode_t TaMultiplet::fgTAM_OK = 0;
const ErrCode_t TaMultiplet::fgTAM_ERROR = -1;

TaMultiplet::TaMultiplet (const UInt_t n) 
  : fN(n)
{
  fPairs.resize (n);
  for (UInt_t i = 0; i < fN; ++i)
    {
      fPairs[i] = NULL;
    }
  fPairsIndex = 0;
}


TaMultiplet::TaMultiplet (const TaMultiplet& copy)
{
  fN = copy.fN;
  fPairs = copy.fPairs;
  fPairsIndex = copy.fPairsIndex;
  fResults = copy.fResults;
}


TaMultiplet &
TaMultiplet::operator= (const TaMultiplet &assign)
{
  if (this != &assign)
    { 
      for (UInt_t i = 0; i < fN; ++i)
	{
	  delete fPairs[i];
	}
      fN = assign.fN;
      fPairs = assign.fPairs;
      fPairsIndex = assign.fPairsIndex;
      fResults = assign.fResults;
    } 
  return *this;
}


TaMultiplet::~TaMultiplet()
{
  for (UInt_t i = 0; i < fN; ++i)
    {
      delete fPairs[i];
    }
}


ErrCode_t
TaMultiplet::RunInit (const TaRun& run)
{
  // Initialization at start of run
  return fgTAM_OK;
}


Bool_t 
TaMultiplet::Fill (VaPair& thisPair) 
{
  // Insert this pair into the list and return true iff this makes a
  // full multiplet.

  EMultipletSynch ms = (thisPair.GetFirst().GetMultipletSynch());

  if (fPairsIndex == fN || fPairsIndex == 0)
    {
      fPairsIndex = 0;
      fResults.clear();
      if (ms != FirstMS)
	{
	  cerr << "TaMultiplet::Fill ERROR: Multiplet empty and MultipletSynch != first (ignore at start of run)" << endl;
	  return false;
	}
    }
  else
    if (ms == FirstMS)
	{
	  cerr << "TaMultiplet::Fill ERROR: Multiplet nonempty and MultipletSynch == first" << endl;
	  return false;
	}

  delete fPairs[fPairsIndex];
  fPairs[fPairsIndex] = &thisPair;
  fPairsIndex++;
  return (fPairsIndex == fN);
}

const VaPair& 
TaMultiplet::GetPair (const UInt_t n) const
{
  return n < fN ? *(fPairs[n]) : *(fPairs[0]);
}

Bool_t
TaMultiplet::DeletePair (const UInt_t n)
{
  // Delete pair n, or pair in next slot to be filled (default)
  // Return true iff a delete was done

  UInt_t nn;
  if (n == 999)
    nn = fPairsIndex == fN ? 0 : fPairsIndex;
  else
    nn = n;
  if (nn < fPairs.size() && fPairs[nn] != NULL)
    {
      delete fPairs[nn];
      fPairs[nn] = NULL;
      return true;
    }
  return false;
}

void 
TaMultiplet::AddResult (const TaLabelledQuantity& lq)
{
  // Insert multiplet analysis results into multiplet.
  fResults.push_back (lq);
}

Double_t
TaMultiplet::GetRightSum (Int_t key) const
{
  // Get sum over pairs of quantity indexed by key for right events in
  // this multiplet.
  Double_t sum = 0;
  for (vector<VaPair*>::const_iterator pairItr  = fPairs.begin(); 
       pairItr != fPairs.end(); 
       pairItr++)
    sum += (*pairItr)->GetRight().GetData (key);
  return sum;
}

Double_t
TaMultiplet::GetLeftSum (Int_t key) const
{
  // Get sum over pairs of quantity indexed by key for left events in
  // this multiplet.
  Double_t sum = 0;
  for (vector<VaPair*>::const_iterator pairItr  = fPairs.begin(); 
       pairItr != fPairs.end(); 
       pairItr++)
    sum += (*pairItr)->GetLeft().GetData (key);
  return sum;
}

Double_t
TaMultiplet::GetRightSumSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get sum over pairs of weighted sum of quantities indexed by keys
  // for right events in this multiplet.
  Double_t sum = 0;
  for (vector<VaPair*>::const_iterator pairItr  = fPairs.begin(); 
       pairItr != fPairs.end(); 
       pairItr++)
    sum += (*pairItr)->GetRight().GetDataSum (keys, wts);
  return sum;
}

Double_t
TaMultiplet::GetLeftSumSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get sum over pairs of weighted sum of quantities indexed by keys
  // for left events in this multiplet.
  Double_t sum = 0;
  for (vector<VaPair*>::const_iterator pairItr  = fPairs.begin(); 
       pairItr != fPairs.end(); 
       pairItr++)
    sum += (*pairItr)->GetLeft().GetDataSum (keys, wts);
  return sum;
}

Double_t 
TaMultiplet::GetDiff (Int_t key) const
{
  // Get difference in quantity indexed by key for this multiplet.
  return GetRightSum(key) - GetLeftSum(key);
}


Double_t 
TaMultiplet::GetDiffSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get difference in weighted sum of quantities indexed by keys for
  // this multiplet.

  return GetRightSumSum (keys, wts) - GetLeftSumSum (keys, wts);
}


Double_t 
TaMultiplet::GetAvg (Int_t key) const
{
  // Get average quantity indexed by key for this multiplet.
  return (GetRightSum (key) + GetLeftSum (key)) / 2.0 / fN;
}


Double_t 
TaMultiplet::GetAvgSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get average weighted sum of quantities indexed by keys for
  // this multiplet.

  return (GetRightSumSum (keys, wts) + 
	  GetLeftSumSum (keys, wts)) / 2.0 / fN;
}

Double_t 
TaMultiplet::GetAvgN (Int_t key, Int_t curmonkey) const
{
  // Get normalized average in quantity indexed by key for this multiplet.

  return ((GetRightSum (key) / GetRightSum (curmonkey))
	  + (GetLeftSum (key) / GetLeftSum (curmonkey))) / 2.0 / fN;
}


Double_t 
TaMultiplet::GetAvgNSum (vector<Int_t> keys, Int_t curmonkey, vector<Double_t> wts) const
{
  // Get normalized average weighted sum of quantities indexed by keys for
  // this multiplet.

  return (((GetRightSumSum (keys, wts)) / GetRightSum (curmonkey)) 
	  + ((GetLeftSumSum (keys, wts)) / GetLeftSum (curmonkey))) / 2.0 / fN;
}


Double_t 
TaMultiplet::GetAsy (Int_t key) const
{
  // Get asymmetry in quantity indexed by key for this multiplet.

  Double_t denom = GetRightSum (key) + GetLeftSum (key);
  if (denom <= 0)
    return -1;
  return (GetRightSum (key) - GetLeftSum (key)) / denom;
}

Double_t 
TaMultiplet::GetAsyN (Int_t key, Int_t curmonkey) const
{
  // Get normalized asymmetry in quantity indexed by key for this multiplet.

  Double_t denom = GetRightSum (key) / GetRightSum (curmonkey)
    + GetLeftSum (key) / GetLeftSum (curmonkey);
  if (denom <= 0)
    return -1;
  return (GetRightSum (key) / GetRightSum (curmonkey)
	  - GetLeftSum (key) / GetLeftSum (curmonkey)) / denom;
}


Double_t 
TaMultiplet::GetAsySum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get asymmetry in weighted sum of quantities indexed by keys for
  // this multiplet.

  Double_t denom = GetRightSumSum (keys, wts) + GetLeftSumSum (keys, wts);
  if (denom <= 0)
    return -1;
  return (GetRightSumSum (keys, wts) - GetLeftSumSum (keys, wts)) / denom;
}


Double_t 
TaMultiplet::GetAsyNSum (vector<Int_t> keys, Int_t curmonkey, vector<Double_t> wts) const
{
  // Get normalized asymmetry in weighted sum of quantities indexed by keys for
  // this multiplet.

  Double_t denom = (GetRightSumSum (keys, wts)) / GetRightSum (curmonkey) + 
    (GetLeftSumSum (keys, wts)) / GetLeftSum (curmonkey);
  if (denom <= 0)
    return -1;
  Double_t numer = ((GetRightSumSum (keys, wts)) / GetRightSum (curmonkey)) - 
		    ((GetLeftSumSum (keys, wts)) / GetLeftSum (curmonkey));

  return  numer/denom;
}


Double_t 
TaMultiplet::GetAsyAve (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get weighted average of asymmetries of quantities indexed by keys
  // for this multiplet.

  Double_t sumxw = 0;
  Double_t sumw = 0;

  if (wts.size() == 0)
    for (vector<Int_t>::const_iterator p = keys.begin();
	 p != keys.end();
	 ++p)
      {
	sumxw += GetAsy (*p);
	sumw++;
      }
  else if (wts.size() != keys.size())
    cerr << "TaMultiplet::GetAsyAve ERROR: Weight and key vector sizes differ" << endl;
  else
    for (size_t i = 0; i < keys.size(); ++i)
      {
	sumxw += wts[i] * GetAsy (keys[i]);
	sumw += wts[i];
      }

  if (sumw <= 0)
    {
      cerr << "TaMultiplet::GetAsyAve ERROR: Weight sum non-positive" << endl;
      return 0;
    }

  return sumxw / sumw;
}

Double_t 
TaMultiplet::GetAsyNAve (vector<Int_t> keys, Int_t curmonkey, vector<Double_t> wts) const
{
  // Get weighted average of normalized asymmetries of quantities
  // indexed by keys for this multiplet.

  Double_t sumxw = 0;
  Double_t sumw = 0;

  if (wts.size() == 0)
    for (vector<Int_t>::const_iterator p = keys.begin();
	 p != keys.end();
	 ++p)
      {
	sumxw += GetAsyN (*p, curmonkey);
	sumw++;
      }
  else if (wts.size() != keys.size())
    cerr << "TaMultiplet::GetAsyNAve ERROR: Weight and key vector sizes differ" << endl;
  else
    for (size_t i = 0; i < keys.size(); ++i)
      {
	sumxw += wts[i] * GetAsyN (keys[i], curmonkey);
	sumw += wts[i];
      }

  if (sumw <= 0)
    {
      cerr << "TaMultiplet::GetAsyNAve ERROR: Weight sum non-positive" << endl;
      return 0;
    }
  return sumxw / sumw;
}


Bool_t 
TaMultiplet::PassedCuts() const
{
  // True if no event has cut condition

  vector<VaPair*>::const_iterator pairItr;
  for (pairItr  = fPairs.begin(); 
       pairItr != fPairs.end() && (*pairItr)->PassedCuts(); 
       pairItr++)
    {} // empty loop body

  return pairItr == fPairs.end();
}

Bool_t 
TaMultiplet::PassedCutsInt (const TaCutList& cl) const
{
  // True if no event is in cut interval

  vector<VaPair*>::const_iterator pairItr;
  for (pairItr  = fPairs.begin(); 
       pairItr != fPairs.end() && (*pairItr)->PassedCutsInt (cl); 
       pairItr++)
    {} // empty loop body

  return pairItr == fPairs.end();
}

Bool_t 
TaMultiplet::PassedCCutsInt (const TaCutList& cl) const
{
  // True if no event is in cut interval (for hall C)

  vector<VaPair*>::const_iterator pairItr;
  for (pairItr  = fPairs.begin(); 
       pairItr != fPairs.end() && (*pairItr)->PassedCCutsInt (cl); 
       pairItr++)
    {} // empty loop body

  return pairItr == fPairs.end();
}

Bool_t 
TaMultiplet::BeamCut () const
{
  // True if any event has beam cut
  vector<VaPair*>::const_iterator pairItr;
  for (pairItr  = fPairs.begin(); 
       pairItr != fPairs.end() && !((*pairItr)->BeamCut());
       pairItr++)
    {} // empty loop body

  return pairItr != fPairs.end();
}

Bool_t 
TaMultiplet::BeamCCut () const
{
  // True if any event has Hall C beam cut
  vector<VaPair*>::const_iterator pairItr;
  for (pairItr  = fPairs.begin(); 
       pairItr != fPairs.end() && !((*pairItr)->BeamCCut());
       pairItr++)
    {} // empty loop body

  return pairItr != fPairs.end();
}



const vector<TaLabelledQuantity>& 
TaMultiplet::GetResults() const
{
  return fResults;
}


// Private member functions

