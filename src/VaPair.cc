//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaPair.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Base class for pairs of events of opposite helicities.  Contains
// (when full) two events, as well as the results of analysis of that
// pair. Different derived classes correspond to different beam
// helicity structures. Methods are provided to compute differences and
// asymmetries for a given device.
//
////////////////////////////////////////////////////////////////////////


#include "TaCutList.hh"
#include "VaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "VaPair.hh" 
#include "TaRun.hh"
#include "TaString.hh"
#include "TaDataBase.hh"

//#define NOISY

#ifndef NODICT
ClassImp(VaPair)
#endif

// Static members
const ErrCode_t VaPair::fgVAP_OK = 0;
const ErrCode_t VaPair::fgVAP_ERROR = -1;
deque< VaEvent > 
VaPair::fgEventQueue;
Bool_t VaPair::fgSkipping = true;
VaEvent VaPair::fgThisWinEv;
VaEvent VaPair::fgLastWinEv;
UInt_t VaPair::fgShreg = 1;
UInt_t VaPair::fgNShreg = 0;
Bool_t VaPair::fgPairMade = false;
UInt_t VaPair::fgNCuts;
Cut_t VaPair::fgSequenceNo;
UInt_t VaPair::fgOldb = 2;
Bool_t VaPair::fgRandom = 1;


VaPair::VaPair() :
  fEvFirst(0),
  fEvSecond(0)
{
}


VaPair::VaPair(const VaPair& copy)
{
  fResults = copy.fResults;
  fEvLeft = copy.fEvLeft;
  fEvRight = copy.fEvRight;
  if (copy.fEvFirst == &(copy.fEvLeft))
    {
      fEvFirst = &fEvLeft;
      fEvSecond = &fEvRight;
    }
  else if (copy.fEvFirst == &(copy.fEvRight))
    {
      fEvFirst = &fEvRight;
      fEvSecond = &fEvLeft;
    }
  else
    {
      fEvFirst = 0;
      fEvSecond = 0;
    }
}


VaPair &
VaPair::operator=(const VaPair &assign)
{
  if (this != &assign)
    { 
      fResults = assign.fResults;
      fEvLeft = assign.fEvLeft;
      fEvRight = assign.fEvRight;
      if (assign.fEvFirst == &(assign.fEvLeft))
	{
	  fEvFirst = &fEvLeft;
	  fEvSecond = &fEvRight;
	}
      else if (assign.fEvFirst == &(assign.fEvRight))
	{
	  fEvFirst = &fEvRight;
	  fEvSecond = &fEvLeft;
	}
      else
	{
	  fEvFirst = 0;
	  fEvSecond = 0;
	}
    } 
  return *this;
}


VaPair::~VaPair()
{
}


ErrCode_t
VaPair::RunInit(const TaRun& run)
{
  // Initialization at start of run -- clear event queue (so we don't
  // pair events from two different runs, for instance)
  fgEventQueue.clear();
  fgSkipping = true;
  fgThisWinEv = VaEvent();
  fgLastWinEv = VaEvent();
  fgShreg = 1;
  fgNShreg = 0;
  fgPairMade = false;

  fgNCuts = (UInt_t) run.GetDataBase().GetNumCuts();
  fgSequenceNo = run.GetDataBase().GetCutNumber("Pair_seq");
  if (fgSequenceNo == fgNCuts)
    fgSequenceNo = run.GetDataBase().GetCutNumber("Sequence"); // backward compat
  if (fgSequenceNo == fgNCuts)
    {
      cerr << "VaPair::RunInit WARNING: Following cut(s) are not defined "
	   << "in database and are required:";
      if (fgSequenceNo == fgNCuts) cerr << " Pair_seq";
      cerr << endl;
      return fgVAP_ERROR;
    }

  // Since there are no non-required pair cuts the following code is
  // commented out (but left for possible future use)

  //   if (fgSequenceNo == fgNCuts)
  //     {
  //       cerr << "VaPair::RunInit WARNING: Following cut(s) are not defined "
  // 	   << "in database and will not be imposed:";
  //       if (fgSequenceNo == fgNCuts) cerr << " Pair_seq";
  //       cerr << endl;
  //     }
  
  // If randomheli is "no" in database set fgRandom to 0, else 1.
  TaString sran = run.GetDataBase().GetRandomHeli();
  fgRandom = sran.CmpNoCase ("no") == 0 ? 0 : 1;

  return fgVAP_OK;
}


