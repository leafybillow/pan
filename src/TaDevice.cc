
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
#include "TaString.hh"
#include "TaDataBase.hh"
#include "TTree.h"

#ifndef NODICT
ClassImp(TaDevice)
#endif

TaDevice::TaDevice() { 
   fNumRaw = 0;
   fRawKeys = new Int_t[MAXKEYS];
   fEvPointer = new Int_t[MAXKEYS];
   fReadOut = new Int_t[MAXKEYS];
   fIsUsed = new Int_t[MAXKEYS];
   fIsRotated = new Int_t[MAXKEYS];
   fAdcPed = new Double_t[4*ADCNUM];
   fDacSlope = new Double_t[4*ADCNUM];
   fDevNum = new Int_t[MAXKEYS];
   fChanNum = new Int_t[MAXKEYS];
   fScalPed = new Double_t[32*SCANUM];
   memset(fRawKeys, 0, MAXKEYS*sizeof(Int_t));
   memset(fEvPointer, 0, MAXKEYS*sizeof(Int_t));
   memset(fReadOut, 0, MAXKEYS*sizeof(Int_t));
   memset(fIsUsed, 0, MAXKEYS*sizeof(Int_t));
   memset(fIsRotated, 0, MAXKEYS*sizeof(Int_t));
   memset(fAdcPed, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDacSlope, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDevNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fChanNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fScalPed, 0, 32*SCANUM*sizeof(Int_t));
}

