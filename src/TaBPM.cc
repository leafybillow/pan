//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBPM.cc  (implementation)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam position monitor data.  
//
//////////////////////////////////////////////////////////////////////////

#include "TaBPM.hh" 
#include "TTree.h"
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaBPM)
#endif

TaBPM::TaBPM(string name) : TaADC(name) {
}

TaBPM::~TaBPM() { }

TaBPM::TaBPM(const TaBPM& rhs) : TaADC(rhs) {
  kappa = rhs.kappa;
  x = rhs.x; 
  y = rhs.x;
  xkey = rhs.xkey;
  ykey = rhs.ykey;
  keys = rhs.keys;
  fData = rhs.fData;
}

TaBPM &TaBPM::operator=(const TaBPM &rhs) {
 if (this != &rhs) {
    TaADC::operator=(rhs);
    kappa = rhs.kappa;
    x = rhs.x; 
    y = rhs.x;
    xkey = rhs.xkey;
    ykey = rhs.ykey;
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
}

void TaBPM::Init(const VaDataBase& db) { 
    TaADC::Init(db);
// Add keys for the analyzed BPM data
    xkey = fName + "x";  // e.g. "bpm8" + "x" = "bpm8x"
    ykey = fName + "y";
    keys.push_back(xkey);  
    keys.push_back(ykey);
    kappa = 18.77;         // overall size calibration
};

void TaBPM::Decode(const TaEvent& event) {
    TaADC::Decode(event);
    Calibrate(event);
    VaDevice::DataCopy();
};

void TaBPM::Calibrate(const TaEvent& event) {
    TaADC::Calibrate(event);
    static map<string, Double_t>::iterator sd;
    static string antenna[] = {"xp","xm","yp","ym"};
    Double_t xydata[4];  // corresponds to antenna[0-3]
    string key;
    for (int chan = 0; chan < 4; chan++) {
        key = fName + antenna[chan];
        sd = corr_data.find(key);
        if (sd != corr_data.end()) {
	  xydata[chan] = corr_data[key];
	} else {        
          xydata[chan] = 0;  
          cout << "TaBPM:: WARNING: No calibration for key "<<key<<endl;
	}
    }
    x = -9999;  y = -9999;
    Double_t sum = xydata[0] + xydata[1];
    if (sum > 0) x = kappa * (xydata[0] - xydata[1])/(xydata[0] + xydata[1]);
    sum = xydata[2] + xydata[3];
    if (sum > 0) y = kappa * (xydata[2] - xydata[3])/(xydata[2] + xydata[3]);
    fData[xkey] = x;
    fData[ykey] = y;
};