Bool_t 
VaPair::Fill( VaEvent& ThisEv, TaRun& run )
{
  // If this event makes a pair with a stored one, put the two events
  // into this pair and return true.  Otherwise store this event and
  // return false.

  Bool_t PairMade = false;
  CheckSequence (ThisEv, run);

  // Skip events until the first event of a new window pair
  if ( ThisEv.GetPairSynch() == FirstPS &&
       ThisEv.GetTimeSlot() == 1 )
    fgSkipping = false;

  if ( !fgSkipping )
    {
#ifdef NOISY
    clog << "Pairing event "  << ThisEv.GetEvNumber() << ": ";
#endif
      // If first of a pair, store it
      if ( ThisEv.GetPairSynch() == FirstPS )
	{
	  if (fgPairMade && fgEventQueue.size() > 0)
	    {
	      // If event queue isn't empty, something is wrong: we
	      // didn't pair off all first events with second events
	      // before another first event came along.
	      cerr << "VaPair::Fill ERROR: Nothing to pair first event "
		   << fgEventQueue[0].GetEvNumber() << " with\n";
	      fgEventQueue.clear();
	      if (ThisEv.GetTimeSlot() == 1)
		fgEventQueue.push_back(ThisEv);
	      else
		fgSkipping = true;
	    }
	  else
	    {
	      fgEventQueue.push_back(ThisEv);
#ifdef NOISY
	      clog << " storing" << endl;
#endif
	    }
	}
      else
	{
	  // If second of a pair, get its partner and build the pair
	  if (fgEventQueue.size() > 0)
	    {
#ifdef NOISY
	      clog << "pairing with event " << fgEventQueue[0].GetEvNumber() << endl;
#endif
	      if (fgEventQueue[0].GetHelicity() == RightHeli)
		{
		  fEvRight = fgEventQueue[0];
		  fEvLeft = ThisEv;
		  fEvFirst = &fEvRight;
		  fEvSecond = &fEvLeft;
		}
	      else
		{
		  fEvRight = ThisEv;
		  fEvLeft = fgEventQueue[0];
		  fEvFirst = &fEvLeft;
		  fEvSecond = &fEvRight;
		}
	      fgEventQueue.pop_front();
	      PairMade = true;
	    }
	  else
	    {
	      // Something's wrong.  This is a second event but the
	      // queue of first events is empty.
	      cerr << "VaPair::Fill ERROR: Nothing to pair second event "
		   << ThisEv.GetEvNumber() << " with\n";
	      fgSkipping = true;
	    }
	}
    }
#ifdef NOISY
  else
    clog << "Skipping event " << ThisEv.GetEvNumber() << endl;
#endif

  fgPairMade = PairMade;
  return PairMade;
}


const VaEvent& 
VaPair::GetRight() const
{
  return fEvRight;
}


const VaEvent& 
VaPair::GetLeft() const
{
  return fEvLeft;
}


const VaEvent& 
VaPair::GetFirst() const
{
  return *fEvFirst;
}


const VaEvent& 
VaPair::GetSecond() const
{
  return *fEvSecond;
}


void 
VaPair::QueuePrint() const
{
  // Print stored events, for diagnostic purposes

  if ( fgEventQueue.size() !=0 )
    {
      cout<<" ----- PAIR event queue print out -------- "<<endl;
      for ( deque< VaEvent >::iterator fEvIdx  = fgEventQueue.begin() ; 
            fEvIdx != fgEventQueue.end() ; 
            fEvIdx++ )
        cout<<"event number "<< (*fEvIdx).GetEvNumber() << endl;
      cout<<" ----------------------------------------- " <<endl;
    }
  else cout<< " Pair Event ring empty, no print out "<<endl; 
}


