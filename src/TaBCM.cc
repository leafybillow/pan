//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBCM.cc  (implementation)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam current monitor data.  
//
//////////////////////////////////////////////////////////////////////////


#include "TaBCM.hh" 
#include "TTree.h"
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaBCM)
#endif

TaBCM::TaBCM(string name) : TaADC(name) {
}

TaBCM::~TaBCM() { }

TaBCM::TaBCM(const TaBCM& rhs) : TaADC(rhs) {
  fName = rhs.fName;
  keys = rhs.keys;
  fData = rhs.fData;
}

TaBCM &TaBCM::operator=(const TaBCM &rhs) {
 if (this != &rhs) {
    TaADC::operator=(rhs);
    fName = rhs.fName;
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
}

void TaBCM::Init(const VaDataBase& db) { 
    TaADC::Init(db);
// Add a key for the corrected data (as opposed to raw data which
// is already in the superclass VaDevice).  
// Restriction:  There is only ONE channel for a BCM.
    if ((long)keys.size() != 1) {
      cout << "TaBCM:: ERROR:  There is only one channel in a BCM"<<endl;
      cout << "If you have two copies of the identical signal, make a 2nd"<<endl;
      cout << "entry in the database."<<endl;
    }
    fName = VaDevice::fName;  // expect this to be 'bcm1', 'bcm2', etc
    keys.push_back(fName);
};

void TaBCM::Decode(const TaEvent& event) {
    TaADC::Decode(event);
    Calibrate(event);
    VaDevice::DataCopy();
};

void TaBCM::Calibrate(const TaEvent& event) {
// The following only works for a unique bcm 'fName' (see restriction above)
    TaADC::Calibrate(event);
    fData[fName] = corr_data[keys[0]];
};






