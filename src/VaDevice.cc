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


#include "VaDevice.hh" 
#include "VaDataBase.hh"
#include "TaEvent.hh"
#include "TTree.h"

#ifdef DICT
ClassImp(VaDevice)
#endif

VaDevice::VaDevice(string name) : fName(name) { 
   inited    = kFALSE;
   ndata     = 0;
};

VaDevice::~VaDevice() { 
   delete keymap;
   delete [] fDataCopy;
}

VaDevice::VaDevice(const VaDevice& rhs) {
   fName = rhs.fName;
   fType = rhs.fType;
   header = rhs.header;
   mask = rhs.mask;
   fData = rhs.fData;
   evbuffer_pointer = rhs.evbuffer_pointer;
   ndata = rhs.ndata;
   fDataCopy = new Double_t[ndata];
   memcpy(fDataCopy, rhs.fDataCopy, ndata*sizeof(Double_t));
   keymap = new TaKeyMap(*(rhs.keymap));
   keys = rhs.keys;
}

VaDevice &VaDevice::operator=(const VaDevice &rhs) {
 if (this != &rhs) {
    fName = rhs.fName;
    fType = rhs.fType;
    header = rhs.header;
    mask = rhs.mask;
    fData = rhs.fData;
    evbuffer_pointer = rhs.evbuffer_pointer;
    ndata = rhs.ndata;
    delete [] fDataCopy;
    fDataCopy = new Double_t[ndata];
    memcpy(fDataCopy, rhs.fDataCopy, ndata*sizeof(Double_t));
    delete keymap;
    keymap = new TaKeyMap(*(rhs.keymap));
    keys = rhs.keys;
 }
 return *this;
}

Double_t VaDevice::GetData( const string &key ) const { 
// To find a value corresponding to a data key 
// Key points to a subunit of the device
    if (fData.find(key) != fData.end()) return fData[key];
    if (VADEVICE_VERBOSE) 
       cout << "VaDevice:: WARNING: no data in "<<fName<<" for key "<<key<<endl;
    return 0; 
}; 

Double_t VaDevice::GetData( const Int_t& chan ) const {
// Indexed channels, assume of form "fName_N" where N = channel
   static char c1[40],c2[40];
   strcpy (c1, fName.c_str());
   sprintf (c2, "_%d", chan);
   strcat (c1, c2);
   string key = c1;
   if (fData.find(key) != fData.end()) return fData[key];
   if (VADEVICE_VERBOSE) 
   cout << "VaDevice:: WARNING: no data  for key "<<key<<endl;
   return 0; 
}; 

void VaDevice::Decode(const TaEvent& event) {
    fData.clear();
    if ( !inited ) {
      cout << "VaDevice:: ERROR:: Device "<<fName<<" not initalized"<<endl;
      cout << "You must initalize the device before using Decode()"<<endl;
      return;
    }
    pair< string, Double_t> data;
    for (vector<string>::iterator ikey = keys.begin(); 
      ikey != keys.end(); ikey++) {
        string key = *ikey;
        data.first = key;
        if ( IsRaw(key) ) {
          data.second = (Double_t)event.GetRawData(evbuffer_pointer[key]);
          fData.insert(data);
        }
    }
    DataCopy();
#ifdef DEBUG    
    DevInfo();
#endif
};

Bool_t VaDevice::IsRaw(const string& key) const {
// To answer if this channel of data is raw or cooked
   return (evbuffer_pointer[key] > 0);
};

Bool_t VaDevice::IsADC() const {
   return kFALSE;
};

Bool_t VaDevice::Defined(const string& akey) const {
  for (vector<string>::iterator ikey = keys.begin(); 
     ikey != keys.end(); ikey++) {
       string key = *ikey;
       if (key == akey) return kTRUE;
  }   
  return kFALSE;
};

Int_t VaDevice::GetADCNumber() const {
   return -1;
};

Int_t VaDevice::GetChannel(string key) const {
   return -1;
};

void VaDevice::DataCopy() {
    Int_t index = 0;
    for (vector<string>::iterator ikey = keys.begin(); 
      ikey != keys.end(); ikey++) {
       string key = *ikey;
       if (index < ndata) fDataCopy[index++] = fData[key];
    }
};

vector<string> VaDevice::GetKeys() {
    return keys;
};

void VaDevice::Init(const VaDataBase& db) {
    keymap  = new TaKeyMap();
    *keymap = db.GetKeyMap(fName);
    keys    = keymap->GetKeys();
    fType   = db.GetDataMapType();
    header  = db.GetHeader(fType);
    mask    = db.GetMask(fType);
    FindHeaders();
    inited  = kTRUE;
};

void VaDevice::FindHeaders() {
// Find the pointers to the event structure for this device.
    for (vector<string>::iterator ikey = keys.begin(); 
               ikey != keys.end(); ikey++) {
       string key = *ikey;
       Int_t offset = keymap->GetEvOffset(key);
       if (offset < 0) continue;
       evbuffer_pointer.insert(make_pair(key, offset));
    }
};

void VaDevice::AddToTree( TTree& rawtree ) {
// Add the raw data of this device to the raw data tree (root output)
// Called by TaRun::InitDevices()
    Int_t bufsize = 5000;
    char tinfo[20];
    ndata = keys.size();
    if (ndata < 20) ndata = 20;
    fDataCopy = new Double_t[ndata];
    Int_t index = 0;
    for (vector<string>::iterator ikey = keys.begin(); 
      ikey != keys.end(); ikey++) {
        string key = *ikey;
        strcpy(tinfo,key.c_str());  strcat(tinfo,"/D");
	rawtree.Branch(key.c_str(), &fDataCopy[index++], tinfo, bufsize);
    }
};

void VaDevice::DevInfo(){
  cout<<" ------------------------ structure of device "<<fName<<" --------------"<<endl;
  cout<<" \n Type : "<<fType<<endl;
  cout<<" Mask : "<<mask<<endl;
  cout<<"       DATA container    "<<endl;
  map<string,Double_t>::const_iterator diter;
  for (diter = fData.begin() ; diter != fData.end() ; diter++ ) {
      cout<< " key "<<diter->first<<" data "<<diter->second<<endl;
   }  
  cout<<"     Event buffer pointers    "<<endl;
  map<string,Int_t>::const_iterator eiter;
  for (eiter = evbuffer_pointer.begin() ; eiter != evbuffer_pointer.end() ; eiter++ ) {
      cout<< " key "<<eiter->first<<" evboff index "<<eiter->second<<endl;
   }  
  cout<<"  copy of datamap keymap  "<<endl;
  keymap->Print(); 
  cout<< " \n _____________________________________________________________________"<<endl; 
};













