#ifndef PAN__TaDevice
#define PAN__TaDevice

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaDevice.hh   (header file)
//        ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    This class contains that component of "device data" which is
//    the same for each event.  This class is owned by TaRun, not by
//    TaEvent.  That way when events are copied they need not carry
//    the data here.
//
//////////////////////////////////////////////////////////////////////////

#define TADEVICE_VERBOSE 1

#include "Rtypes.h"
#include "DevTypes.hh"
#include <vector>
#include <map>
#include <iterator>
#include <string>

class VaDataBase;

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

    void Init(VaDataBase& db);    // Initalize from database
      
    Int_t GetNumRaw() const;
    Int_t GetRawKey(const Int_t& index) const;
    Int_t GetEvPointer(const Int_t& index) const; 
    Double_t GetPedestal(const Int_t& index) const;
    Double_t GetDacInt(const Int_t& index) const;
    Double_t GetDacSlope(const Int_t& index) const;
    Int_t GetDevNum(const Int_t& index) const;
    Int_t GetChanNum(const Int_t& index) const;
    Int_t GetRawIndex(const Int_t& key) const;
    Int_t GetCalIndex(const Int_t& key) const;
    Bool_t IsUsed(const Int_t& key) const;
    map<string, Int_t> GetKeyList() const;

 protected:

    Int_t fNumRaw;
    Int_t *fRawKeys, *fEvPointer, *fReadOut, *fIsUsed;
    Double_t *fAdcPed, *fScalPed, *fDacInt, *fDacSlope;
    Int_t *fDevNum, *fChanNum;
    map<string, Int_t> fKeyToIdx;
    void InitKeyList();
    Int_t AddRawKey(string keyname);

 private:

    void Create(const TaDevice&);
    void Uncreate();

#ifdef DICT
ClassDef (TaDevice, 0)   // Collection of devices that contain data
#endif

}; 

// For the following "Get" methods, typically used by TaEvent, it is the
// users responsibility to check the index (not checked here, for performance).

inline Int_t TaDevice::GetNumRaw() const { return fNumRaw; };

inline Int_t TaDevice::GetRawKey(const Int_t& index) const  {
  return fRawKeys[index];
};

inline Int_t TaDevice::GetEvPointer(const Int_t& index) const  {
  return fEvPointer[index];
};

inline Double_t TaDevice::GetDacInt(const Int_t& index) const  {
  return fDacInt[index];
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

#endif













