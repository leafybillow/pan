//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaDevice.cc   (implementation file)
//        ^^^^^^^^^^^
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//////////////////////////////////////////////////////////////////////////

//#define DEBUG 1

#include "TaDevice.hh" 
#include "VaDataBase.hh"
#include "TTree.h"

#ifdef DICT
ClassImp(TaDevice)
#endif

TaDevice::TaDevice() { 
   fNumRaw = 0;
   fRawKeys = new Int_t[MAXKEYS];
   fEvPointer = new Int_t[MAXKEYS];
   fPedestal = new Double_t[4*ADCNUM];
   fDacInt = new Double_t[4*ADCNUM];
   fDacSlope = new Double_t[4*ADCNUM];
   fAdcNum = new Int_t[MAXKEYS];
   fChanNum = new Int_t[MAXKEYS];
   memset(fRawKeys, 0, MAXKEYS*sizeof(Int_t));
   memset(fEvPointer, 0, MAXKEYS*sizeof(Int_t));
   memset(fPedestal, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDacInt, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDacSlope, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fAdcNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fChanNum, 0, MAXKEYS*sizeof(Int_t));
}

TaDevice::~TaDevice() {
  Uncreate();
}

TaDevice::TaDevice(const TaDevice& rhs) 
{
  Create (rhs);
}; 

TaDevice &TaDevice::operator=(const TaDevice &rhs)
{
  if ( &rhs != this )
    {
      Uncreate();
      Create (rhs);
    }
  return *this;
};

map<string, Int_t> TaDevice::GetKeyList() const {
  return fKeyToIdx;
};

void TaDevice::Init(VaDataBase& db) {
// Initialized the key mapping to integers, and   
// the list of raw data defined in the database.
// The database defines the channel mapping, but the
// keys in the database must match the fKeyToIdx map.

   Int_t key,iadc,ichan;
   string keystr;
   InitKeyList();
   TaKeyMap keymap;
   db.DataMapReStart();
   while ( db.NextDataMap() ) {
     string devicename = db.GetDataMapName();  
     keymap = db.GetKeyMap(devicename);
     vector<string> vkeys = keymap.GetKeys();
     for (vector<string>::iterator is = vkeys.begin(); 
        is != vkeys.end(); is++) {
           string keystr = *is;
           key = AddRawKey(keystr);
           if (key < 0) continue;
           fEvPointer[key] = keymap.GetEvOffset(keystr);
           fAdcNum[key]  = keymap.GetAdc(keystr);
           fChanNum[key] = keymap.GetChan(keystr);
      }
   }
   for (iadc = 0; iadc < ADCNUM; iadc++) {
     for (ichan = 0; ichan < 4; ichan++) {
        fPedestal[iadc*4 + ichan] = db.GetPedestal(iadc, ichan);
        fDacSlope[iadc*4 + ichan] = db.GetDacNoise(iadc, ichan, "slope");
        fDacInt[iadc*4 + ichan] = db.GetDacNoise(iadc, ichan, "int");
     }
   }
};

Int_t TaDevice::AddRawKey(string keyname) {
// Add a key to the list of raw data.
   Int_t key = GetKey(keyname);      
   if (key <= 0 && TADEVICE_VERBOSE == 1) {
      cout << "TaDevice::AddRawKey::WARNING:  Key "<<keyname;
      cout << " not in the pre-approved list."<<endl;
      return -1;
   }
   if (key >= MAXKEYS) {
      cout << "TaDevice::AddRawKey::ERROR:  Attempting to add too many keys.";
      cout << "Compile with a bigger MAXKEYS parameter."<<endl;
      return -1;
   }
   fRawKeys[fNumRaw] = key;
   fEvPointer[key] = 0;
   fNumRaw++;
   return key;
}     

Int_t TaDevice::GetKey(string keystr) const {
// Returns the integer key corresponding to the string key name.
// Users of this class who have a list of string keys should upon
// initialization of the code call this method to determine the 
// corresponding list of integer keys.  Then use those integers.  
// The string keys are still useful, however, to define names of
// variables in the tree, and to initialize datamap from database, 
// but one should *NOT* use this method inside an event loop !!
  
  //  return fKeyToIdx[keystr];
  return (fKeyToIdx.find(keystr))->second;
}


string TaDevice::GetKey(Int_t key) const {
  static string nothing = "nothing";
  for (map<string, Int_t>::const_iterator si = fKeyToIdx.begin(); 
       si != fKeyToIdx.end();  si++) {
        if (key == si->second) return si->first;
  }
  return nothing;
}

