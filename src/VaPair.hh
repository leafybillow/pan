#ifndef PAN_VaPair
#define PAN_VaPair 
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaPair.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Base class for pairs of events of opposite helicities.  Contains
// (when full) two events, as well as the results of analysis of that
// pair. Different derived classes correspond to different beam
// helicity structures, i.e., different methods of getting a pair from
// an event sequence. Methods are provided to compute differences and
// asymmetries for a given device.
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include <deque>
#include <string>
#include <vector>


class TaCutList;
class TaRun;
class TaEvent;
class TaLabelledQuantity;

class VaPair {
  
public:
  
  VaPair();
  VaPair(const VaPair& copy);
  VaPair& operator=( const VaPair& assign);  
  
  virtual ~VaPair();
  
  virtual ErrCode_t RunInit(const TaRun&);
  virtual Bool_t Fill (TaEvent&, TaRun&);  // check for pair and fill
  const TaEvent& GetRight() const;
  const TaEvent& GetLeft() const;
  void QueuePrint() const;   
  void AddResult (const TaLabelledQuantity&); 
  Double_t GetDiff (Int_t);
  Double_t GetAsy (Int_t);
  Bool_t PassedCuts(); // True if neither event has cut condition
  Bool_t PassedCutsInt(const TaCutList& cl); // True if neither event is in cut interval
  const vector<TaLabelledQuantity>& GetResults() const;
  
protected:

  // Constants
  static const ErrCode_t fgVAP_ERROR;  // returned on error
  static const ErrCode_t fgVAP_OK;      // returned on success

  // Private member functions
  virtual void CheckSequence (TaEvent&, TaRun&) = 0; // look for helicity/synch errors
  virtual UInt_t RanBit (UInt_t hRead = 2) = 0;
  virtual Bool_t HelSeqOK (EHelicity h);

  // Static data members  
  static deque< TaEvent > fgEventQueue;  // Events waiting to be paired
  static Bool_t  fgSkipping;   // true until first event of first full window pair/quad etc.
  static TaEvent fgThisWinEv;  // first ev of this window
  static TaEvent fgLastWinEv;  // first ev of last window
  static UInt_t  fgShreg;      // value for sequence algorithm      
  static UInt_t  fgNShreg;     // count since fgShreg was reset
  static Bool_t  fgPairMade;   // set in Fill to true if pair made, else false
  static Cut_t   fgSequenceNo; // cut number for sequence

  // Data members
  TaEvent fEvLeft;                       // "Left" helicity event
  TaEvent fEvRight;                      // "Right" helicity event
  vector<TaLabelledQuantity> fResults;   // Pair analysis results
  
#ifdef DICT
  ClassDef( VaPair, 0 )  // Base class for helicity pairs
#endif
};

#endif
