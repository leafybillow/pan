#ifndef PAN_TaPanamDevice
#define PAN_TaPanamDevice 
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaPanamDevice.hh  (header)
//
// Author:  A. Vacheret <http://www.jlab.org/~vacheret>
//
//
////////////////////////////////////////////////////////////////////////
//
//     Class for Panam monitor device. Handle easy display 
//     of Pan devices. 
//
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "TObject.h"
#include "TH1.h"
#include <string>
#include <iostream>
#include "PanTypes.hh"
#include "TaStripChart.hh"
#include "TaRun.hh"
#include "TPad.h"

using namespace std;

class TaPanamDevice : public TObject {
  
public:
  
  TaPanamDevice();
  TaPanamDevice(char* name, Int_t namekey, 
                Int_t SNumChan, Int_t SNumEvperChan,
                Int_t histobin, 
                Axis_t xsmin, Axis_t xsmax, 
                Axis_t xhmin, Axis_t xhmax, 
                Int_t color); 
  TaPanamDevice(const TaPanamDevice& copy);
  TaPanamDevice& operator=( const TaPanamDevice& assign);  
  virtual ~TaPanamDevice();
    
  TaStripChart* GetSData() const {return fSData;};
  TaStripChart* GetSDataRMS() const {return fSDataRMS;};
  char* GetName() const {return fName;};
  Double_t  GetDataVal() const {return fDataVal;};
  TH1D*   GetHData() const { return fHData;};
  virtual void InitSCPad(UInt_t plotidx);
  virtual void DisplaySC(UInt_t plotidx);  
  virtual void DrawHPad();  
  virtual void FillFromEvent(TaRun& run);
  TH1D*   GetPlot(char* const plotname, Int_t const plottype) const;

protected:

  void Init();
   //data members 
  vector<TaStripChart*> fSCArray;
  TaStripChart*  fSData;
  TaStripChart*  fSDataRMS;
  TH1D*          fHData;
  char*         fName;
  Double_t      fDataVal;          
  Int_t         fDevicekey;
  Int_t         fSNumOfChan;
  Int_t         fSNumOfEvPerChan;
  Int_t         fHbins;
  Int_t         fColor;
  Float_t       fXSmin;
  Float_t       fXSmax;  
  Float_t       fXHmin;
  Float_t       fXHmax;  
 
     
  ClassDef( TaPanamDevice, 0 )  // Base class 
};

#endif
