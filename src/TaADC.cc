//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaADC.cc   (implementation)
//           ^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Data from a Princeton/Harvard ADC used by HAPPEX DAQ.
//
//////////////////////////////////////////////////////////////////////////

#include "TaADC.hh" 
#include "VaDataBase.hh"
#include "TaEvent.hh"

#ifdef DICT
ClassImp(TaADC)
#endif

TaADC::TaADC(string name) : VaDevice(name) {
}

TaADC::~TaADC() { }

void TaADC::Init(const VaDataBase& db) { 
  VaDevice::Init(db);
  fType  = "adc";
  header = db.GetHeader(fType);
  mask   = db.GetMask(fType);
  fadcnumber = -1;
  char label[20];
  pair<string, Int_t> si;
  pair<string, Double_t> sd;
  for (vector<string>::iterator ikey = keys.begin();
    ikey != keys.end(); ikey++) {
      string key = *ikey;
      if ((fadcnumber != -1) && (fadcnumber != keymap->GetAdc(key))) {
        cout << "TaADC:: ERROR: Duplicate ADC number for this key "<<key<<endl;
        cout << "This is an error in the database."<<endl;
      }
      fadcnumber = keymap->GetAdc(key);
      Int_t chan = keymap->GetChan(key);
      sprintf(label,"adc%d",fadcnumber); fadclabel = label;
      si.first = key;  sd.first = key;
      si.second = (int)db.GetPedestal(fadcnumber, chan); pedestal.insert(si);
      si.second = chan;  channels.insert(si);
      sd.second = db.GetDacNoise(fadcnumber, chan, "slope"); dacslope.insert(sd);
      sd.second = db.GetDacNoise(fadcnumber, chan, "int"); dacint.insert(sd);
  }
  // Here one could add keys like "adcNchanM" synonomous with the keys above,
  // and map them onto the above keyed data, but I won't bother for now.
};

TaADC::TaADC(const TaADC& rhs) : VaDevice(rhs) {
    fadcnumber = rhs.fadcnumber;
    fadclabel = rhs.fadclabel;
    pedestal = rhs.pedestal;
    channels = rhs.channels;
    dacslope = rhs.dacslope;
    dacint = rhs.dacint;
    corr_data = rhs.corr_data;
    keys = rhs.keys;
    fData = rhs.fData;
}

TaADC &TaADC::operator=(const TaADC &rhs) {
 if (this != &rhs) {
    VaDevice::operator=(rhs);
    fadcnumber = rhs.fadcnumber;
    fadclabel = rhs.fadclabel;
    pedestal = rhs.pedestal;
    channels = rhs.channels;
    dacslope = rhs.dacslope;
    dacint = rhs.dacint;
    corr_data = rhs.corr_data;
    keys = rhs.keys;
    fData = rhs.fData;
 }
 return *this;
}

Bool_t TaADC::IsADC() const {
   return kTRUE;
};

Int_t TaADC::GetADCNumber() const {
  return fadcnumber; 
};

string TaADC::GetADCLabel() const {
  return fadclabel; 
};

Int_t TaADC::GetChannel(string key) const {
  map< string, Int_t >::const_iterator chci = channels.find(key);
  if ( chci==channels.end() ) return 0;
  return chci->second; 
};

void TaADC::Decode(const TaEvent& event) {
  VaDevice::Decode(event);
  VaDevice::DataCopy();
};

void TaADC::Calibrate(const TaEvent& event) {
  if ( !inited ) {
      cout << "TaADC:: ERROR: calibration not set up.  Must Init(db)"<<endl;
      return;
  }
  corr_data.clear();
  for (vector<string>::iterator ikey = keys.begin();
    ikey != keys.end(); ikey++) {
    string key = *ikey;
    if ( !IsRaw(key) ) continue;       // not raw data
    Double_t dacvalue = event.GetData(fadclabel,"dac");
    Double_t corrected = event.GetData(VaDevice::fName,key) - pedestal[key] -
          ( dacslope[key] * dacvalue - dacint[key] );
    corr_data.insert(make_pair(key, corrected));
  }    
};







