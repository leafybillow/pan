#ifndef PAN_TaPairFromPair
#define PAN_TaPairFromPair
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPairFromPair.hh  (interface)
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

#include "Rtypes.h"
#include "PanTypes.hh"
#include "VaPair.hh"

class TaCutList;
class TaEvent;
class TaRun;

class TaPairFromPair : public VaPair {
  
public:
  
  // Constructors/destructors/operators
  TaPairFromPair();
  TaPairFromPair(const TaPairFromPair& copy);
  TaPairFromPair& operator=( const TaPairFromPair& assign);  
  
  ~TaPairFromPair();
  
  // Major functions
  ErrCode_t RunInit(const TaRun&);
  Bool_t Fill (TaEvent&, TaRun&); // check for pair and fill
  
private:
  
  // Constants
  static const ErrCode_t fgTAPFP_ERROR;  // returned on error
  static const ErrCode_t fgTAPFP_OK;      // returned on success

  // Private member functions
  void CheckSequence (TaEvent&, TaRun&); // look for helicity/synch errors
  Bool_t ProcessPairing();
  UInt_t  RanBit(); 
  Bool_t TaPairFromPair::HelSeqOK (EHelicity h);

  // Data members
  static Bool_t  fgSkipping;   // true until first event of first full window
  static TaEvent fgThisWinEv;  // first ev of this window
  static TaEvent fgLastWinEv;  // first ev of last window
  static UInt_t  fgShreg;      // value for sequence algorithm      
  static UInt_t  fgNShreg;     // count since fgShreg was reset
  static Bool_t  fgPairMade;   // set in Fill to true if pair made, else false
  static Bool_t  fgNeedHelCheck; // need to check helicity on next first event
  static Cut_t fgSequenceNo; // cut number for sequence

#ifdef DICT
  ClassDef(TaPairFromPair, 0)  // Event pair from window pair helicity structure
#endif
};

#endif
