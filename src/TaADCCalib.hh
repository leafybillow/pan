////////////////////////////////////////////////////////////////////////
//
// TaBeamAna.hh
//
// Beam Quality Analysis class for pan
//
// Authors: A. Vacheret  Sep 2001 (Round 2);
//          R. Holmes Nov 2001 (Round 3)
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaADCCalib
#define PAN_TaADCCalib

#include <TObject.h>
#include <TTree.h> 
#include <vector>
#include "VaAnalysis.hh"
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"


#define ADC_MaxSlot 10
#define ADC_MaxChan 4

//class TaStatistics;
class TaRun;

class TaADCCalib: public VaAnalysis {
  
public:
  
  // Constructors/destructors/operators
  TaADCCalib();
  TaADCCalib(const string& anName);
  ~TaADCCalib();
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  
  // Major functions
  
  // Data access functions
  
private:
  
  //
  // Data members
  //
  Int_t typeFlag;
  //    array will be set true for each slot, channel  with an existing key
  Bool_t chanExists[ADC_MaxSlot][ADC_MaxChan]; 

  TFile *hfile;    // root histo file
  TH1F **hist;     // Array of histograms

  vector<Double_t>   fSumX;        // sum of X
  vector<Double_t>   fSumX2;       // sum of X^2
  vector<Int_t >     nEntries;     // nEntries for this channel

  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  TaADCCalib(const TaADCCalib& copy);
  TaADCCalib& operator=( const TaADCCalib& assign);
  
  //
  // Member functions
  //
  virtual void TaADCCalib::Init();
  void TaADCCalib::InitPed();
  void TaADCCalib::InitDAC();
  virtual void TaADCCalib::ProcessRun();
  virtual void TaADCCalib::Finish();
  void TaADCCalib::FinishPed();
  void TaADCCalib::FinishDAC();
  void TaADCCalib::EventAnalysis ();
  void TaADCCalib::PairAnalysis ();
  void TaADCCalib::InitChanLists ();
  

#ifdef DICT
  ClassDef(TaADCCalib, 0)
#endif
};

#endif


