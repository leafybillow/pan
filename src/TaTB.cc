//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaTB.cc   (implementation)
//           ^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels, K.Paschke
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
    
void TaTB::Init(const VaDataBase& db) { 
  VaDevice::Init(db);
  fType  = "timeboard";
  keys.push_back("oversample_bin");
  header = db.GetHeader(fType);
  mask   = db.GetMask(fType);
}

void TaTB::Decode(const TaEvent& event) {
  VaDevice::Decode(event); 
  ExtractSignal(event); 
  VaDevice::DataCopy();
}


void TaTB::ExtractSignal( const TaEvent& event) {
  if (! inited) {
    cout<<"TaTB:: ERROR: calibration not set up. Must Init(db)"<<endl;
    fData.clear();
    return;
  }
  fData["oversample_bin"]  = ((Int_t)GetData("oversample") & 0xFF00) >> 8;   // time slot in os cycle  
};





