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
#include <vector>
#include "VaAnalysis.hh"
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TGraphErrors.h"


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


  // data members for pedestal analysis
  TH1F **phist;     // Array of histograms
  vector<Double_t>   fSumX;        // sum of X
  vector<Double_t>   fSumX2;       // sum of X^2
  vector<Int_t >     nEntries;     // nEntries for this channel

  // data members  for DAC analysis
  vector<Double_t>   fSumY;        // additional sums needed for linear fit
  vector<Double_t>   fSumXY;
  //
  //   additional stuff needed for graph of residuals
  //
  static const Int_t MaxNoiseDACBin = 65536;
  TGraphErrors **dgraphs;   // array of graphs
  TGraphErrors **rgraphs;   // array of graphs
  vector< vector<Int_t> >  dEntries;   // number of entries in each chan#/DAC bin
  vector< vector<Double_t> >  dADCsum;  // sum of ADC value for each chan#/DAC bin
  vector< vector<Double_t> >  dADCsum2; // sum of ADC value-squared


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
  ClassDef(TaADCCalib, 0)  // ADC calibration analysis
#endif
};

#endif


