//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaTIR.cc  (implementation)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Trigger Interrupt Register data.  Contains bits from an
//    I/O register, such as 'helicity', 'realtime', etc.
//
//////////////////////////////////////////////////////////////////////////


#include "TaTIR.hh" 
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaTIR)
#endif

TaTIR::TaTIR(string name) : VaDevice(name) {
}

TaTIR::~TaTIR() {
}


TaTIR::TaTIR(const TaTIR& rhs) : VaDevice(rhs) {
  keys = rhs.keys;
  fData = rhs.fData;
}

TaTIR &TaTIR::operator=(const TaTIR &rhs) {
 if (this != &rhs) {
    VaDevice::operator=(rhs);
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
}
 
void TaTIR::Init(const VaDataBase& db) { 
  VaDevice::Init(db);
  fType  = "tir";
  keys.push_back("helicity");  
  keys.push_back("pairsynch");   
  keys.push_back("timeslot");  
  header = db.GetHeader(fType);
  mask   = db.GetMask(fType);
}
   
void TaTIR::Decode(const TaEvent& event) {
  VaDevice::Decode(event);
  ExtractSignal(event); 
  VaDevice::DataCopy();
};

void TaTIR::ExtractSignal( const TaEvent& event) {
  if (! inited) {
    cout<<"TaTIR:: ERROR: calibration not set up. Must Init(db)"<<endl;
    fData.clear();
    return;
  }
  fData["timeslot"]  = ((Int_t)GetData("tirdata") & 0x20) >> 5;   // time slot  
  fData["helicity"]  = ((Int_t)GetData("tirdata") & 0x40) >> 6;   // helicity bit 
  fData["pairsynch"] = ((Int_t)GetData("tirdata") & 0x80) >> 7;   // pairsynch
};




