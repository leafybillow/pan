#ifndef PAN__TaDevice
#define PAN__TaDevice

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaDevice.hh   (header file)
//        ^^^^^^^^^^^
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
//////////////////////////////////////////////////////////////////////////
//
//    This class contains that component of "device data" which is
//    the same for each event.  This class is owned by TaRun, not by
//    VaEvent.  That way when events are copied they need not carry
//    the data here.
//
//////////////////////////////////////////////////////////////////////////

#define TADEVICE_VERBOSE 1

#include "Rtypes.h"
#include "DevTypes.hh"
#include "TaDataBase.hh"
#include "VaEvent.hh"
#include <vector>
#include <map>
#include <iterator>
#include <string>

using namespace std;

class TaDataBase;

class TaDevice {

 public:

    TaDevice(); 
    virtual ~TaDevice();

    TaDevice(const TaDevice& copy);
    TaDevice& operator=( const TaDevice& assign);

// Use the following to find integer keys corresponding to string key names.
// But DO NOT use this inside an event loop.  Instead, build a list of integers
    Int_t GetKey(string keystr) const;    // Get key int corresp. to key string
    string GetKey(Int_t key) const;       // inverse map 

    void Init(TaDataBase& db);    // Initalize from database
      
    Int_t GetNumRaw() const;
    Int_t GetRawKey(const Int_t& index) const;
    Int_t GetEvPointer(const Int_t& index) const; 
    Int_t GetCrate(const Int_t& index) const;
    Int_t GetOffset(const Int_t& key) const;
    void FindHeaders(const Int_t& roc, const Int_t& ipt, const Int_t& data);
    void PrintHeaders();
    Double_t GetPedestal(const Int_t& index) const;
    Double_t GetDacSlope(const Int_t& index) const;
    Int_t GetDevNum(const Int_t& index) const;
    Int_t GetChanNum(const Int_t& index) const;
    Int_t GetRawIndex(const Int_t& key) const;
    Int_t GetCalIndex(const Int_t& key) const;
    Int_t GetCorrIndex(const Int_t& key) const;
    Bool_t IsUsed(const Int_t& key) const;
    Bool_t IsRotated(const Int_t& key) const;
    Bool_t IsAdc(const Int_t& key) const;
    Bool_t IsScaler(const Int_t& key) const;
    Bool_t IsTimeboard(const Int_t& key) const;
    Bool_t IsTir(const Int_t& key) const;
    Bool_t IsDaqFlag(const Int_t& key) const;
    void SetUsed(const Int_t& key) const;
    map<string, Int_t> GetKeyList() const;

 protected:

    Int_t fNumRaw, fNtied;
    UInt_t fgAdcHeader, fgScalHeader, fgTbdHeader;
    UInt_t fgTirHeader, fgDaqHeader;
    UInt_t fgAdcMask, fgScalMask, fgTbdMask;
    UInt_t fgTirMask,  fgDaqMask;       
    Int_t *fRawKeys, *fEvPointer, *fCrate;
    Int_t *fReadOut, *fIsUsed, *fIsRotated;
    Double_t *fAdcPed, *fScalPed, *fDacSlope;
    Int_t *fDevNum, *fChanNum;
    Int_t *fAdcptr, *fScalptr;   
    Int_t *fTbdptr, *fTirptr;    
    Int_t *fDaqptr;
    map<string, Int_t> fKeyToIdx;
    vector<TaKeyMap> fTiedKeys;
    vector<string> fRotateList;
    void InitKeyList();
    Int_t AddRawKey(string keyname);
    void AddTiedDevices(TaKeyMap& keymap);
    void BpmDefRotate();

 private:

    void Create(const TaDevice&);
    void Uncreate();

#ifndef NODICT
ClassDef (TaDevice, 0)   // Collection of devices that contain data
#endif

}; 

// For the following "Get" methods, typically used by VaEvent, it is the
// users responsibility to check the index (not checked here, for performance).

inline Int_t TaDevice::GetNumRaw() const { return fNumRaw; };

inline Int_t TaDevice::GetRawKey(const Int_t& index) const  {
  return fRawKeys[index];
};

inline Int_t TaDevice::GetEvPointer(const Int_t& index) const  {
  return fEvPointer[index];
};

inline Int_t TaDevice::GetCrate(const Int_t& index) const  {
  return fCrate[index];
};

inline Int_t TaDevice::GetOffset(const Int_t& key) const {
  Int_t crate = GetCrate(key);
  if (crate <= 0 || crate > MAXROC) return 0;
  if (IsAdc(key)) return fAdcptr[crate];
  if (IsScaler(key)) return fScalptr[crate];
  if (IsTimeboard(key)) return fTbdptr[crate];
  if (IsTir(key)) return fTirptr[crate];
  if (IsDaqFlag(key)) return fDaqptr[crate];
  return 0;
};

inline Double_t TaDevice::GetDacSlope(const Int_t& index) const  {
  return fDacSlope[index];
};

inline Int_t TaDevice::GetDevNum(const Int_t& index) const  {
  return fDevNum[index];
};

inline Int_t TaDevice::GetChanNum(const Int_t& index) const  {
  return fChanNum[index];
};

inline Bool_t TaDevice::IsUsed(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fIsUsed[key] == 1) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsRotated(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fIsRotated[key] == 1) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsAdc(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fReadOut[key] == ADCREADOUT) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsScaler(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fReadOut[key] == SCALREADOUT) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsTimeboard(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fReadOut[key] == TBDREADOUT) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsTir(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fReadOut[key] == TIRREADOUT) return kTRUE;
  }
  return kFALSE;
};

inline Bool_t TaDevice::IsDaqFlag(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) {
    if (fReadOut[key] == DAQFLAG) return kTRUE;
  }
  return kFALSE;
};

inline void TaDevice::SetUsed(const Int_t& key) const {
  if (key >= 0 && key < MAXKEYS) fIsUsed[key] = 1;
  return;
};

#endif