void TaDevice::InitKeyList() {
// Initialize the mapping between key names and integers.
// Users of device data should access the data using the integer keys.
// Example of how to get data:
//               GetData(IBCM1R);   
//
// I tried to anticipate all future devices, but if you need
// to add a new one, put it here (another 'insert' line) and
// define the key in DevTypes.hh

// BPM 8  raw antenna (xp, xm, yp, ym) and calibrated data (x, y)
  fKeyToIdx.insert(make_pair((string)"bpm8xp",IBPM8XP));
  fKeyToIdx.insert(make_pair((string)"bpm8xm",IBPM8XM));
  fKeyToIdx.insert(make_pair((string)"bpm8yp",IBPM8YP));
  fKeyToIdx.insert(make_pair((string)"bpm8ym",IBPM8YM));
  fKeyToIdx.insert(make_pair((string)"bpm8x", IBPM8X));
  fKeyToIdx.insert(make_pair((string)"bpm8y", IBPM8Y));

// BPM 10
  fKeyToIdx.insert(make_pair((string)"bpm10xp",IBPM10XP));
  fKeyToIdx.insert(make_pair((string)"bpm10xm",IBPM10XM));
  fKeyToIdx.insert(make_pair((string)"bpm10yp",IBPM10YP));
  fKeyToIdx.insert(make_pair((string)"bpm10ym",IBPM10YM));
  fKeyToIdx.insert(make_pair((string)"bpm10x", IBPM10X));
  fKeyToIdx.insert(make_pair((string)"bpm10y", IBPM10Y));

  fKeyToIdx.insert(make_pair((string)"bpm12xp",IBPM12XP));
  fKeyToIdx.insert(make_pair((string)"bpm12xm",IBPM12XM));
  fKeyToIdx.insert(make_pair((string)"bpm12yp",IBPM12YP));
  fKeyToIdx.insert(make_pair((string)"bpm12ym",IBPM12YM));
  fKeyToIdx.insert(make_pair((string)"bpm12x", IBPM12X));
  fKeyToIdx.insert(make_pair((string)"bpm12y", IBPM12Y));

  fKeyToIdx.insert(make_pair((string)"bpm4axp",IBPM4AXP));
  fKeyToIdx.insert(make_pair((string)"bpm4axm",IBPM4AXM));
  fKeyToIdx.insert(make_pair((string)"bpm4ayp",IBPM4AYP));
  fKeyToIdx.insert(make_pair((string)"bpm4aym",IBPM4AYM));
  fKeyToIdx.insert(make_pair((string)"bpm4ax", IBPM4AX));
  fKeyToIdx.insert(make_pair((string)"bpm4ay", IBPM4AY));

  fKeyToIdx.insert(make_pair((string)"bpm4bxp",IBPM4BXP));
  fKeyToIdx.insert(make_pair((string)"bpm4bxm",IBPM4BXM));
  fKeyToIdx.insert(make_pair((string)"bpm4byp",IBPM4BYP));
  fKeyToIdx.insert(make_pair((string)"bpm4bym",IBPM4BYM));
  fKeyToIdx.insert(make_pair((string)"bpm4bx", IBPM4BX));
  fKeyToIdx.insert(make_pair((string)"bpm4by", IBPM4BY));

// Injector striplines
  fKeyToIdx.insert(make_pair((string)"bpmin1xp",IBPMIN1XP));
  fKeyToIdx.insert(make_pair((string)"bpmin1xm",IBPMIN1XM));
  fKeyToIdx.insert(make_pair((string)"bpmin1yp",IBPMIN1YP));
  fKeyToIdx.insert(make_pair((string)"bpmin1ym",IBPMIN1YM));
  fKeyToIdx.insert(make_pair((string)"bpmin1x", IBPMIN1X));
  fKeyToIdx.insert(make_pair((string)"bpmin1y", IBPMIN1Y));

  fKeyToIdx.insert(make_pair((string)"bpmin2xp",IBPMIN2XP));
  fKeyToIdx.insert(make_pair((string)"bpmin2xm",IBPMIN2XM));
  fKeyToIdx.insert(make_pair((string)"bpmin2yp",IBPMIN2YP));
  fKeyToIdx.insert(make_pair((string)"bpmin2ym",IBPMIN2YM));
  fKeyToIdx.insert(make_pair((string)"bpmin2x", IBPMIN2X));
  fKeyToIdx.insert(make_pair((string)"bpmin2y", IBPMIN2Y));

// G0 cavity monitors, raw data ("r") and calibrated 
  fKeyToIdx.insert(make_pair((string)"bpmcav1xr",IBPMCAV1XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav1yr",IBPMCAV1YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav1x", IBPMCAV1X));
  fKeyToIdx.insert(make_pair((string)"bpmcav1y", IBPMCAV1Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav2xr",IBPMCAV2XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav2yr",IBPMCAV2YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav2x", IBPMCAV2X));
  fKeyToIdx.insert(make_pair((string)"bpmcav2y", IBPMCAV2Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav3xr",IBPMCAV3XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav3yr",IBPMCAV3YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav3x", IBPMCAV3X));
  fKeyToIdx.insert(make_pair((string)"bpmcav3y", IBPMCAV3Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav4xr",IBPMCAV4XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav4yr",IBPMCAV4YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav4x", IBPMCAV4X));
  fKeyToIdx.insert(make_pair((string)"bpmcav4y", IBPMCAV4Y));

// Old cavity monitors (Happex1 era BCMs).  
// Raw ("r") and calibrated
  fKeyToIdx.insert(make_pair((string)"bcm1r", IBCM1R));
  fKeyToIdx.insert(make_pair((string)"bcm1",  IBCM1));

  fKeyToIdx.insert(make_pair((string)"bcm2r", IBCM2R));
  fKeyToIdx.insert(make_pair((string)"bcm2",  IBCM2));

  fKeyToIdx.insert(make_pair((string)"bcm3r", IBCM3R));
  fKeyToIdx.insert(make_pair((string)"bcm3" , IBCM3));

// G0 cavity BCM1  raw data ("r") and calibrated data
  fKeyToIdx.insert(make_pair((string)"bcmcav1r",IBCMCAV1R));
  fKeyToIdx.insert(make_pair((string)"bcmcav1", IBCMCAV1));

  fKeyToIdx.insert(make_pair((string)"bcmcav2r", IBCMCAV2R));
  fKeyToIdx.insert(make_pair((string)"bcmcav2",  IBCMCAV2));

  fKeyToIdx.insert(make_pair((string)"bcmcav3r", IBCMCAV3R));
  fKeyToIdx.insert(make_pair((string)"bcmcav3",  IBCMCAV3));

  fKeyToIdx.insert(make_pair((string)"bcmcav4r", IBCMCAV4R));
  fKeyToIdx.insert(make_pair((string)"bcmcav4",  IBCMCAV4));

// Batteries
  fKeyToIdx.insert(make_pair((string)"batt1", IBATT1));
  fKeyToIdx.insert(make_pair((string)"batt2", IBATT2));
  fKeyToIdx.insert(make_pair((string)"batt3", IBATT3));
  fKeyToIdx.insert(make_pair((string)"batt4", IBATT4));
  fKeyToIdx.insert(make_pair((string)"batt5", IBATT5));
        
// Detectors, raw data ("r") and calibrated data
  fKeyToIdx.insert(make_pair((string)"det1r", IDET1R));
  fKeyToIdx.insert(make_pair((string)"det1",  IDET1));

  fKeyToIdx.insert(make_pair((string)"det2r", IDET2R));
  fKeyToIdx.insert(make_pair((string)"det2",  IDET2));

  fKeyToIdx.insert(make_pair((string)"det3r", IDET3R));
  fKeyToIdx.insert(make_pair((string)"det3",  IDET3));

  fKeyToIdx.insert(make_pair((string)"det4r", IDET4R));
  fKeyToIdx.insert(make_pair((string)"det4",  IDET4));
  
// ADCs first index is ADC slot, second is channel
// These are RAW data
  fKeyToIdx.insert(make_pair((string)"adc0_0",IADC0_0));
  fKeyToIdx.insert(make_pair((string)"adc0_1",IADC0_1));
  fKeyToIdx.insert(make_pair((string)"adc0_2",IADC0_2));
  fKeyToIdx.insert(make_pair((string)"adc0_3",IADC0_3));
  fKeyToIdx.insert(make_pair((string)"adc1_0",IADC1_0));
  fKeyToIdx.insert(make_pair((string)"adc1_1",IADC1_1));
  fKeyToIdx.insert(make_pair((string)"adc1_2",IADC1_2));
  fKeyToIdx.insert(make_pair((string)"adc1_3",IADC1_3));
  fKeyToIdx.insert(make_pair((string)"adc2_0",IADC2_0));
  fKeyToIdx.insert(make_pair((string)"adc2_1",IADC2_1));
  fKeyToIdx.insert(make_pair((string)"adc2_2",IADC2_2));
  fKeyToIdx.insert(make_pair((string)"adc2_3",IADC2_3));
  fKeyToIdx.insert(make_pair((string)"adc3_0",IADC3_0));
  fKeyToIdx.insert(make_pair((string)"adc3_1",IADC3_1));
  fKeyToIdx.insert(make_pair((string)"adc3_2",IADC3_2));
  fKeyToIdx.insert(make_pair((string)"adc3_3",IADC3_3));
  fKeyToIdx.insert(make_pair((string)"adc4_0",IADC4_0));
  fKeyToIdx.insert(make_pair((string)"adc4_1",IADC4_1));
  fKeyToIdx.insert(make_pair((string)"adc4_2",IADC4_2));
  fKeyToIdx.insert(make_pair((string)"adc4_3",IADC4_3));
  fKeyToIdx.insert(make_pair((string)"adc5_0",IADC5_0));
  fKeyToIdx.insert(make_pair((string)"adc5_1",IADC5_1));
  fKeyToIdx.insert(make_pair((string)"adc5_2",IADC5_2));
  fKeyToIdx.insert(make_pair((string)"adc5_3",IADC5_3));
  fKeyToIdx.insert(make_pair((string)"adc6_0",IADC6_0));
  fKeyToIdx.insert(make_pair((string)"adc6_1",IADC6_1));
  fKeyToIdx.insert(make_pair((string)"adc6_2",IADC6_2));
  fKeyToIdx.insert(make_pair((string)"adc6_3",IADC6_3));
  fKeyToIdx.insert(make_pair((string)"adc7_0",IADC7_0));
  fKeyToIdx.insert(make_pair((string)"adc7_1",IADC7_1));
  fKeyToIdx.insert(make_pair((string)"adc7_2",IADC7_2));
  fKeyToIdx.insert(make_pair((string)"adc7_3",IADC7_3));
  fKeyToIdx.insert(make_pair((string)"adc8_0",IADC8_0));
  fKeyToIdx.insert(make_pair((string)"adc8_1",IADC8_1));
  fKeyToIdx.insert(make_pair((string)"adc8_2",IADC8_2));
  fKeyToIdx.insert(make_pair((string)"adc8_3",IADC8_3));
  fKeyToIdx.insert(make_pair((string)"adc9_0",IADC9_0));
  fKeyToIdx.insert(make_pair((string)"adc9_1",IADC9_1));
  fKeyToIdx.insert(make_pair((string)"adc9_2",IADC9_2));
  fKeyToIdx.insert(make_pair((string)"adc9_3",IADC9_3));
  fKeyToIdx.insert(make_pair((string)"adc10_0",IADC10_0));
  fKeyToIdx.insert(make_pair((string)"adc10_1",IADC10_1));
  fKeyToIdx.insert(make_pair((string)"adc10_2",IADC10_2));
  fKeyToIdx.insert(make_pair((string)"adc10_3",IADC10_3));
  fKeyToIdx.insert(make_pair((string)"adc11_0",IADC11_0));
  fKeyToIdx.insert(make_pair((string)"adc11_1",IADC11_1));
  fKeyToIdx.insert(make_pair((string)"adc11_2",IADC11_2));
  fKeyToIdx.insert(make_pair((string)"adc11_3",IADC11_3));
  fKeyToIdx.insert(make_pair((string)"adc12_0",IADC12_0));
  fKeyToIdx.insert(make_pair((string)"adc12_1",IADC12_1));
  fKeyToIdx.insert(make_pair((string)"adc12_2",IADC12_2));
  fKeyToIdx.insert(make_pair((string)"adc12_3",IADC12_3));
  fKeyToIdx.insert(make_pair((string)"adc13_0",IADC13_0));
  fKeyToIdx.insert(make_pair((string)"adc13_1",IADC13_1));
  fKeyToIdx.insert(make_pair((string)"adc13_2",IADC13_2));
  fKeyToIdx.insert(make_pair((string)"adc13_3",IADC13_3));
  fKeyToIdx.insert(make_pair((string)"adc14_0",IADC14_0));
  fKeyToIdx.insert(make_pair((string)"adc14_1",IADC14_1));
  fKeyToIdx.insert(make_pair((string)"adc14_2",IADC14_2));
  fKeyToIdx.insert(make_pair((string)"adc14_3",IADC14_3));

// These are CALIBRATED data (dac noise and pedestal subtracted)
  fKeyToIdx.insert(make_pair((string)"adc_cal0_0",IADC_CAL0_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal0_1",IADC_CAL0_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal0_2",IADC_CAL0_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal0_3",IADC_CAL0_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal1_0",IADC_CAL1_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal1_1",IADC_CAL1_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal1_2",IADC_CAL1_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal1_3",IADC_CAL1_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal2_0",IADC_CAL2_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal2_1",IADC_CAL2_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal2_2",IADC_CAL2_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal2_3",IADC_CAL2_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal3_0",IADC_CAL3_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal3_1",IADC_CAL3_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal3_2",IADC_CAL3_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal3_3",IADC_CAL3_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal4_0",IADC_CAL4_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal4_1",IADC_CAL4_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal4_2",IADC_CAL4_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal4_3",IADC_CAL4_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal5_0",IADC_CAL5_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal5_1",IADC_CAL5_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal5_2",IADC_CAL5_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal5_3",IADC_CAL5_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal6_0",IADC_CAL6_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal6_1",IADC_CAL6_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal6_2",IADC_CAL6_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal6_3",IADC_CAL6_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal7_0",IADC_CAL7_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal7_1",IADC_CAL7_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal7_2",IADC_CAL7_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal7_3",IADC_CAL7_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal8_0",IADC_CAL8_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal8_1",IADC_CAL8_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal8_2",IADC_CAL8_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal8_3",IADC_CAL8_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal9_0",IADC_CAL9_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal9_1",IADC_CAL9_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal9_2",IADC_CAL9_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal9_3",IADC_CAL9_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal10_0",IADC_CAL10_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal10_1",IADC_CAL10_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal10_2",IADC_CAL10_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal10_3",IADC_CAL10_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal11_0",IADC_CAL11_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal11_1",IADC_CAL11_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal11_2",IADC_CAL11_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal11_3",IADC_CAL11_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal12_0",IADC_CAL12_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal12_1",IADC_CAL12_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal12_2",IADC_CAL12_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal12_3",IADC_CAL12_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal13_0",IADC_CAL13_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal13_1",IADC_CAL13_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal13_2",IADC_CAL13_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal13_3",IADC_CAL13_3));
  fKeyToIdx.insert(make_pair((string)"adc_cal14_0",IADC_CAL14_0));
  fKeyToIdx.insert(make_pair((string)"adc_cal14_1",IADC_CAL14_1));
  fKeyToIdx.insert(make_pair((string)"adc_cal14_2",IADC_CAL14_2));
  fKeyToIdx.insert(make_pair((string)"adc_cal14_3",IADC_CAL14_3));

// DAC noise
  fKeyToIdx.insert(make_pair((string)"dac0",IDAC0));
  fKeyToIdx.insert(make_pair((string)"dac1",IDAC1));
  fKeyToIdx.insert(make_pair((string)"dac2",IDAC2));
  fKeyToIdx.insert(make_pair((string)"dac3",IDAC3));
  fKeyToIdx.insert(make_pair((string)"dac4",IDAC4));
  fKeyToIdx.insert(make_pair((string)"dac5",IDAC5));
  fKeyToIdx.insert(make_pair((string)"dac6",IDAC6));
  fKeyToIdx.insert(make_pair((string)"dac7",IDAC7));
  fKeyToIdx.insert(make_pair((string)"dac8",IDAC8));
  fKeyToIdx.insert(make_pair((string)"dac9",IDAC9));
  fKeyToIdx.insert(make_pair((string)"dac10",IDAC10));
  fKeyToIdx.insert(make_pair((string)"dac11",IDAC11));
  fKeyToIdx.insert(make_pair((string)"dac12",IDAC12));
  fKeyToIdx.insert(make_pair((string)"dac13",IDAC13));
  fKeyToIdx.insert(make_pair((string)"dac14",IDAC14));

// CSR data
  fKeyToIdx.insert(make_pair((string)"csr0",ICSR0));
  fKeyToIdx.insert(make_pair((string)"csr1",ICSR1));
  fKeyToIdx.insert(make_pair((string)"csr2",ICSR2));
  fKeyToIdx.insert(make_pair((string)"csr3",ICSR3));
  fKeyToIdx.insert(make_pair((string)"csr4",ICSR4));
  fKeyToIdx.insert(make_pair((string)"csr5",ICSR5));
  fKeyToIdx.insert(make_pair((string)"csr6",ICSR6));
  fKeyToIdx.insert(make_pair((string)"csr7",ICSR7));
  fKeyToIdx.insert(make_pair((string)"csr8",ICSR8));
  fKeyToIdx.insert(make_pair((string)"csr9",ICSR9));
  fKeyToIdx.insert(make_pair((string)"csr10",ICSR10));
  fKeyToIdx.insert(make_pair((string)"csr11",ICSR11));
  fKeyToIdx.insert(make_pair((string)"csr12",ICSR12));
  fKeyToIdx.insert(make_pair((string)"csr13",ICSR13));
  fKeyToIdx.insert(make_pair((string)"csr14",ICSR14));

// Scalers
  fKeyToIdx.insert(make_pair((string)"scaler0_0",ISCALER0_0));
  fKeyToIdx.insert(make_pair((string)"scaler0_1",ISCALER0_1));
  fKeyToIdx.insert(make_pair((string)"scaler0_2",ISCALER0_2));
  fKeyToIdx.insert(make_pair((string)"scaler0_3",ISCALER0_3));
  fKeyToIdx.insert(make_pair((string)"scaler0_4",ISCALER0_4));
  fKeyToIdx.insert(make_pair((string)"scaler0_5",ISCALER0_5));
  fKeyToIdx.insert(make_pair((string)"scaler0_6",ISCALER0_6));
  fKeyToIdx.insert(make_pair((string)"scaler0_7",ISCALER0_7));
  fKeyToIdx.insert(make_pair((string)"scaler0_8",ISCALER0_8));
  fKeyToIdx.insert(make_pair((string)"scaler0_9",ISCALER0_9));
  fKeyToIdx.insert(make_pair((string)"scaler0_10",ISCALER0_10));
  fKeyToIdx.insert(make_pair((string)"scaler0_11",ISCALER0_11));
  fKeyToIdx.insert(make_pair((string)"scaler0_12",ISCALER0_12));
  fKeyToIdx.insert(make_pair((string)"scaler0_13",ISCALER0_13));
  fKeyToIdx.insert(make_pair((string)"scaler0_14",ISCALER0_14));
  fKeyToIdx.insert(make_pair((string)"scaler0_15",ISCALER0_15));
  fKeyToIdx.insert(make_pair((string)"scaler0_16",ISCALER0_16));
  fKeyToIdx.insert(make_pair((string)"scaler0_17",ISCALER0_17));
  fKeyToIdx.insert(make_pair((string)"scaler0_18",ISCALER0_18));
  fKeyToIdx.insert(make_pair((string)"scaler0_19",ISCALER0_19));
  fKeyToIdx.insert(make_pair((string)"scaler0_20",ISCALER0_20));
  fKeyToIdx.insert(make_pair((string)"scaler0_21",ISCALER0_21));
  fKeyToIdx.insert(make_pair((string)"scaler0_22",ISCALER0_22));
  fKeyToIdx.insert(make_pair((string)"scaler0_23",ISCALER0_23));
  fKeyToIdx.insert(make_pair((string)"scaler0_24",ISCALER0_24));
  fKeyToIdx.insert(make_pair((string)"scaler0_25",ISCALER0_25));
  fKeyToIdx.insert(make_pair((string)"scaler0_26",ISCALER0_26));
  fKeyToIdx.insert(make_pair((string)"scaler0_27",ISCALER0_27));
  fKeyToIdx.insert(make_pair((string)"scaler0_28",ISCALER0_28));
  fKeyToIdx.insert(make_pair((string)"scaler0_29",ISCALER0_29));
  fKeyToIdx.insert(make_pair((string)"scaler0_30",ISCALER0_30));
  fKeyToIdx.insert(make_pair((string)"scaler0_31",ISCALER0_31));

  fKeyToIdx.insert(make_pair((string)"scaler1_0",ISCALER1_0));
  fKeyToIdx.insert(make_pair((string)"scaler1_1",ISCALER1_1));
  fKeyToIdx.insert(make_pair((string)"scaler1_2",ISCALER1_2));
  fKeyToIdx.insert(make_pair((string)"scaler1_3",ISCALER1_3));
  fKeyToIdx.insert(make_pair((string)"scaler1_4",ISCALER1_4));
  fKeyToIdx.insert(make_pair((string)"scaler1_5",ISCALER1_5));
  fKeyToIdx.insert(make_pair((string)"scaler1_6",ISCALER1_6));
  fKeyToIdx.insert(make_pair((string)"scaler1_7",ISCALER1_7));
  fKeyToIdx.insert(make_pair((string)"scaler1_8",ISCALER1_8));
  fKeyToIdx.insert(make_pair((string)"scaler1_9",ISCALER1_9));
  fKeyToIdx.insert(make_pair((string)"scaler1_10",ISCALER1_10));
  fKeyToIdx.insert(make_pair((string)"scaler1_11",ISCALER1_11));
  fKeyToIdx.insert(make_pair((string)"scaler1_12",ISCALER1_12));
  fKeyToIdx.insert(make_pair((string)"scaler1_13",ISCALER1_13));
  fKeyToIdx.insert(make_pair((string)"scaler1_14",ISCALER1_14));
  fKeyToIdx.insert(make_pair((string)"scaler1_15",ISCALER1_15));
  fKeyToIdx.insert(make_pair((string)"scaler1_16",ISCALER1_16));
  fKeyToIdx.insert(make_pair((string)"scaler1_17",ISCALER1_17));
  fKeyToIdx.insert(make_pair((string)"scaler1_18",ISCALER1_18));
  fKeyToIdx.insert(make_pair((string)"scaler1_19",ISCALER1_19));
  fKeyToIdx.insert(make_pair((string)"scaler1_20",ISCALER1_20));
  fKeyToIdx.insert(make_pair((string)"scaler1_21",ISCALER1_21));
  fKeyToIdx.insert(make_pair((string)"scaler1_22",ISCALER1_22));
  fKeyToIdx.insert(make_pair((string)"scaler1_23",ISCALER1_23));
  fKeyToIdx.insert(make_pair((string)"scaler1_24",ISCALER1_24));
  fKeyToIdx.insert(make_pair((string)"scaler1_25",ISCALER1_25));
  fKeyToIdx.insert(make_pair((string)"scaler1_26",ISCALER1_26));
  fKeyToIdx.insert(make_pair((string)"scaler1_27",ISCALER1_27));
  fKeyToIdx.insert(make_pair((string)"scaler1_28",ISCALER1_28));
  fKeyToIdx.insert(make_pair((string)"scaler1_29",ISCALER1_29));
  fKeyToIdx.insert(make_pair((string)"scaler1_30",ISCALER1_30));
  fKeyToIdx.insert(make_pair((string)"scaler1_31",ISCALER1_31));

  fKeyToIdx.insert(make_pair((string)"scaler2_0",ISCALER2_0));
  fKeyToIdx.insert(make_pair((string)"scaler2_1",ISCALER2_1));
  fKeyToIdx.insert(make_pair((string)"scaler2_2",ISCALER2_2));
  fKeyToIdx.insert(make_pair((string)"scaler2_3",ISCALER2_3));
  fKeyToIdx.insert(make_pair((string)"scaler2_4",ISCALER2_4));
  fKeyToIdx.insert(make_pair((string)"scaler2_5",ISCALER2_5));
  fKeyToIdx.insert(make_pair((string)"scaler2_6",ISCALER2_6));
  fKeyToIdx.insert(make_pair((string)"scaler2_7",ISCALER2_7));
  fKeyToIdx.insert(make_pair((string)"scaler2_8",ISCALER2_8));
  fKeyToIdx.insert(make_pair((string)"scaler2_9",ISCALER2_9));
  fKeyToIdx.insert(make_pair((string)"scaler2_10",ISCALER2_10));
  fKeyToIdx.insert(make_pair((string)"scaler2_11",ISCALER2_11));
  fKeyToIdx.insert(make_pair((string)"scaler2_12",ISCALER2_12));
  fKeyToIdx.insert(make_pair((string)"scaler2_13",ISCALER2_13));
  fKeyToIdx.insert(make_pair((string)"scaler2_14",ISCALER2_14));
  fKeyToIdx.insert(make_pair((string)"scaler2_15",ISCALER2_15));
  fKeyToIdx.insert(make_pair((string)"scaler2_16",ISCALER2_16));
  fKeyToIdx.insert(make_pair((string)"scaler2_17",ISCALER2_17));
  fKeyToIdx.insert(make_pair((string)"scaler2_18",ISCALER2_18));
  fKeyToIdx.insert(make_pair((string)"scaler2_19",ISCALER2_19));
  fKeyToIdx.insert(make_pair((string)"scaler2_20",ISCALER2_20));
  fKeyToIdx.insert(make_pair((string)"scaler2_21",ISCALER2_21));
  fKeyToIdx.insert(make_pair((string)"scaler2_22",ISCALER2_22));
  fKeyToIdx.insert(make_pair((string)"scaler2_23",ISCALER2_23));
  fKeyToIdx.insert(make_pair((string)"scaler2_24",ISCALER2_24));
  fKeyToIdx.insert(make_pair((string)"scaler2_25",ISCALER2_25));
  fKeyToIdx.insert(make_pair((string)"scaler2_26",ISCALER2_26));
  fKeyToIdx.insert(make_pair((string)"scaler2_27",ISCALER2_27));
  fKeyToIdx.insert(make_pair((string)"scaler2_28",ISCALER2_28));
  fKeyToIdx.insert(make_pair((string)"scaler2_29",ISCALER2_29));
  fKeyToIdx.insert(make_pair((string)"scaler2_30",ISCALER2_30));
  fKeyToIdx.insert(make_pair((string)"scaler2_31",ISCALER2_31));

  fKeyToIdx.insert(make_pair((string)"scaler3_0",ISCALER3_0));
  fKeyToIdx.insert(make_pair((string)"scaler3_1",ISCALER3_1));
  fKeyToIdx.insert(make_pair((string)"scaler3_2",ISCALER3_2));
  fKeyToIdx.insert(make_pair((string)"scaler3_3",ISCALER3_3));
  fKeyToIdx.insert(make_pair((string)"scaler3_4",ISCALER3_4));
  fKeyToIdx.insert(make_pair((string)"scaler3_5",ISCALER3_5));
  fKeyToIdx.insert(make_pair((string)"scaler3_6",ISCALER3_6));
  fKeyToIdx.insert(make_pair((string)"scaler3_7",ISCALER3_7));
  fKeyToIdx.insert(make_pair((string)"scaler3_8",ISCALER3_8));
  fKeyToIdx.insert(make_pair((string)"scaler3_9",ISCALER3_9));
  fKeyToIdx.insert(make_pair((string)"scaler3_10",ISCALER3_10));
  fKeyToIdx.insert(make_pair((string)"scaler3_11",ISCALER3_11));
  fKeyToIdx.insert(make_pair((string)"scaler3_12",ISCALER3_12));
  fKeyToIdx.insert(make_pair((string)"scaler3_13",ISCALER3_13));
  fKeyToIdx.insert(make_pair((string)"scaler3_14",ISCALER3_14));
  fKeyToIdx.insert(make_pair((string)"scaler3_15",ISCALER3_15));
  fKeyToIdx.insert(make_pair((string)"scaler3_16",ISCALER3_16));
  fKeyToIdx.insert(make_pair((string)"scaler3_17",ISCALER3_17));
  fKeyToIdx.insert(make_pair((string)"scaler3_18",ISCALER3_18));
  fKeyToIdx.insert(make_pair((string)"scaler3_19",ISCALER3_19));
  fKeyToIdx.insert(make_pair((string)"scaler3_20",ISCALER3_20));
  fKeyToIdx.insert(make_pair((string)"scaler3_21",ISCALER3_21));
  fKeyToIdx.insert(make_pair((string)"scaler3_22",ISCALER3_22));
  fKeyToIdx.insert(make_pair((string)"scaler3_23",ISCALER3_23));
  fKeyToIdx.insert(make_pair((string)"scaler3_24",ISCALER3_24));
  fKeyToIdx.insert(make_pair((string)"scaler3_25",ISCALER3_25));
  fKeyToIdx.insert(make_pair((string)"scaler3_26",ISCALER3_26));
  fKeyToIdx.insert(make_pair((string)"scaler3_27",ISCALER3_27));
  fKeyToIdx.insert(make_pair((string)"scaler3_28",ISCALER3_28));
  fKeyToIdx.insert(make_pair((string)"scaler3_29",ISCALER3_29));
  fKeyToIdx.insert(make_pair((string)"scaler3_30",ISCALER3_30));
  fKeyToIdx.insert(make_pair((string)"scaler3_31",ISCALER3_31));

// TIR data from various crates
  fKeyToIdx.insert(make_pair((string)"tirdata", ITIRDATA));   // 1st crate
  fKeyToIdx.insert(make_pair((string)"tirdata2",ITIRDATA1));
  fKeyToIdx.insert(make_pair((string)"tirdata3",ITIRDATA2));
  fKeyToIdx.insert(make_pair((string)"tirdata4",ITIRDATA3));

// Helicity from various crates
  fKeyToIdx.insert(make_pair((string)"helicity", IHELICITY));  // 1st crate
  fKeyToIdx.insert(make_pair((string)"helicity2",IHELICITY1));
  fKeyToIdx.insert(make_pair((string)"helicity3",IHELICITY2));
  fKeyToIdx.insert(make_pair((string)"helicity4",IHELICITY3));

// Time slot from various crates
  fKeyToIdx.insert(make_pair((string)"timeslot", ITIMESLOT));
  fKeyToIdx.insert(make_pair((string)"timeslot2",ITIMESLOT1));
  fKeyToIdx.insert(make_pair((string)"timeslot3",ITIMESLOT2));
  fKeyToIdx.insert(make_pair((string)"timeslot4",ITIMESLOT3));

  fKeyToIdx.insert(make_pair((string)"rampdelay",IRAMPDELAY));
  fKeyToIdx.insert(make_pair((string)"rampdelay1",IRAMPDELAY1));
  fKeyToIdx.insert(make_pair((string)"rampdelay2",IRAMPDELAY2));
  fKeyToIdx.insert(make_pair((string)"rampdelay3",IRAMPDELAY3));

  fKeyToIdx.insert(make_pair((string)"integtime",IINTEGTIME));
  fKeyToIdx.insert(make_pair((string)"integtime1",IINTEGTIME1));
  fKeyToIdx.insert(make_pair((string)"integtime2",IINTEGTIME2));
  fKeyToIdx.insert(make_pair((string)"integtime3",IINTETTIME3));

  fKeyToIdx.insert(make_pair((string)"oversample",IOVERSAMPLE));
  fKeyToIdx.insert(make_pair((string)"oversample1",IOVERSAMPLE1));
  fKeyToIdx.insert(make_pair((string)"oversample2",IOVERSAMPLE2));
  fKeyToIdx.insert(make_pair((string)"oversample3",IOVERSAMPLE3));

  fKeyToIdx.insert(make_pair((string)"precdac",IPRECDAC));
  fKeyToIdx.insert(make_pair((string)"precdac1",IPRECDAC1));
  fKeyToIdx.insert(make_pair((string)"precdac2",IPRECDAC2));
  fKeyToIdx.insert(make_pair((string)"precdac3",IPRECDAC3));

};

void TaDevice::Create(const TaDevice& rhs)
{
   fNumRaw = rhs.fNumRaw;
   fKeyToIdx = rhs.fKeyToIdx;
   fRawKeys = new Int_t[MAXKEYS];
   memcpy(fRawKeys, rhs.fRawKeys, MAXKEYS*sizeof(Int_t));
   fEvPointer = new Int_t[MAXKEYS];
   memcpy(fEvPointer, rhs.fEvPointer, MAXKEYS*sizeof(Int_t));
   fPedestal = new Double_t[4*ADCNUM];
   memcpy(fPedestal, rhs.fPedestal, 4*ADCNUM*sizeof(Double_t));
   fDacInt = new Double_t[4*ADCNUM];
   memcpy(fDacInt, rhs.fDacInt, 4*ADCNUM*sizeof(Double_t));
   fDacSlope = new Double_t[4*ADCNUM];
   memcpy(fDacSlope, rhs.fDacSlope, 4*ADCNUM*sizeof(Double_t));
   fAdcNum = new Int_t[MAXKEYS];
   memcpy(fAdcNum, rhs.fAdcNum, MAXKEYS*sizeof(Int_t));
   fChanNum = new Int_t[MAXKEYS];
   memcpy(fChanNum, rhs.fChanNum, MAXKEYS*sizeof(Int_t));

 };

void TaDevice::Uncreate()
{
   delete [] fRawKeys;
   delete [] fEvPointer;
   delete [] fPedestal;
   delete [] fDacInt;
   delete [] fDacSlope;
   delete [] fAdcNum;
   delete [] fChanNum;
};


