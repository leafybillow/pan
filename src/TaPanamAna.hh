//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPanamAna.hh  (interface)
//
// Author: A. Vacheret <http://www.jlab.org/~vacheret>
//
////////////////////////////////////////////////////////////////////////
//
//    Online analysis.  This class derives from VaAnalysis.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaPanamAna
#define PAN_TaPanamAna

#include <TObject.h>
#include <TTree.h>
#include <THStack.h>
#include <TGraph.h>
#include <TH2.h>
#include "TaString.hh"
#include "VaAnalysis.hh"
#include "TaPanamDevice.hh"
#include "TaPanamADevice.hh"

//#include "TaPanamMultiDevice.hh"
//#include <vector>

using std::vector;

class TaRun;

class TaPanamAna: public VaAnalysis {
  
public:

  enum NumOfChan { striptime = 180, numlumi = 4, numdalton = 4};
  
  // Constructors/destructors/operators
  TaPanamAna();
  virtual ~TaPanamAna();
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  
  // Major functions
  vector<string> GetHistoForListBox() const; 
  void InitDevicesList(vector<string> arrayofname);
  void InitMonitorDevices();
  //  void InitMonitorMultiDevices();
  void InitDevicePad(Int_t devidx,UInt_t plotidx, Bool_t choice);
  void DisplayDevice(Int_t devidx,UInt_t plotidx, Bool_t choice);
  void InitADevicePad(Int_t devidx,UInt_t plotidx, Bool_t choice);
  void DisplayADevice(Int_t devidx,UInt_t plotidx, Bool_t choice);
  void DefineADCStacks( Bool_t opt);
  virtual void InitADCStack(Int_t idx);   
  virtual void DrawADCStack(Int_t idx);   
  virtual void InitLumiGraph(Int_t idx);
  virtual void DrawLumiGraph(Int_t idx);
  virtual void InitDaltonGraph(Int_t idx);
  virtual void DrawDaltonGraph(Int_t idx);
  TaPanamDevice* GetPanamDevice(Char_t* const devname) const;
  TaPanamADevice* GetPanamADevice(Char_t* const adevname) const;
  //  TaPanamDevice* GetPanamMultiDevice(Char_t* const devname) const;
  // Data access functions
  
private:
  
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  TaPanamAna(const TaPanamAna& copy);
  TaPanamAna& operator=( const TaPanamAna& assign);
  
  // Member functions
  void EventAnalysis ();
  void PairAnalysis ();
  void InitChanLists ();
  string itos(Int_t i);
  void FillEventPlots();
  void FillPairPlots();
  
  // Data members
  vector<string>               fArrayOfDataName;
  vector<TaPanamDevice*>       fMonDev;
  vector<TaPanamADevice*>      fMonADev;
  vector<TaPanamADevice*>      fLumiDev;
  vector<TaPanamADevice*>      fDaltonDev;
  vector<THStack*>             fSTADC; 
  vector<THStack*>             fSTADC_RMS; 
  vector<TGraph*>              fLumiGraph;
  vector<TGraph*>              fDaltonGraph;
  TGraph                       *fLumiADC,*fLumiADCRMS; 
  TGraph                       *fLumiAsy,*fLumiAsyRMS;
  //  TGraph                     *fLumiADCBCMDiff,*fLumiADCBCMDiffRMS; 
  TGraph                       *fLumiADCBCMDiffAsy,*fLumiADCBCMDiffAsyRMS;

  TGraph    *fDaltonADC,*fDaltonADCRMS; 
  TGraph    *fDaltonAsy,*fDaltonAsyRMS;
  TGraph    *fDaltonBCMNormADC,*fDaltonBCMNormADCRMS; 
  TGraph    *fDaltonBCMNormAsy,*fDaltonBCMNormAsyRMS;
  TH2D      *fLimits;
  
  Int_t fADC_color[4]; 
  Int_t fTime;
  Int_t fStrip_timeslot;                         // 
  Int_t fADC_count;
  Int_t fBCM_count;
  Int_t fBPM_count;

  char* dlabel;
  Char_t* fLastADCName;

  Float_t LumiChan[numlumi];
  Float_t LumiADC[numlumi];
  Float_t LumiADCRMS[numlumi];
  Float_t LumiAsy[numlumi];
  Float_t LumiAsyRMS[numlumi];
  Float_t LumiADCBCMDiffAsy[numlumi];
  Float_t LumiADCBCMDiffAsyRMS[numlumi];
  Float_t Daltons[numdalton];
  Float_t DaltonsADC[numdalton];
  Float_t DaltonsADCRMS[numdalton];
  Float_t DaltonsBCMNormADC[numdalton];
  Float_t DaltonsBCMNormADCRMS[numdalton];
  Float_t DaltonsAsy[numdalton];
  Float_t DaltonsAsyRMS[numdalton];
  Float_t DaltonsBCMNormAsy[numdalton];
  Float_t DaltonsBCMNormAsyRMS[numdalton];
 
  Bool_t fIsLumi,fIsDalton,fIsADC;
  Bool_t fADCStackSCorH;
  //#ifdef DICT
  ClassDef(TaPanamAna, 0)  // Monitoring analysis
    //#endif
};

#endif
