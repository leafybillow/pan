//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPairFromPair.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Class which makes and analyzes opposite helicity event pairs
//    from a data stream structured as helicity window pairs.  Derived
//    from VaPair.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY

#include "TaEvent.hh"
#include "TaPairFromPair.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"

#ifdef DICT
ClassImp(TaPairFromPair)
#endif

Bool_t TaPairFromPair::fgSkipping = true;
TaEvent TaPairFromPair::fgThisWinEv;
TaEvent TaPairFromPair::fgLastWinEv;
UInt_t TaPairFromPair::fgShreg = 1;      // value for sequence algorithm      
UInt_t TaPairFromPair::fgNShreg = 0;     // count since fgShreg was reset
Bool_t TaPairFromPair::fgPairMade = false;   // set in Fill to true if pair made, else false
Bool_t TaPairFromPair::fgNeedHelCheck = true; // need to check helicity on next first event

TaPairFromPair::TaPairFromPair():VaPair()
{
}

TaPairFromPair::TaPairFromPair(const TaPairFromPair& copy):VaPair(copy)
{
}

TaPairFromPair &TaPairFromPair::operator=(const TaPairFromPair &assign)
{
  VaPair::operator=(assign);
  return *this;
}

TaPairFromPair::~TaPairFromPair()
{
}

void 
TaPairFromPair::RunInit()
{
  VaPair::RunInit();
  fgSkipping = true;
  fgThisWinEv = TaEvent();
  fgLastWinEv = TaEvent();
  fgShreg = 1;
  fgNShreg = 0;
  fgPairMade = false;
  fgNeedHelCheck = true;
}

void TaPairFromPair::CheckSequence( TaEvent& ThisEv, TaRun& run )
{
  // Look for sequence errors in the beam's window pair structure.
  // Errors include:
  //
  // Pairsynch unchanged from previous window
  // In first window of pair, helicity does not match expected value
  // In second window of pair, helicity unchanged from first window
  // In second and later events of window, pairsynch changed from first event
  // In second and later events of window, helicity changed from first event

  const Int_t PSCHANGE  = 0x1;
  const Int_t PSSAME    = 0x2;
  const Int_t HELCHANGE = 0x3;
  const Int_t HELSAME   = 0x4;
  const Int_t HELWRONG  = 0x5;

  Int_t val = 0;
  if ( ThisEv.GetTimeSlot() == 1 )
    { 
      // start of new window.
      // Store event for comparison to later ones
      fgLastWinEv = fgThisWinEv;
      fgThisWinEv = ThisEv;
    }

  if ( fgLastWinEv.GetEvNumber() > 0 )
    {
      // Comparisons to last window
      // See if pairsynch changed since last window
      if ( ThisEv.GetPairSynch() == fgLastWinEv.GetPairSynch() )
	{
	  cout << "TaPairFromPair::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber() 
	       << " pair synch unchanged" << endl;
	  val = PSSAME;
	}
      
      if ( fgNeedHelCheck && (ThisEv.GetPairSynch() == FirstPS) )
	// See if helicity is right
	{
	  fgNeedHelCheck = false;
	  if (!HelSeqOK (ThisEv.GetDelHelicity()))
	    {
	      cout << "TaPairFromPair::CheckEvent ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " helicity sequence error" << endl;
	      val = HELWRONG;
	    }	      
	} 
      else if (ThisEv.GetPairSynch() == SecondPS)
	// See if helicity changed
	if ( ThisEv.GetDelHelicity() == fgLastWinEv.GetDelHelicity() )
	  {
	    fgNeedHelCheck =  true;
	    cout << "TaPairFromPair::CheckSequence ERROR: Event " 
		 << ThisEv.GetEvNumber() 
		 << " helicity unchanged" << endl;
	    val = HELSAME;
	  }
    }

  if ( ThisEv.GetTimeSlot() != 1 && fgThisWinEv.GetEvNumber() != 0 )
    {
      // Comparisons to last event
      // See if pairsynch stayed the same
      if ( ThisEv.GetPairSynch() != fgThisWinEv.GetPairSynch() )
	{
	  cout << "TaPairFromPair::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber()
	       << " pairsynch change in mid window\n";
	  val = PSCHANGE;
	}
      // See if helicity stayed the same
      if ( ThisEv.GetDelHelicity() != fgThisWinEv.GetDelHelicity() )
	{
	  cout << "TaPairFromPair::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber()
	       << " helicity change in mid window\n";
	  val = HELCHANGE;
	}
    }
  
#ifdef NOISY
  clog << "This Event  " << ThisEv.GetEvNumber()
       << " hel/ps/ts " << (UInt_t)ThisEv.GetHelicity() 
       << " " << (UInt_t)ThisEv.GetPairSynch()
       << " " << ThisEv.GetTimeSlot() << endl;
  clog << "This Window " << fgThisWinEv.GetEvNumber()
       << " hel/ps/ts " << (UInt_t)fgThisWinEv.GetHelicity() 
       << " " << (UInt_t)fgThisWinEv.GetPairSynch()
       << " " << fgThisWinEv.GetTimeSlot() << endl;
  if(fgLastWinEv.GetEvNumber() != 0){
     clog << "Last Window " << fgLastWinEv.GetEvNumber()
          << " hel/ps/ts " << (UInt_t)fgLastWinEv.GetHelicity() 
          << " " << (UInt_t)fgLastWinEv.GetPairSynch()
          << " " << fgLastWinEv.GetTimeSlot() << endl;
  }
#endif

  ThisEv.AddCut (SequenceCut, val);
  run.UpdateCutList (SequenceCut, val, ThisEv.GetEvNumber());
}


