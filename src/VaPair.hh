#ifndef PAN_VaPair
#define PAN_VaPair 
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT parity analyzer  Pan           
//
//              VaPair.hh           Author : A. Vacheret, R. S. Holmes
//
//    base class of pairing process in analyis of runs     
//
//    Round 1-2 Sep-Oct 2001  AV
//    Extensively modified Dec 2001 RSH
//
//  This class is implemented as follow:
// 
//  The VaAnalysis object will create a pair object at the beginning
//  of run analysis. A event "ring" will be created and seen by all 
//  the pair object created after (it will be shared because declared 
//   as STATIC ).
//  The purpose of this object is to accummulate events and when pairing 
//  is possible, we compute results, send these results to the pair tree     
//  and send it again to the acccumpair() function of the Analysismanager.
//  if not the event is discarded.
//  In case of oversampling it will still events in the ring and a new pair
//  will created if slot numbers matched together. 
//
//////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include <deque>
#include <string>
#include <vector>

class TaEvent;
class TaLabelledQuantity;

class VaPair {
  
public:
  
  VaPair();
  VaPair(const VaPair& copy);
  VaPair& operator=( const VaPair& assign);  
  
  virtual ~VaPair();
  
  static void Init();
  virtual Bool_t Fill( TaEvent& ) = 0;
  const TaEvent& GetRight() const;
  const TaEvent& GetLeft() const;
  void QueuePrint() const;   
  void AddResult (const TaLabelledQuantity&); 
  Double_t GetDiff (string);
  Double_t GetAsy (string);
  Bool_t PassedCuts(); // True if neither event has cut condition
  const vector<TaLabelledQuantity>& GetResults() const;
  
protected:
  
  static deque< TaEvent > fgEventQueue;
  TaEvent fEvLeft;
  TaEvent fEvRight;
  vector<TaLabelledQuantity> fResults;
  
#ifdef DICT
  ClassDef( VaPair, 0 );
#endif
};

#endif
