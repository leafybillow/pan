//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           VaAnalysis.hh  (header file)
//           ^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Analysis base class.
//
//////////////////////////////////////////////////////////////////////////

#ifndef PAN_VaAnalysis
#define PAN_VaAnalysis

#include "Rtypes.h"
#include "TTree.h"
#include "PanTypes.hh"
#include <deque>
#include <string>
#include <strstream>
#include <utility>
#include <vector>

class TaRun;
class TaEvent;
class VaPair;

class VaAnalysis
{
  // Base class for analysis.  

  // Most of the member functions are virtual, although some have
  // default implementations that probably will not need to be changed
  // by most derived classes.

  // Calling function should call Init, RunIni, Process, RunFini, and
  // Finish in that order.  These respectively initialize the whole
  // analysis; initialize the analysis of a run (somewhat redundant
  // since at present Pan handles only one run per analysis); process
  // the run; finish processing the run (again somewhat redundant);
  // and finish the analysis.

public:

  // Constructors/destructors/operators
  VaAnalysis();
  virtual ~VaAnalysis();

  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are protected.

  // Major functions
  virtual void Init();
  virtual void RunIni(TaRun&);
  virtual void ProcessRun();
  virtual void RunFini();
  virtual void Finish();

  // Data access functions
  size_t PairsLeft() const { return fPDeque.size(); }

protected:

  // Protected member functions

  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are protected.
  VaAnalysis( const VaAnalysis& copy);
  VaAnalysis& operator=(const VaAnalysis& assign);

  // PreProcessEvt checks events, puts them on the event delay queues,
  // puts events into pairs and pushes pairs onto the pair delay
  // queue.  ProcessEvt (ProcessPair) handles analysis of each event
  // (pair) that has passed through the event (pair) delay queues.  The
  // actual analysis happens in protected member functions
  // EventAnalysis and PairAnalysis; these are pure virtual and must
  // be supplied by the derived classes.

  virtual void PreProcessEvt();
  virtual void ProcessEvt();
  virtual void ProcessPair();
  virtual void EventAnalysis () = 0;
  virtual void PairAnalysis () = 0;
  void NewPrePair();
  virtual void InitChanLists ();
  virtual void InitTree ();
  virtual vector<pair<string,string> > ChanList (const string& devtype, 
						 const string& channel, 
						 const string& other);
  virtual void AutoPairAna();

  // Data members
  TaRun* fRun;
  UInt_t fMaxNumEv;
  TaEvent* fPreEvt;
  VaPair* fPrePair;
  // There are three delay queues (actually deques) fEHelDeque is used
  // to implement delayed helicity.  fEDeque and fPDeque are used to
  // implement cut extensions.  Note pairs are queued by pointers
  // (because VaPair is abstract) but events are simply copied
  // (because TaEvent is concrete)
  deque<TaEvent> fEHelDeque;
  deque<TaEvent> fEDeque;
  deque<VaPair*> fPDeque;
  TaEvent* fEvt;
  VaPair* fPair;
  size_t fEHelDequeMax;
  size_t fEDequeMax;
  size_t fPDequeMax;
  vector<pair<string,string> > fCopyList;
  vector<pair<string,string> > fDiffList;
  vector<pair<string,string> > fAsymList;
  TTree* fPairTree;
  Int_t fTreeREvNum; // right ev number for tree
  Int_t fTreeLEvNum; // left ev number for tree
  Double_t fTreeMEvNum; // mean ev number for tree
  Int_t fTreeOKCond; // pair passes cut conditions
  Int_t fTreeOKCut; // pair not in cut interval
  Double_t* fTreeSpace; // other data for tree
  UInt_t fEvtProc;
  UInt_t fPairProc;
  EPairType fPairType;

  // Define LEAKCHECK to check that new = del
#define LEAKCHECK
#ifdef LEAKCHECK
  void LeakCheck();
  static UInt_t fLeakNewEvt;
  static UInt_t fLeakDelEvt;
  static UInt_t fLeakNewPair;
  static UInt_t fLeakDelPair;
#endif

#ifdef DICT
  ClassDef(VaAnalysis, 0);
#endif

};

#endif
