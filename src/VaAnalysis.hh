//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaAnalysis.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Abstract base class of analysis. Derived classes include TaADCCalib
// (for computation of pedestals and DAC noise slopes) and TaBeamAna
// (for analysis of beam characteristics).  Future derived classes may
// include TaAsymAna (for analysis of physics asymmetries),
// TaModulaAna (for computation of beam modulation coefficients), and
// TaCorrecAna (for computation of corrections due to
// helicity-correlated beam differences).  Each of these is
// responsible for some treatment of TaEvents from a TaRun.  The type
// of analysis to be done is specified in the database, and the
// TaAnalysisManager instantiates the appropriate analysis class
// accordingly.
//
// VaAnalysis has initialization and termination routines for both the
// overall analysis and the analysis of a particular run.  At present
// Pan is designed to analyze only a single run, but these routines
// provide for a possible future version that will handle multiple
// runs.
//
// The main event loop is inside the ProcessRun method.  The three
// main methods called from here are PreProcessEvt, ProcessEvt, and
// ProcessPair.  The first of these places the most recently read
// event into a delay queue until the delayed helicity information for
// that event becomes available.  Cut conditions are checked for here.
// Once the helicity information is added the event is pushed onto a
// second delay queue, while the events are used to construct pairs
// which are pushed onto a third delay queue.  These two delay queues
// are used to hold events and pairs until we can tell whether they
// fall within a cut interval caused by a cut condition arising later.
// Events and pairs which emerge from the ends of these queues are
// analyzed in ProcessEvt and ProcessPair, respectively.  Analysis
// results are added to the events and pairs themselves.
//
////////////////////////////////////////////////////////////////////////

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
class AnaList {
// Utility class of variable info
public: 
  AnaList(string svar, Int_t ivar, string sun, UInt_t iflag) :
        fVarStr(svar), fVarInt(ivar), fUniStr(sun), fFlagInt(iflag) { }
  string fVarStr; 
  Int_t fVarInt;
  string fUniStr;
  UInt_t fFlagInt;
};

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
  virtual void Init(const Bool_t&);
  virtual ErrCode_t RunIni(TaRun&);
  virtual ErrCode_t RunReIni(TaRun&);
  virtual ErrCode_t ProcessRun();
  virtual void RunFini();
  virtual void Finish();

  // Data access functions
  size_t PairsLeft() const { return fPDeque.size(); }

  // Constants
  static const UInt_t fgNO_STATS;
  static const UInt_t fgNO_BEAM_NO_ASY;
  static const UInt_t fgCOPY;
  static const UInt_t fgDIFF;
  static const UInt_t fgASY;
  static const ErrCode_t fgVAANA_ERROR;  // returned on error
  static const ErrCode_t fgVAANA_OK;      // returned on success
  static const UInt_t fgNumBpmFdbk;

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

  virtual ErrCode_t PreProcessEvt();
  virtual ErrCode_t ProcessEvt();
  virtual ErrCode_t ProcessPair();
  virtual void EventAnalysis () = 0;
  virtual void PairAnalysis () = 0;
  ErrCode_t NewPrePair();
  virtual void InitChanLists ();
  virtual void InitTree ();
  virtual vector<AnaList> ChanList (const string& devtype, 
				      const string& channel, 
				      const string& other,
				      const UInt_t flags = 0);
  virtual vector<AnaList> ChanListFromName (const string& chanbase, 
					    const string& other, 
					    const UInt_t flags = 0);
  virtual void AutoPairAna();

  virtual void QasyRunFeedback();
  virtual void QasyEndFeedback();
  virtual void PZTRunFeedback();
  virtual void PZTEndFeedback();
  virtual void SendVoltagePC();
  virtual void QasySendEPICS();
  virtual void PZTSendEPICS();
  virtual void SendVoltagePZT();
  virtual void GetLastSetPt();

  // Data members
  TaRun* fRun;                  // Run being analyzed
  UInt_t fMaxNumEv;             // Max number of events to analyze
  TaEvent* fPreEvt;             // Event being preprocessed
  VaPair* fPrePair;             // Pair being built
  // There are three delay queues (actually deques) fEHelDeque is used
  // to implement delayed helicity.  fEDeque and fPDeque are used to
  // implement cut extensions.  Note pairs are queued by pointers
  // (because VaPair is abstract) but events are simply copied
  // (because TaEvent is concrete)
  deque<TaEvent> fEHelDeque;    // Helicity delay event deque
  deque<TaEvent> fEDeque;       // Cut delay event deque
  deque<VaPair*> fPDeque;       // Cut delay pair deque
  TaEvent* fEvt;                // Event being analyzed
  VaPair* fPair;                // Pair being analyzed
  size_t fEHelDequeMax;         // Max size of helicity delay event deque
  size_t fEDequeMax;            // Max size of cut delay event deque
  size_t fPDequeMax;            // Max size of cut delay pair deque
  vector<AnaList> fTreeList;    // Quantities to put in the pair results and pair tree
  TTree* fPairTree;             // Pair tree for Root file
  Int_t fTreeREvNum;            // Right ev number for tree
  Int_t fTreeLEvNum;            // Left ev number for tree
  Double_t fTreeMEvNum;         // Mean ev number for tree
  Int_t fTreeOKCond;            // Pair passes cut conditions
  Int_t fTreeOKCut;             // Pair not in cut interval
  Double_t* fTreeSpace;         // Other data for tree
  Bool_t fOnlFlag;              // Flag whether data are online or not. 
  UInt_t fEvtProc;              // Number of events processed
  UInt_t fPairProc;             // Number of pairs processed
  EPairType fPairType;          // Type of beam helicity structure
  Bool_t fFirstPass;            // Pass 1 or 2?

  Bool_t fQSwitch;              // value of the switch for IA feedback
  Bool_t fZSwitch;              // value of the switch for PZT feedbac
  Int_t fRunNum;                // current run number 
  UInt_t fQTimeScale;           // timescale of IA feedback 
  UInt_t fZTimeScale;           // timescale of PZT feedback
  UInt_t fZNpair;               // PZT feedback number of pair  
  UInt_t fQNpair;               // IA feedback number of pair 
  Int_t fQStartPair;            // IA feedback pair start 
  Int_t fQStopPair;             // IA feedback pair stop
  Int_t fQfeedNum;              // IA feedback number
  Int_t fZpair[2];              // PZT feedback number of pair Dx,Dy
  Int_t fZStartPair;            // PZT feedback pair start
  Int_t fZStopPair;             // PZT feedback pair stop
  Int_t fZfeedNum;              // PZT feedback number
  vector<Double_t> fQsum;       // IA charge asymmetry sum 
  vector<Double_t> fZsum4B[2];  // PZT Dx,Dy sum
  Double_t fQmean1;             // IA charge asymmetry mean value 1st pass  
  Double_t fQmean2;             // IA charge asymmetry mean value 2nd pass
  Double_t fQRMS;               // IA charge asymmetry RMS 
  Double_t fQasy;               // IA charge asymmetry 
  Double_t fQasyEr;             // IA charge asymmetry error
  Double_t fZ4Bmean1[2];        // PZT Dx.Dy mean value 1st pass
  Double_t fZ4Bmean2[2];        // PZT Dx,Dy mean value 2nd pass 
  Double_t fZ4BRMS[2];          // PZT Dx,Dy RMS
  Double_t fZ4Bdiff[2];         // PZT BPM4B Dx,Dy  
  Double_t fZ4BdiffEr[2];       // PZT BPM4B Dx,Dy errors
  Double_t fIAslope;            // IA feedback claibration slope
  Double_t *fPZTMatrix;         // PZT matrix
  Double_t fIAlast;             // last value of the IA voltage 
  Double_t fPZTXlast;            // last value of the X PZT voltage
  Double_t fPZTYlast;           // last valure of the Y PZT voltage

  // Define LEAKCHECK to check that new = del
#define LEAKCHECK
#ifdef LEAKCHECK
  void LeakCheck();
  static UInt_t fLeakNewEvt;    // count of event allocations
  static UInt_t fLeakDelEvt;    // count of event deallocations
  static UInt_t fLeakNewPair;   // count of pair allocations
  static UInt_t fLeakDelPair;   // count of pair deallocations
#endif

#ifdef DICT
  ClassDef(VaAnalysis, 0)  // Interface class for data analysis
#endif

};

#endif