Bool_t 
TaPairFromPair::Fill( TaEvent& ThisEv, TaRun& run )
{
  // If this event makes a pair with a stored one, put the two events
  // into this pair and return true.  Otherwise store this event and
  // return false.

  Bool_t PairMade = false;
  CheckSequence (ThisEv, run);

  // Skip events until the first event of a new window
  if ( ThisEv.GetPairSynch() == FirstPS &&
       ThisEv.GetTimeSlot() == 1 )
    fgSkipping = false;

  if ( !fgSkipping )
    {
#ifdef NOISY
    clog << "Pairing event "  << ThisEv.GetEvNumber() << endl;
#endif
      // If first of a pair, store it
      if ( ThisEv.GetPairSynch() == FirstPS )
	{
	  if (fgPairMade && fgEventQueue.size() > 0)
	    {
	      // If event queue isn't empty, something is wrong: we
	      // didn't pair off all first events with second events
	      // before another first event came along.
	      cerr << "TaPairFromPair::Fill ERROR: Nothing to pair first event "
		   << fgEventQueue[0].GetEvNumber() << " with\n";
	      fgEventQueue.clear();
	      if (ThisEv.GetTimeSlot() == 1)
		fgEventQueue.push_back(ThisEv);
	      else
		fgSkipping = true;
	    }
	  else
	    fgEventQueue.push_back(ThisEv);
	}
      else
	{
	  // If second of a pair, get its partner and build the pair
	  if (fgEventQueue.size() > 0)
	    {
	      if (fgEventQueue[0].GetDelHelicity() == RightHeli)
		{
		  fEvRight = fgEventQueue[0];
		  fEvLeft = ThisEv;
		}
	      else
		{
		  fEvRight = ThisEv;
		  fEvLeft = fgEventQueue[0];
		}
	      fgEventQueue.pop_front();
	      PairMade = true;
	    }
	  else
	    {
	      // Something's wrong.  This is a second event but the
	      // queue of first events is empty.
	      cerr << "TaPairFromPair::Fill ERROR: Nothing to pair second event "
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


UInt_t 
TaPairFromPair::RanBit()
{
  // Pseudorandom bit generator.  New bit is XOR of bits 17, 22, 23, 24
  // of 24 bit shift register fgShreg.  New fgShreg is old one shifted
  // one bit left, with new bit injected at bit 1. (bit numbered 1 on
  // right to 24 on left.)  New bit is returned.  This algorithm mimics
  // the one implemented in hardware in the helicity box and is used for
  // random helicity mode to set the helicity bit for the first window
  // of each window pair.
 
  UInt_t bit24  = (fgShreg & 0x800000) != 0;
  UInt_t bit23  = (fgShreg & 0x400000) != 0;
  UInt_t bit22  = (fgShreg & 0x200000) != 0;
  UInt_t bit17  = (fgShreg & 0x010000) != 0;
  UInt_t newbit = ( bit24 ^ bit23 ^ bit22 ^ bit17 ) & 0x1;
  fgShreg = ( newbit | (fgShreg << 1 )) & 0xFFFFFF;
  return newbit; 
}


Bool_t
TaPairFromPair::HelSeqOK (EHelicity h)
{
  // Return true iff helicity h matches what we expect to find next.  
  
  // Get this helicity bit (or 2 if unknown)
  UInt_t hb = ( h == UnkHeli ? 2 :
		( h == RightHeli ? 1 : 0 ) );

  // Get expected helicity bit (or 2 if unknown)
  UInt_t eb;
  eb = RanBit();
  Bool_t expectOK = (fgNShreg++ > 24);

  if ( hb != 2 && hb != eb )
    {
      // Not the expected value, put it in shift register and
      // reset count
      fgShreg = (fgShreg & 0xFFFFFE) | hb;
      if (expectOK)
	fgNShreg = 0;
    }

#ifdef NOISY
      if ( eb == 2 || eb != hb )
	clog << "Helicity expected/got = " << eb << " " << hb 
	     << " | " << fgShreg 
	     << " fgNShreg = " << fgNShreg << endl;
#endif NOISY

  // Generate error if expected is known and does not match found

  return ( !expectOK || (eb == 2 || eb == hb ));
}
