//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaFdbkAna.hh  (header file)
//           ^^^^^^^^^^^^
//
//    Authors :  A. Vacheret, R. Holmes, R. Michaels
//
//    Derivated class from VaAnalysis. Charge Asymetry and BPM diffs 
//    feedbacks monitoring/analysis included. This is done if 
//    Database feedback flag is ON and ONLINE running is chosen. Otherwise
//    it is working as VaAnalysis.  
//    
//  Preliminary NOTES :
//
//  !!!! WARNING !!!!
// 
//  1. This class require some precompilation flags to work completly (ONLINE=1)
//  2. check the name of EPICS variables ! 
//  3. Be sure you have feedback flags ON in Database.
// 
//  Qasym feedback frequency is 1/2700 good pairs and 1/2000 at the end. 
//
//////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaFdbkAna
#define PAN_TaFdbkAna

#include <TObject.h>
#include <TTree.h> 
#include "VaAnalysis.hh"


class TaRun;
class VaPair;
class TaCutList;


class TaFdbkAna: public VaAnalysis{

public:

  // Constructors/destructors/operators
  TaFdbkAna();
  ~TaFdbkAna();
  // Copy c'tor and operator= are defined privatly.
  
  //Major functions
  virtual void Init();
  virtual void RunIni(TaRun&);
  virtual void ProcessRun();
  virtual void RunFini();
  virtual void Finish();

  //Data Access functions 

protected:

  TaFdbkAna(const TaFdbkAna& copy);
  TaFdbkAna& operator=( const TaFdbkAna& assign);
  
  // Member functions
  void IsOnlineRun();  // Online EPICS safety function   
  virtual void EventAnalysis ();
  virtual void PairAnalysis ();
  virtual void InitChanLists ();
  virtual void InitTree ();
  void QasymRunFeedback();
  void PZTRunFeedback();
  void QasymEndRunFeedback();
  void PZTEndRunFeedback();
  // Data members : "Q" Charge asym parameters, "Z" PZT parameters
  
  Int_t fRunNum;
  Int_t fQStartPair,fQStopPair, fNQpair; 
  Int_t fQlevFdbk, fQnum, fQsent;
  Int_t fZStartpair,fZStopPair, fNZpair;
  Int_t fZlevFdbk, fZnum, fZsent; 
  vector<Double_t> fQasym, fZdiff4AX, fZdiff4AY,fZdiff4BX,fZdiff4BY;  
  Double_t fQasymMean1, fQasymMean2, fQasymRMS, fQasymErr, fQmevFdbk, fZmevFdbk ; 
  Double_t fZdiff4AXMean1, fZdiff4AXMean2, fZdiff4AXRMS, fZdiff4AXErr; 
  Double_t fZdiff4AYMean1, fZdiff4AYMean2, fZdiff4AYRMS, fZdiff4AYErr; 
  Double_t fZdiff4BXMean1, fZdiff4BXMean2, fZdiff4BXRMS, fZdiff4BXErr; 
  Double_t fZdiff4BYMean1, fZdiff4BYMean2, fZdiff4BYRMS, fZdiff4BYErr; //
  TTree* fFdbkTree;  // feedback tree where all run's feedback info are stored.

#ifdef DICT
  ClassDef(TaFdbkAna, 0)
#endif
};

#endif







