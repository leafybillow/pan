#ifndef PAN_TaPanamADevice
#define PAN_TaPanamADevice 
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaPanamADevice.hh  (header)
//
// Author:  A. Vacheret <http://www.jlab.org/~vacheret>
//
//
////////////////////////////////////////////////////////////////////////
//
//     Class for Panam monitor device. Handle easy display 
//     of Pan devices which requires asymmetry plots. 
//
//
////////////////////////////////////////////////////////////////////////

#include "TaStripChart.hh"
#include "TaPanamDevice.hh"
#include "VaPair.hh"


class TaPanamADevice : public TaPanamDevice {
  
public:
  
  TaPanamADevice();
  TaPanamADevice(char* name, Int_t namekey, 
                 Int_t SNumChan, Int_t SNumEvperChan,
                 Int_t SANumChan, Int_t SANumEvperChan,
                 Int_t histobin,
                 Axis_t xsmin, Axis_t xsmax,
                 Axis_t xhmin, Axis_t xhmax, 
                 Int_t histoabin,  
                 Axis_t xsamin, Axis_t xsamax,
                 Axis_t xhamin, Axis_t xhamax, 
                 Int_t color, Int_t acolor, Bool_t datatype); 
  TaPanamADevice(const TaPanamADevice& copy);
  TaPanamADevice& operator=( const TaPanamADevice& assign);  
  virtual ~TaPanamADevice();
    
  Double_t  GetADataVal() const {return fADataVal;};
  TaStripChart* GetSAData() const {return fSAData;};
  TaStripChart* GetSADataRMS() const {return fSADataRMS;};
  TH1D*   GetHAData() const { return fHAData;};
  //void InitSCPad(UInt_t plotidx);
  //  void DisplaySC(UInt_t plotidx);  
  //  void DisplayH(UInt_t plotidx);  
  void FillFromPair(VaPair& pair);
  void DrawHPad(UInt_t plotidx);
  //  TH1D*   GetPlot(char* const plotname, Int_t const plottype) const;

protected:

  void Init();
   //data members 
  vector<TH1D*>          fHArray;
  TaStripChart*  fSAData;
  TaStripChart*  fSADataRMS;
  TH1D*          fHAData;
  Double_t       fADataVal;          
  Int_t         fSANumOfChan;
  Int_t         fSANumOfEvPerChan;
  Int_t         fHAbins;
  Int_t         fAColor;
  Float_t       fXSAmin;
  Float_t       fXSAmax;  
  Float_t       fXHAmin;
  Float_t       fXHAmax;  
  Bool_t        fWhichData;
     
  ClassDef( TaPanamADevice, 0 )  // Base class 
};

#endif
