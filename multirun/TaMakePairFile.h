//////////////////////////////////////////////////////////////////////
//
// TaMakePairFile.h
//
//  Class that compiles together many runs into a single rootfile
//
//////////////////////////////////////////////////////////////////////

#ifndef TaMakePairFile_h
#define TaMakePairFile_h

#include <multirun/TaPairSelector.h>
#include <multirun/TaVarChooser.h>
#include <src/TaFileName.hh>
#include <TFile.h>
#include <TTree.h>
#include <vector>
#include <TString.h>

#ifndef MAXDOUBLEDATA
#define MAXDOUBLEDATA 100
#endif

#ifndef MAXINTDATA
#define MAXINTDATA    10
#endif

using namespace std;

class TaMakePairFile {

 public:
  TaMakePairFile(TString rootfilename, TString chooserfilename);
  virtual ~TaMakePairFile() {};
  void RunLoop();
  Bool_t SetRunList(TString listfilename);
  Bool_t SetRunList(vector <pair <UInt_t,UInt_t> > list);
  void Finish();
  void SetDitSlopeFile(TString ditfilename) {fDitFilename = ditfilename;};

 private:
  TFile             *fRootFile;
  TTree             *fTree;
  TaPairSelector    *pairSelect;
  TaPairSelector    *regSelect;
  TaVarChooser      *fChooser;
  TString            fDitFilename;
  TString            fRunFilename;
  vector <TString>   doubleVars;
  vector <TString>   intVars;
  vector <TString>   doubleRegVars;
  vector <TString>   intRegVars;
  Double_t           doubleData[MAXDOUBLEDATA];
  Int_t              intData[MAXINTDATA];
  Double_t           doubleRegData[MAXDOUBLEDATA];
  Int_t              intRegData[MAXINTDATA];
  vector <UInt_t>    ditBPMindex;
  vector <UInt_t>    DETindex;
  vector <pair <UInt_t,UInt_t> > runlist; // first is run, second is slug
  

  // Some default leafs, not in the pan/redana rootfiles.
  Int_t runnumber;
  Int_t slowsign;
  Int_t slug;
  Int_t ok_Both;
  Int_t ok_Left;
  Int_t ok_Right;
  Int_t m_ev_num_off;

  void EventLoop(Long64_t nevents);
  void MakeBranches();
  void PushToDoubleList(vector <TString>,TString,TString);
  void PushToDoubleList(vector <TString> thisvar,
			TString suffix) {
    PushToDoubleList(thisvar,suffix,""); };
  void MakeVarList();
  Bool_t isConfigured();

};

#endif