void 
VaPair::AddResult (const TaLabelledQuantity& lq)
{
  // Insert pair analysis results into pair.
  fResults.push_back(lq);
}


Double_t 
VaPair::GetDiff (Int_t key) const
{
  // Get difference in quantity indexed by key for this pair.
  return GetRight().GetData(key) - GetLeft().GetData(key);
}


Double_t 
VaPair::GetDiffSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get difference in weighted sum of quantities indexed by keys for
  // this pair.

  return GetRight().GetDataSum (keys, wts) - GetLeft().GetDataSum (keys, wts);
}


Double_t 
VaPair::GetAsy (Int_t key) const
{
  // Get asymmetry in quantity indexed by key for this pair.

  Double_t denom = GetRight().GetData(key) + GetLeft().GetData(key);
  if ( denom <= 0 )
    {
//        cerr << "VaPair::GetAsy ERROR: Denominator is <= zero, key = " 
//  	   << key << endl;
      return -1;
    }
  return (GetRight().GetData(key) - GetLeft().GetData(key)) / denom;
}


Double_t 
VaPair::GetAsySum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get asymmetry in weighted sum of quantities indexed by keys for
  // this pair.

  Double_t denom = GetRight().GetDataSum (keys, wts) + 
    GetLeft().GetDataSum (keys, wts);
  if ( denom <= 0 )
    {
      cerr << "VaPair::GetAsySum ERROR: Denominator is <= zero" << endl;
      return -1;
    }
  return (GetRight().GetDataSum (keys, wts) - 
	  GetLeft().GetDataSum (keys, wts)) / denom;
}


Double_t 
VaPair::GetAsyAve (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get weighted average of asymmetries of quantities indexed by keys
  // for this pair.

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
    cerr << "VaPair::GetAsyAve ERROR: Weight and key vector sizes differ" << endl;
  else
    for (size_t i = 0; i < keys.size(); ++i)
      {
	sumxw += wts[i] * GetAsy (keys[i]);
	sumw += wts[i];
      }

  if (sumw <= 0)
    {
      cerr << "VaPair::GetAsyAve ERROR: Weight sum non-positive" << endl;
      return 0;
    }

  return sumxw / sumw;
}


Bool_t 
VaPair::PassedCuts() const
{
// True if neither event has cut condition

  return ! ( GetLeft().CutStatus() || GetRight().CutStatus() );
}

Bool_t 
VaPair::PassedCutsInt(const TaCutList& cl) const
{
// True if neither event is in cut interval

  return ( cl.OK(GetLeft()) && cl.OK(GetRight()) );
}

Bool_t 
VaPair::PassedCCutsInt(const TaCutList& cl) const
{
// True if neither event is in cut interval (for hall C)

  return ( cl.OKC(GetLeft()) && cl.OKC(GetRight()) );
}

const vector<TaLabelledQuantity>& 
VaPair::GetResults() const
{
  return fResults;
}


// Private member functions

Bool_t
VaPair::HelSeqOK (EHelicity h)
{
  // Return true iff helicity h matches what we expect to find next.  
  
  // Get this helicity bit (or 2 if unknown)
  UInt_t hb = ( h == UnkHeli ? 2 :
		( h == RightHeli ? 0 : 1 ) );

  // Get expected helicity bit (or 2 if unknown)
  UInt_t eb;
  if (fgRandom)
    eb = RanBit (hb);
  else
    {
      eb = fgOldb;
      fgOldb = hb;
    }

  Bool_t expectOK = (fgNShreg++ > 24);

  if ( expectOK && hb != 2 && hb != eb )
    {
      // Not the expected value, reset count
	fgNShreg = 0;
    }

#ifdef NOISY
  clog << "Helicity expected/got = " << eb << " " << hb;
  if (fgRandom)
    clog << " | " << fgShreg;
  clog << " fgNShreg = " << fgNShreg;
  if ( eb == 2 || eb != hb )
    clog << " (MISMATCH)";
  clog  << endl;
#endif

  // Generate error if expected is known and does not match found

  return ( !expectOK || (eb == 2 || eb == hb ));
}
