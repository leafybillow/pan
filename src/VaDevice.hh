#ifndef PAN__VaDevice
#define PAN__VaDevice

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        VaDevice.hh   (header file)
//        ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Base class of device objects like BPMs, BCMs, TIR, scalers, etc ...    
//
//    The public interface :
//
//    1. Decode(const TaEvent& event)  -- decode the event, must be called
//                                        in the event loop by TaRun.
//    2. GetData(const string& key) -- to get the data.  They keys are the
//            same as used by database.  They are mnemonic names we use to refer
//            to channels of *RAW* data, like "bcm1raw", "bpm8xp", "helicity", etc.
//            Derived classes like TaBPM add their own keys of cooked data, 
//            like "bpm8x" (x position), etc.
//    3. GetKeys() is used by TaEvent to figure out what keys are available.
//    4. Init(...) Initalize from a database (called by TaRun)
//    5. AddToTree(....) Add data to the raw data tree (called by TaRun)
//
//////////////////////////////////////////////////////////////////////////

#define VADEVICE_VERBOSE 1
#include "Rtypes.h"
#include <map>
#include <vector>
#include <iterator>
#include <string>

class VaDataBase;
class TaKeyMap;
class TaEvent;
class TTree;

class VaDevice {

 public:

    VaDevice(string name); 
    virtual ~VaDevice();
    VaDevice(const VaDevice& copy);
    VaDevice& operator=( const VaDevice& assign);

    string GetName() { return fName; };
    string GetType() { return fType; };
    virtual void Decode(const TaEvent& event);  // Decode an event
    Double_t GetData( const string &key) const; // To get data for key 
    virtual Double_t GetData( const Int_t& chan ) const;  // for indexed keys (e.g. scalers)
    virtual vector<string> GetKeys();           // Retrieve keys of device
    virtual void Init(const VaDataBase& db);    // Initalize from database
    virtual void AddToTree(TTree &tree);        // Add data to root Tree
    void DevInfo();                             // debug printout of data
    virtual Bool_t IsADC() const;               // true if the device is an ADC           
    virtual Int_t GetADCNumber() const;         // ADC index
    virtual Int_t GetChannel(string key) const; // ADC channel
    Bool_t IsRaw(const string& key) const;      // true if the key pertains to raw data
    Bool_t Defined(const string& key) const;    // true if this key was defined.
      
 protected:

    string fName,fType;
    Bool_t inited;
    Int_t header, mask, sizeconst;
    map< string , Double_t > fData;
    map< string, Int_t > evbuffer_pointer;
    Int_t ndata;
    Double_t *fDataCopy;
    TaKeyMap *keymap;
    vector<string> keys;
    virtual void DataCopy();
    virtual void FindHeaders();

 private:

#ifdef DICT
ClassDef (VaDevice, 0)   // Device containing data
#endif

};

#endif





