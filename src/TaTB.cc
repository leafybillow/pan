//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaTB.cc   (implementation)
//           ^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Timing board data.  
//
//////////////////////////////////////////////////////////////////////////


#include "TaTB.hh" 
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaTB)
#endif

TaTB::TaTB(string name) : VaDevice(name) {
}

TaTB::~TaTB() {
}

void TaTB::Init(const VaDataBase& db) { 
  VaDevice::Init(db);
  fType  = "timeboard";
  header = db.GetHeader(fType);
  mask   = db.GetMask(fType);
}

TaTB::TaTB(const TaTB& rhs) : VaDevice(rhs) {
  keys = rhs.keys;
  fData = rhs.fData;
}

TaTB &TaTB::operator=(const TaTB &rhs) {
 if (this != &rhs) {
    VaDevice::operator=(rhs);
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
}
    
void TaTB::Decode(const TaEvent& event) {
  VaDevice::Decode(event); 
}