TaDevice::~TaDevice() {
  Uncreate();
  fKeyToIdx.clear();
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

void TaDevice::Init(TaDataBase& db) {
// Initialized the key mapping to integers, and   
// the list of raw data defined in the database.
// The database defines the channel mapping, but the
// keys in the database must match the fKeyToIdx map.
// Set up rotated BPMs.

   Int_t key,iadc,isca,ichan;
   string keystr;
   InitKeyList();
   TaKeyMap keymap;
   BpmDefRotate();  
   db.DataMapReStart();
   while ( db.NextDataMap() ) {
     string devicename = db.GetDataMapName();  
     int isrotate = 0;
     for (vector<string>::iterator is = fRotateList.begin();
       is != fRotateList.end(); is++) {
         TaString sdevice = *is;
         if (sdevice.CmpNoCase(devicename) == 0) {
            isrotate = 1;
            break;
	 }
     }
     keymap = db.GetKeyMap(devicename);
     vector<string> vkeys = keymap.GetKeys();
     for (vector<string>::iterator is = vkeys.begin(); 
        is != vkeys.end(); is++) {
           string keystr = *is;
           key = AddRawKey(keystr);
           if (key < 0) continue;
           fEvPointer[key] = keymap.GetEvOffset(keystr);
           fReadOut[key] = -1;
           fIsUsed[key] = 1;
           if (keymap.IsAdc(keystr)) fReadOut[key] = ADCREADOUT;
           if (keymap.IsScaler(keystr)) fReadOut[key] = SCALREADOUT;
           fDevNum[key]  = keymap.GetAdc(keystr);
           fChanNum[key] = keymap.GetChan(keystr);
// Rotate state first set from default but can over-ridden by database
           if (keymap.GetRotate() != -1) {
               fIsRotated[key] = keymap.GetRotate();
               isrotate = fIsRotated[key];
	   }
           if (isrotate) {
               fIsRotated[key] = 1;
               fIsRotated[GetKey(devicename+"x")] = 1;
               fIsRotated[GetKey(devicename+"y")] = 1;
	   }
      }
      if (isrotate) cout << "  BPM "<<devicename<<" is ROTATED"<<endl;
   }
   for (iadc = 0; iadc < ADCNUM; iadc++) {
     for (ichan = 0; ichan < 4; ichan++) {
        fAdcPed[iadc*4 + ichan] = db.GetAdcPed(iadc, ichan);
        fDacSlope[iadc*4 + ichan] = db.GetDacNoise(iadc, ichan, "slope");
     }
   }
   for (isca = 0; isca < SCANUM; isca++) {
     for (ichan = 0;  ichan < 32; ichan++) {
       fScalPed[isca*32 + ichan] = db.GetScalPed(isca, ichan);
     }
   }
};

void TaDevice::BpmDefRotate() {
// Build default list of rotated BPMs.  
// BPMs listed here are rotated by 45 degrees.  The others are not
// rotated, they are up/down.  This can be over-ridden by the
// database:  by convention a device-name (3rd column) can have
// suffix "_r" or "_ur" to force rotation or unrotation.  Leaving
// out this suffix will result in the default behavior here.
// The suffix "_r" or "_ur" are otherwise IGNORED.
// Example of device-name in db file:  
//     "bpm8"    --> rotated (default here)
//     "bpm8_r"  --> still rotated
//     "bpm8_ur" --> unrotated.

   fRotateList.clear();
   fRotateList.push_back("bpm0L02");  
   fRotateList.push_back("bpm0L06");  
   fRotateList.push_back("bpm8");
   fRotateList.push_back("bpm10");
   fRotateList.push_back("bpm12");
   fRotateList.push_back("bpm4a");
   fRotateList.push_back("bpm4b");

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
  
  map<string, Int_t>::const_iterator ki = fKeyToIdx.find(keystr);
  if (ki == fKeyToIdx.end()) {
    cout << "TaDevice::GetKey:: ERROR:  cannot find key "<<keystr;
    cout << " returning zero "<<endl;
    return 0;
  }
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

Double_t TaDevice::GetPedestal(const Int_t& key) const {
  Int_t index;
  if (fReadOut[key] == ADCREADOUT) {
     index = key - ADCOFF;
     if (index >= 0 && index < 4*ADCNUM) return fAdcPed[index];
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = key - SCAOFF;
     if (index >= 0 && index < 32*SCANUM) return fScalPed[index];
  }  
  return 0;
}

Int_t TaDevice::GetRawIndex(const Int_t& key) const {
// Return a pointer to the raw ADC or SCALER data corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ADCOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = SCAOFF + 32*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  return index;
}

Int_t TaDevice::GetCalIndex(const Int_t& key) const {
// Return a pointer to the calibrated ADC or SCALER data corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ACCOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = SCCOFF + 32*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  return index;  
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
  fKeyToIdx.insert(make_pair((string)"bpm8xws", IBPM8XWS));
  fKeyToIdx.insert(make_pair((string)"bpm8yws", IBPM8YWS));
  fKeyToIdx.insert(make_pair((string)"bpm8ws", IBPM8WS));

// BPM 10
  fKeyToIdx.insert(make_pair((string)"bpm10xp",IBPM10XP));
  fKeyToIdx.insert(make_pair((string)"bpm10xm",IBPM10XM));
  fKeyToIdx.insert(make_pair((string)"bpm10yp",IBPM10YP));
  fKeyToIdx.insert(make_pair((string)"bpm10ym",IBPM10YM));
  fKeyToIdx.insert(make_pair((string)"bpm10x", IBPM10X));
  fKeyToIdx.insert(make_pair((string)"bpm10y", IBPM10Y));
  fKeyToIdx.insert(make_pair((string)"bpm10xws", IBPM10XWS));
  fKeyToIdx.insert(make_pair((string)"bpm10yws", IBPM10YWS));
  fKeyToIdx.insert(make_pair((string)"bpm10ws", IBPM10WS));

  fKeyToIdx.insert(make_pair((string)"bpm12xp",IBPM12XP));
  fKeyToIdx.insert(make_pair((string)"bpm12xm",IBPM12XM));
  fKeyToIdx.insert(make_pair((string)"bpm12yp",IBPM12YP));
  fKeyToIdx.insert(make_pair((string)"bpm12ym",IBPM12YM));
  fKeyToIdx.insert(make_pair((string)"bpm12x", IBPM12X));
  fKeyToIdx.insert(make_pair((string)"bpm12y", IBPM12Y));
  fKeyToIdx.insert(make_pair((string)"bpm12xws", IBPM12XWS));
  fKeyToIdx.insert(make_pair((string)"bpm12yws", IBPM12YWS));
  fKeyToIdx.insert(make_pair((string)"bpm12ws", IBPM12WS));

  fKeyToIdx.insert(make_pair((string)"bpm4axp",IBPM4AXP));
  fKeyToIdx.insert(make_pair((string)"bpm4axm",IBPM4AXM));
  fKeyToIdx.insert(make_pair((string)"bpm4ayp",IBPM4AYP));
  fKeyToIdx.insert(make_pair((string)"bpm4aym",IBPM4AYM));
  fKeyToIdx.insert(make_pair((string)"bpm4ax", IBPM4AX));
  fKeyToIdx.insert(make_pair((string)"bpm4ay", IBPM4AY));
  fKeyToIdx.insert(make_pair((string)"bpm4axws", IBPM4AXWS));
  fKeyToIdx.insert(make_pair((string)"bpm4ayws", IBPM4AYWS));
  fKeyToIdx.insert(make_pair((string)"bpm4aws", IBPM4AWS));

  fKeyToIdx.insert(make_pair((string)"bpm4bxp",IBPM4BXP));
  fKeyToIdx.insert(make_pair((string)"bpm4bxm",IBPM4BXM));
  fKeyToIdx.insert(make_pair((string)"bpm4byp",IBPM4BYP));
  fKeyToIdx.insert(make_pair((string)"bpm4bym",IBPM4BYM));
  fKeyToIdx.insert(make_pair((string)"bpm4bx", IBPM4BX));
  fKeyToIdx.insert(make_pair((string)"bpm4by", IBPM4BY));
  fKeyToIdx.insert(make_pair((string)"bpm4bxws", IBPM4BXWS));
  fKeyToIdx.insert(make_pair((string)"bpm4byws", IBPM4BYWS));
  fKeyToIdx.insert(make_pair((string)"bpm4bws", IBPM4BWS));

// Injector striplines
  fKeyToIdx.insert(make_pair((string)"bpm1I02xp",IBPM1I02XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xm",IBPM1I02XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I02yp",IBPM1I02YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ym",IBPM1I02YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I02x", IBPM1I02X));
  fKeyToIdx.insert(make_pair((string)"bpm1I02y", IBPM1I02Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xws", IBPM1I02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I02yws", IBPM1I02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ws", IBPM1I02WS));

  fKeyToIdx.insert(make_pair((string)"bpm1I04xp",IBPM1I04XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xm",IBPM1I04XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I04yp",IBPM1I04YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ym",IBPM1I04YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I04x", IBPM1I04X));
  fKeyToIdx.insert(make_pair((string)"bpm1I04y", IBPM1I04Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xws", IBPM1I04XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I04yws", IBPM1I04YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ws", IBPM1I04WS));

  fKeyToIdx.insert(make_pair((string)"bpm1I06xp",IBPM1I06XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xm",IBPM1I06XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I06yp",IBPM1I06YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ym",IBPM1I06YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I06x", IBPM1I06X));
  fKeyToIdx.insert(make_pair((string)"bpm1I06y", IBPM1I06Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xws", IBPM1I06XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I06yws", IBPM1I06YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ws", IBPM1I06WS));

  fKeyToIdx.insert(make_pair((string)"bpm0I02xp",IBPM0I02XP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xm",IBPM0I02XM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02yp",IBPM0I02YP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ym",IBPM0I02YM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02x", IBPM0I02X));
  fKeyToIdx.insert(make_pair((string)"bpm0I02y", IBPM0I02Y));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xws", IBPM0I02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02yws", IBPM0I02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ws", IBPM0I02WS));

  fKeyToIdx.insert(make_pair((string)"bpm0I02Axp",IBPM0I02AXP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axm",IBPM0I02AXM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ayp",IBPM0I02AYP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aym",IBPM0I02AYM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ax", IBPM0I02AX));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ay", IBPM0I02AY));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axws", IBPM0I02AXWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ayws", IBPM0I02AYWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aws", IBPM0I02AWS));

  fKeyToIdx.insert(make_pair((string)"bpm0L02xp",IBPM0L02XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xm",IBPM0L02XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L02yp",IBPM0L02YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ym",IBPM0L02YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L02x", IBPM0L02X));
  fKeyToIdx.insert(make_pair((string)"bpm0L02y", IBPM0L02Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xws", IBPM0L02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L02yws", IBPM0L02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ws", IBPM0L02WS));

  fKeyToIdx.insert(make_pair((string)"bpm0L06xp",IBPM0L06XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xm",IBPM0L06XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L06yp",IBPM0L06YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ym",IBPM0L06YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L06x", IBPM0L06X));
  fKeyToIdx.insert(make_pair((string)"bpm0L06y", IBPM0L06Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xws", IBPM0L06XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L06yws", IBPM0L06YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ws", IBPM0L06WS));

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

  fKeyToIdx.insert(make_pair((string)"bcm4r", IBCM4R));
  fKeyToIdx.insert(make_pair((string)"bcm4" , IBCM4));

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
  fKeyToIdx.insert(make_pair((string)"adc15_0",IADC15_0));
  fKeyToIdx.insert(make_pair((string)"adc15_1",IADC15_1));
  fKeyToIdx.insert(make_pair((string)"adc15_2",IADC15_2));
  fKeyToIdx.insert(make_pair((string)"adc15_3",IADC15_3));
  fKeyToIdx.insert(make_pair((string)"adc16_0",IADC16_0));
  fKeyToIdx.insert(make_pair((string)"adc16_1",IADC16_1));
  fKeyToIdx.insert(make_pair((string)"adc16_2",IADC16_2));
  fKeyToIdx.insert(make_pair((string)"adc16_3",IADC16_3));
  fKeyToIdx.insert(make_pair((string)"adc17_0",IADC17_0));
  fKeyToIdx.insert(make_pair((string)"adc17_1",IADC17_1));
  fKeyToIdx.insert(make_pair((string)"adc17_2",IADC17_2));
  fKeyToIdx.insert(make_pair((string)"adc17_3",IADC17_3));
  fKeyToIdx.insert(make_pair((string)"adc18_0",IADC18_0));
  fKeyToIdx.insert(make_pair((string)"adc18_1",IADC18_1));
  fKeyToIdx.insert(make_pair((string)"adc18_2",IADC18_2));
  fKeyToIdx.insert(make_pair((string)"adc18_3",IADC18_3));
  fKeyToIdx.insert(make_pair((string)"adc19_0",IADC19_0));
  fKeyToIdx.insert(make_pair((string)"adc19_1",IADC19_1));
  fKeyToIdx.insert(make_pair((string)"adc19_2",IADC19_2));
  fKeyToIdx.insert(make_pair((string)"adc19_3",IADC19_3));
  fKeyToIdx.insert(make_pair((string)"adc20_0",IADC20_0));
  fKeyToIdx.insert(make_pair((string)"adc20_1",IADC20_1));
  fKeyToIdx.insert(make_pair((string)"adc20_2",IADC20_2));
  fKeyToIdx.insert(make_pair((string)"adc20_3",IADC20_3));
  fKeyToIdx.insert(make_pair((string)"adc21_0",IADC21_0));
  fKeyToIdx.insert(make_pair((string)"adc21_1",IADC21_1));
  fKeyToIdx.insert(make_pair((string)"adc21_2",IADC21_2));
  fKeyToIdx.insert(make_pair((string)"adc21_3",IADC21_3));
  fKeyToIdx.insert(make_pair((string)"adc22_0",IADC22_0));
  fKeyToIdx.insert(make_pair((string)"adc22_1",IADC22_1));
  fKeyToIdx.insert(make_pair((string)"adc22_2",IADC22_2));
  fKeyToIdx.insert(make_pair((string)"adc22_3",IADC22_3));

// These are CALIBRATED data (dac noise and pedestal subtracted)
  fKeyToIdx.insert(make_pair((string)"adc0_0_cal",IADC0_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc0_1_cal",IADC0_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc0_2_cal",IADC0_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc0_3_cal",IADC0_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc1_0_cal",IADC1_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc1_1_cal",IADC1_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc1_2_cal",IADC1_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc1_3_cal",IADC1_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc2_0_cal",IADC2_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc2_1_cal",IADC2_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc2_2_cal",IADC2_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc2_3_cal",IADC2_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc3_0_cal",IADC3_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc3_1_cal",IADC3_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc3_2_cal",IADC3_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc3_3_cal",IADC3_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc4_0_cal",IADC4_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc4_1_cal",IADC4_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc4_2_cal",IADC4_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc4_3_cal",IADC4_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc5_0_cal",IADC5_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc5_1_cal",IADC5_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc5_2_cal",IADC5_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc5_3_cal",IADC5_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc6_0_cal",IADC6_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc6_1_cal",IADC6_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc6_2_cal",IADC6_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc6_3_cal",IADC6_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc7_0_cal",IADC7_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc7_1_cal",IADC7_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc7_2_cal",IADC7_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc7_3_cal",IADC7_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc8_0_cal",IADC8_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc8_1_cal",IADC8_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc8_2_cal",IADC8_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc8_3_cal",IADC8_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc9_0_cal",IADC9_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc9_1_cal",IADC9_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc9_2_cal",IADC9_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc9_3_cal",IADC9_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc10_0_cal",IADC10_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc10_1_cal",IADC10_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc10_2_cal",IADC10_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc10_3_cal",IADC10_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc11_0_cal",IADC11_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc11_1_cal",IADC11_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc11_2_cal",IADC11_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc11_3_cal",IADC11_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc12_0_cal",IADC12_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc12_1_cal",IADC12_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc12_2_cal",IADC12_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc12_3_cal",IADC12_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc13_0_cal",IADC13_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc13_1_cal",IADC13_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc13_2_cal",IADC13_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc13_3_cal",IADC13_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc14_0_cal",IADC14_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc14_1_cal",IADC14_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc14_2_cal",IADC14_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc14_3_cal",IADC14_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc15_0_cal",IADC15_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc15_1_cal",IADC15_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc15_2_cal",IADC15_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc15_3_cal",IADC15_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc16_0_cal",IADC16_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc16_1_cal",IADC16_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc16_2_cal",IADC16_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc16_3_cal",IADC16_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc17_0_cal",IADC17_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc17_1_cal",IADC17_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc17_2_cal",IADC17_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc17_3_cal",IADC17_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc18_0_cal",IADC18_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc18_1_cal",IADC18_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc18_2_cal",IADC18_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc18_3_cal",IADC18_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc19_0_cal",IADC19_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc19_1_cal",IADC19_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc19_2_cal",IADC19_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc19_3_cal",IADC19_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc20_0_cal",IADC20_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc20_1_cal",IADC20_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc20_2_cal",IADC20_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc20_3_cal",IADC20_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc21_0_cal",IADC21_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc21_1_cal",IADC21_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc21_2_cal",IADC21_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc21_3_cal",IADC21_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc22_0_cal",IADC22_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc22_1_cal",IADC22_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc22_2_cal",IADC22_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc22_3_cal",IADC22_3_CAL));

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
  fKeyToIdx.insert(make_pair((string)"dac15",IDAC15));
  fKeyToIdx.insert(make_pair((string)"dac16",IDAC16));
  fKeyToIdx.insert(make_pair((string)"dac17",IDAC17));
  fKeyToIdx.insert(make_pair((string)"dac18",IDAC18));
  fKeyToIdx.insert(make_pair((string)"dac19",IDAC19));
  fKeyToIdx.insert(make_pair((string)"dac20",IDAC20));
  fKeyToIdx.insert(make_pair((string)"dac21",IDAC21));
  fKeyToIdx.insert(make_pair((string)"dac22",IDAC22));

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
  fKeyToIdx.insert(make_pair((string)"csr15",ICSR15));
  fKeyToIdx.insert(make_pair((string)"csr16",ICSR16));
  fKeyToIdx.insert(make_pair((string)"csr17",ICSR17));
  fKeyToIdx.insert(make_pair((string)"csr18",ICSR18));
  fKeyToIdx.insert(make_pair((string)"csr19",ICSR19));
  fKeyToIdx.insert(make_pair((string)"csr20",ICSR20));
  fKeyToIdx.insert(make_pair((string)"csr21",ICSR21));
  fKeyToIdx.insert(make_pair((string)"csr22",ICSR22));

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

  // Calibrated Scaler Data for V2F (ped subtracted, clk divided)
  fKeyToIdx.insert(make_pair((string)"scaler0_0_cal",ISCALER0_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_1_cal",ISCALER0_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_2_cal",ISCALER0_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_3_cal",ISCALER0_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_4_cal",ISCALER0_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_5_cal",ISCALER0_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_6_cal",ISCALER0_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_7_cal",ISCALER0_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_8_cal",ISCALER0_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_9_cal",ISCALER0_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_10_cal",ISCALER0_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_11_cal",ISCALER0_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_12_cal",ISCALER0_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_13_cal",ISCALER0_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_14_cal",ISCALER0_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_15_cal",ISCALER0_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_16_cal",ISCALER0_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_17_cal",ISCALER0_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_18_cal",ISCALER0_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_19_cal",ISCALER0_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_20_cal",ISCALER0_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_21_cal",ISCALER0_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_22_cal",ISCALER0_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_23_cal",ISCALER0_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_24_cal",ISCALER0_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_25_cal",ISCALER0_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_26_cal",ISCALER0_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_27_cal",ISCALER0_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_28_cal",ISCALER0_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_29_cal",ISCALER0_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_30_cal",ISCALER0_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler0_31_cal",ISCALER0_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler1_0_cal",ISCALER1_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_1_cal",ISCALER1_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_2_cal",ISCALER1_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_3_cal",ISCALER1_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_4_cal",ISCALER1_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_5_cal",ISCALER1_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_6_cal",ISCALER1_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_7_cal",ISCALER1_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_8_cal",ISCALER1_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_9_cal",ISCALER1_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_10_cal",ISCALER1_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_11_cal",ISCALER1_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_12_cal",ISCALER1_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_13_cal",ISCALER1_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_14_cal",ISCALER1_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_15_cal",ISCALER1_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_16_cal",ISCALER1_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_17_cal",ISCALER1_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_18_cal",ISCALER1_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_19_cal",ISCALER1_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_20_cal",ISCALER1_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_21_cal",ISCALER1_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_22_cal",ISCALER1_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_23_cal",ISCALER1_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_24_cal",ISCALER1_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_25_cal",ISCALER1_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_26_cal",ISCALER1_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_27_cal",ISCALER1_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_28_cal",ISCALER1_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_29_cal",ISCALER1_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_30_cal",ISCALER1_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler1_31_cal",ISCALER1_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler2_0_cal",ISCALER2_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_1_cal",ISCALER2_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_2_cal",ISCALER2_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_3_cal",ISCALER2_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_4_cal",ISCALER2_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_5_cal",ISCALER2_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_6_cal",ISCALER2_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_7_cal",ISCALER2_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_8_cal",ISCALER2_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_9_cal",ISCALER2_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_10_cal",ISCALER2_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_11_cal",ISCALER2_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_12_cal",ISCALER2_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_13_cal",ISCALER2_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_14_cal",ISCALER2_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_15_cal",ISCALER2_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_16_cal",ISCALER2_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_17_cal",ISCALER2_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_18_cal",ISCALER2_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_19_cal",ISCALER2_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_20_cal",ISCALER2_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_21_cal",ISCALER2_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_22_cal",ISCALER2_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_23_cal",ISCALER2_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_24_cal",ISCALER2_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_25_cal",ISCALER2_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_26_cal",ISCALER2_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_27_cal",ISCALER2_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_28_cal",ISCALER2_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_29_cal",ISCALER2_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_30_cal",ISCALER2_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler2_31_cal",ISCALER2_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler3_0_cal",ISCALER3_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_1_cal",ISCALER3_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_2_cal",ISCALER3_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_3_cal",ISCALER3_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_4_cal",ISCALER3_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_5_cal",ISCALER3_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_6_cal",ISCALER3_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_7_cal",ISCALER3_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_8_cal",ISCALER3_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_9_cal",ISCALER3_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_10_cal",ISCALER3_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_11_cal",ISCALER3_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_12_cal",ISCALER3_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_13_cal",ISCALER3_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_14_cal",ISCALER3_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_15_cal",ISCALER3_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_16_cal",ISCALER3_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_17_cal",ISCALER3_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_18_cal",ISCALER3_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_19_cal",ISCALER3_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_20_cal",ISCALER3_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_21_cal",ISCALER3_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_22_cal",ISCALER3_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_23_cal",ISCALER3_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_24_cal",ISCALER3_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_25_cal",ISCALER3_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_26_cal",ISCALER3_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_27_cal",ISCALER3_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_28_cal",ISCALER3_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_29_cal",ISCALER3_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_30_cal",ISCALER3_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler3_31_cal",ISCALER3_31_CAL));


// TIR data from various crates
  fKeyToIdx.insert(make_pair((string)"tirdata", ITIRDATA));   // 1st crate
  fKeyToIdx.insert(make_pair((string)"tirdata1",ITIRDATA1));
  fKeyToIdx.insert(make_pair((string)"tirdata2",ITIRDATA2));
  fKeyToIdx.insert(make_pair((string)"tirdata3",ITIRDATA3));

// Helicity from various crates
  fKeyToIdx.insert(make_pair((string)"helicity", IHELICITY));  // 1st crate
  fKeyToIdx.insert(make_pair((string)"helicity1",IHELICITY1));
  fKeyToIdx.insert(make_pair((string)"helicity2",IHELICITY2));
  fKeyToIdx.insert(make_pair((string)"helicity3",IHELICITY3));

// Time slot from various crates
  fKeyToIdx.insert(make_pair((string)"timeslot", ITIMESLOT));
  fKeyToIdx.insert(make_pair((string)"timeslot1",ITIMESLOT1));
  fKeyToIdx.insert(make_pair((string)"timeslot2",ITIMESLOT2));
  fKeyToIdx.insert(make_pair((string)"timeslot3",ITIMESLOT3));

  fKeyToIdx.insert(make_pair((string)"timeboard",ITIMEBOARD));
  fKeyToIdx.insert(make_pair((string)"timeboard1",ITIMEBOARD1));
  fKeyToIdx.insert(make_pair((string)"timeboard2",ITIMEBOARD2));
  fKeyToIdx.insert(make_pair((string)"timeboard3",ITIMEBOARD3));

  fKeyToIdx.insert(make_pair((string)"pairsynch",IPAIRSYNCH));
  fKeyToIdx.insert(make_pair((string)"pairsynch1",IPAIRSYNCH1));
  fKeyToIdx.insert(make_pair((string)"pairsynch2",IPAIRSYNCH2));
  fKeyToIdx.insert(make_pair((string)"pairsynch3",IPAIRSYNCH3));

  fKeyToIdx.insert(make_pair((string)"quadsynch",IQUADSYNCH));
  fKeyToIdx.insert(make_pair((string)"quadsynch1",IQUADSYNCH1));
  fKeyToIdx.insert(make_pair((string)"quadsynch2",IQUADSYNCH2));
  fKeyToIdx.insert(make_pair((string)"quadsynch3",IQUADSYNCH3));

  fKeyToIdx.insert(make_pair((string)"rampdelay",IRAMPDELAY));
  fKeyToIdx.insert(make_pair((string)"rampdelay1",IRAMPDELAY1));
  fKeyToIdx.insert(make_pair((string)"rampdelay2",IRAMPDELAY2));
  fKeyToIdx.insert(make_pair((string)"rampdelay3",IRAMPDELAY3));

  fKeyToIdx.insert(make_pair((string)"integtime",IINTEGTIME));
  fKeyToIdx.insert(make_pair((string)"integtime1",IINTEGTIME1));
  fKeyToIdx.insert(make_pair((string)"integtime2",IINTEGTIME2));
  fKeyToIdx.insert(make_pair((string)"integtime3",IINTEGTIME3));

  fKeyToIdx.insert(make_pair((string)"oversample",IOVERSAMPLE));
  fKeyToIdx.insert(make_pair((string)"oversample1",IOVERSAMPLE1));
  fKeyToIdx.insert(make_pair((string)"oversample2",IOVERSAMPLE2));
  fKeyToIdx.insert(make_pair((string)"oversample3",IOVERSAMPLE3));

  fKeyToIdx.insert(make_pair((string)"precdac",IPRECDAC));
  fKeyToIdx.insert(make_pair((string)"precdac1",IPRECDAC1));
  fKeyToIdx.insert(make_pair((string)"precdac2",IPRECDAC2));
  fKeyToIdx.insert(make_pair((string)"precdac3",IPRECDAC3));

  fKeyToIdx.insert(make_pair((string)"pitadac",IPITADAC));
  fKeyToIdx.insert(make_pair((string)"pitadac1",IPITADAC1));
  fKeyToIdx.insert(make_pair((string)"pitadac2",IPITADAC2));
  fKeyToIdx.insert(make_pair((string)"pitadac3",IPITADAC3));

  fKeyToIdx.insert(make_pair((string)"lumi1r", ILUMI1R));
  fKeyToIdx.insert(make_pair((string)"lumi1",  ILUMI1));
  fKeyToIdx.insert(make_pair((string)"lumi2r", ILUMI2R));
  fKeyToIdx.insert(make_pair((string)"lumi2",  ILUMI2));
  fKeyToIdx.insert(make_pair((string)"lumi3r", ILUMI3R));
  fKeyToIdx.insert(make_pair((string)"lumi3",  ILUMI3));
  fKeyToIdx.insert(make_pair((string)"lumi4r", ILUMI4R));
  fKeyToIdx.insert(make_pair((string)"lumi4",  ILUMI4));

  fKeyToIdx.insert(make_pair((string)"v2f_clk0",IV2F_CLK0));
  fKeyToIdx.insert(make_pair((string)"v2f_clk1",IV2F_CLK1));
  fKeyToIdx.insert(make_pair((string)"v2f_clk2",IV2F_CLK2));
  fKeyToIdx.insert(make_pair((string)"v2f_clk3",IV2F_CLK3));

  fKeyToIdx.insert(make_pair((string)"bmw_clean",IBMW_CLN));
  fKeyToIdx.insert(make_pair((string)"bmw_obj",IBMW_OBJ));
  fKeyToIdx.insert(make_pair((string)"bmw_val",IBMW_VAL));
  fKeyToIdx.insert(make_pair((string)"bmw_cyc",IBMW_CYC));

};

void TaDevice::Create(const TaDevice& rhs)
{
   fNumRaw = rhs.fNumRaw;
   fKeyToIdx = rhs.fKeyToIdx;
   fRawKeys = new Int_t[MAXKEYS];
   memcpy(fRawKeys, rhs.fRawKeys, MAXKEYS*sizeof(Int_t));
   fEvPointer = new Int_t[MAXKEYS];
   memcpy(fEvPointer, rhs.fEvPointer, MAXKEYS*sizeof(Int_t));
   fReadOut = new Int_t[MAXKEYS];
   memcpy(fReadOut, rhs.fReadOut, MAXKEYS*sizeof(Int_t));
   fIsUsed = new Int_t[MAXKEYS];
   memcpy(fIsUsed, rhs.fIsUsed, MAXKEYS*sizeof(Int_t));
   fIsRotated = new Int_t[MAXKEYS];
   memcpy(fIsRotated, rhs.fIsRotated, MAXKEYS*sizeof(Int_t));
   fAdcPed = new Double_t[4*ADCNUM];
   memcpy(fAdcPed, rhs.fAdcPed, 4*ADCNUM*sizeof(Double_t));
   fDacSlope = new Double_t[4*ADCNUM];
   memcpy(fDacSlope, rhs.fDacSlope, 4*ADCNUM*sizeof(Double_t));
   fDevNum = new Int_t[MAXKEYS];
   memcpy(fDevNum, rhs.fDevNum, MAXKEYS*sizeof(Int_t));
   fChanNum = new Int_t[MAXKEYS];
   memcpy(fChanNum, rhs.fChanNum, MAXKEYS*sizeof(Int_t));
};

void TaDevice::Uncreate()
{
   delete [] fRawKeys;
   delete [] fEvPointer;
   delete [] fReadOut;
   delete [] fIsUsed;
   delete [] fIsRotated;
   delete [] fAdcPed;
   delete [] fDacSlope;
   delete [] fDevNum;
   delete [] fChanNum;
};


