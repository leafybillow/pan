//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaScaler.cc  (implementation)
//           ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Scaler data.  
//
//////////////////////////////////////////////////////////////////////////


#include "TaScaler.hh" 
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaScaler)
#endif

TaScaler::TaScaler(string name) : VaDevice(name) {
}

TaScaler::~TaScaler() {
}

void TaScaler::Init(const VaDataBase& db) { 
  VaDevice::Init(db);
  fType  = "scaler";
  header = db.GetHeader(fType);
  mask   = db.GetMask(fType);
};

TaScaler::TaScaler(const TaScaler& rhs) : VaDevice(rhs) {
  keys = rhs.keys;
  fData = rhs.fData;
};

TaScaler &TaScaler::operator=(const TaScaler &rhs) {
 if (this != &rhs) {
    VaDevice::operator=(rhs);
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
};
    
void TaScaler::Decode(const TaEvent& event) {
  VaDevice::Decode(event);
};











