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
    Int_t GetKey(string keystr);    // Get key int corresp. to key string

    void Init(const VaDataBase& db);    // Initalize from database
      
    Int_t GetNumRaw() const { return fNumRaw; };
    Int_t GetRawKey(const Int_t& index) const;
    Int_t GetEvPointer(const Int_t& index) const; 
    Double_t GetPedestal(const Int_t& index) const;
    Double_t GetDacInt(const Int_t& index) const;
    Double_t GetDacSlope(const Int_t& index) const;
    Int_t GetAdcNum(const Int_t& index) const;
    Int_t GetChanNum(const Int_t& index) const;
    map<string, Int_t> GetKeyList() const;

 protected:

    Int_t fNumRaw;
    Int_t *fRawKeys, *fEvPointer;
    Double_t *fPedestal, *fDacInt, *fDacSlope;
    Int_t *fAdcNum, *fChanNum;
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

#endif





