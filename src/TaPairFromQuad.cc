//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPairFromQuad.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Class which makes and analyzes opposite helicity event pairs
//    from a data stream structured as helicity window quadruples.
//    Derived from VaPair.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY

#include "TaEvent.hh"
#include "TaPairFromQuad.hh"
#include "TaRun.hh"

#ifdef DICT
ClassImp(TaPairFromQuad)
#endif

// Static members
Int_t TaPairFromQuad::fgQuadCount;

TaPairFromQuad::TaPairFromQuad():VaPair()
{
}

TaPairFromQuad::TaPairFromQuad(const TaPairFromQuad& copy):VaPair(copy)
{
}

TaPairFromQuad &TaPairFromQuad::operator=(const TaPairFromQuad &assign)
{
  VaPair::operator=(assign);
  return *this;
}

TaPairFromQuad::~TaPairFromQuad()
{
}

ErrCode_t
TaPairFromQuad::RunInit(const TaRun& run)
{
  if (VaPair::RunInit(run) == fgVAP_ERROR)
    return fgVAP_ERROR;
  fgQuadCount = 5;
  return fgVAP_OK;
}

void TaPairFromQuad::CheckSequence( TaEvent& ThisEv, TaRun& run )
{
  // Look for sequence errors in the beam's window pair structure.
  // Errors include:
  //
  // Pairsynch unchanged from previous window
  // quadsync == FirstQS when OtherQS expected
  // quadsync == OtherQS when FirstQS expected
  // In first window of quad, helicity does not match expected value
  // In second window of quad, helicity unchanged from first window
  // In third window of quad, helicity changed from second window
  // In fourth window of quad, helicity unchanged from third window
  // In first or third window of quad, pairsynch is FirstPS
  // In second or fourth window of quad, pairsynch is SecondPS
  // In second and later events of window, pairsynch changed from first event
  // In second and later events of window, quadsynch changed from first event
  // In second and later events of window, helicity changed from first event

  const Int_t EPSCHANGE  = 0x1;
  const Int_t WPSSAME    = 0x2;
  const Int_t EHELCHANGE = 0x3;
  const Int_t WHELSAME   = 0x4;
  const Int_t WHELWRONG  = 0x5;
  const Int_t WHELCHANGE = 0x6;
  const Int_t WQSFIRST   = 0x7;
  const Int_t WQSOTHER   = 0x8;
  const Int_t EQSCHANGE  = 0x9;
  const Int_t WQSPSWRONG = 0x10;

  Int_t val = 0;

  static UInt_t gLastTimeSlot;
  Bool_t newWin = ThisEv.GetTimeSlot() == 1 ||
    ThisEv.GetTimeSlot() <= gLastTimeSlot;
  gLastTimeSlot = ThisEv.GetTimeSlot();
  
  EPairSynch lps = ThisEv.GetPairSynch();
  EQuadSynch lqs = ThisEv.GetQuadSynch();
  
  //    clog << "TaPairFromQuad::CheckSequence hel/ps/qs/ts="
  //         << " " << (ThisEv.GetDelHelicity() == RightHeli ? "R" : "L")
  //         << " " << (ThisEv.GetPairSynch() == FirstPS ? "F" : "S")
  //         << " " << (ThisEv.GetQuadSynch() == FirstQS ? "F" : "O")
  //         << " " << ThisEv.GetTimeSlot()
  //         << endl;
  
  if (newWin)
    { 
      // New window.
      // Store event for comparison to later ones
      fgLastWinEv = fgThisWinEv;
      fgThisWinEv = ThisEv;
      if (lqs == FirstQS && fgQuadCount == 5)
	fgQuadCount = 0;
      else
	{
	  fgQuadCount = (fgQuadCount + 1) % 4;
	  if (lqs == FirstQS && fgQuadCount != 0)
	    {
	      cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " unexpected first window of quad" << endl;
	      val = WQSFIRST;
	      fgQuadCount = 0;
	    }
	  else if (lqs == OtherQS && fgQuadCount == 0)
	    {
	      cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " unexpected non-first window of quad" << endl;
	      val = WQSOTHER;
	    }
	}

      if (fgQuadCount == 0)
	// See if helicity is right
	{
	  if (!HelSeqOK (ThisEv.GetDelHelicity()))
	    {
	      cout << "TaPairFromQuad::CheckEvent ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " helicity sequence error" << endl;
	      val = WHELWRONG;
	    }	      
	} 

      if (((fgQuadCount == 0 || fgQuadCount == 2) && lps == SecondPS)
	  ||
	  ((fgQuadCount == 1 || fgQuadCount == 3) && lps == FirstPS))
	// See if pairsynch is right
	{
	  cout << "TaPairFromQuad::CheckEvent ERROR: Event " 
	       << ThisEv.GetEvNumber() 
	       << " pairsynch/quadsynch mismatch" << endl;
	  val = WQSPSWRONG;
	} 

      if ( fgLastWinEv.GetEvNumber() > 0 )
	{
	  // Comparisons to last window
	  // See if pairsynch changed since last window
	  if ( lps == fgLastWinEv.GetPairSynch() )
	    {
	      cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
		   << ThisEv.GetEvNumber() 
		   << " pair synch unchanged" << endl;
	      val = WPSSAME;
	    }
	  
	  if (fgQuadCount == 1 || fgQuadCount == 3)
	    // See if helicity changed
	    {
	      if ( ThisEv.GetDelHelicity() == fgLastWinEv.GetDelHelicity() )
		{
		  cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " helicity unchanged from previous window" << endl;
		  val = WHELSAME;
		}
	    }
	  else if (fgQuadCount == 2)
	    // See if helicity unchanged
	    {
	      if ( ThisEv.GetDelHelicity() != fgLastWinEv.GetDelHelicity() )
		{
		  cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
		       << ThisEv.GetEvNumber() 
		       << " helicity changed from previous window" << endl;
		  val = WHELCHANGE;
		}
	    }
	}
    }

  if ( !newWin && fgThisWinEv.GetEvNumber() != 0 )
    {
      // Comparisons to last event
      // See if pairsynch stayed the same
      if ( lps != fgThisWinEv.GetPairSynch() )
	{
	  cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber()
	       << " pairsynch change in mid window\n";
	  val = EPSCHANGE;
	}
      // See if quadsynch stayed the same
      if ( lqs != fgThisWinEv.GetQuadSynch() )
	{
	  cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber()
	       << " quadsynch change in mid window\n";
	  val = EQSCHANGE;
	}
      // See if helicity stayed the same
      if ( ThisEv.GetDelHelicity() != fgThisWinEv.GetDelHelicity() )
	{
	  cout << "TaPairFromQuad::CheckSequence ERROR: Event " 
	       << ThisEv.GetEvNumber()
	       << " helicity change in mid window\n";
	  val = EHELCHANGE;
	}
    }
  
#ifdef NOISY
  clog << "This Event  " << ThisEv.GetEvNumber()
       << " hel/ps/qs/ts " << (UInt_t)ThisEv.GetHelicity() 
       << " " << (UInt_t)ThisEv.GetPairSynch()
       << " " << (UInt_t)ThisEv.GetQuadSynch()
       << " " << ThisEv.GetTimeSlot() << endl;
  clog << "This Window " << fgThisWinEv.GetEvNumber()
       << " hel/ps/qs/ts " << (UInt_t)fgThisWinEv.GetHelicity() 
       << " " << (UInt_t)fgThisWinEv.GetPairSynch()
       << " " << (UInt_t)fgThisWinEv.GetQuadSynch()
       << " " << fgThisWinEv.GetTimeSlot() << endl;
  if(fgLastWinEv.GetEvNumber() != 0){
     clog << "Last Window " << fgLastWinEv.GetEvNumber()
          << " hel/ps/qs/ts " << (UInt_t)fgLastWinEv.GetHelicity() 
          << " " << (UInt_t)fgLastWinEv.GetPairSynch()
          << " " << (UInt_t)fgLastWinEv.GetQuadSynch()
          << " " << fgLastWinEv.GetTimeSlot() << endl;
  }
#endif

  ThisEv.AddCut (fgSequenceNo, val);
  run.UpdateCutList (fgSequenceNo, val, ThisEv.GetEvNumber());
}


UInt_t 
TaPairFromQuad::RanBit (UInt_t hRead = 2)
{
  // Pseudorandom bit predictor.  Following algorithm taken from
  // "Numerical Recipes in C" Press, Flannery, et al., 1988.  New bit
  // is returned.  This algorithm mimics the one implemented in
  // hardware in the helicity box and is used for random helicity mode
  // to set the helicity bit for the first window of each window quad.
  // Except: if the helicity bit actually read is passed as argument,
  // it is used to update the shift register, not the generated bit.

  const UInt_t IB1 = 0x1;	        // Bit 1 mask
  const UInt_t IB3 = 0x4;	        // Bit 3 mask
  const UInt_t IB4 = 0x8;	        // Bit 4 mask
  const UInt_t IB24 = 0x800000;         // Bit 24 mask
  const UInt_t MASK = IB1+IB3+IB4+IB24;	// 100000000000000000001101
 
  int hPred = (fgShreg & IB24) ? 1 : 0;

  if ((hRead == 2 ? hPred : hRead) == 1)
    fgShreg = ((fgShreg ^ MASK) << 1) | IB1;
  else
    fgShreg <<= 1;

  return hPred;
}
