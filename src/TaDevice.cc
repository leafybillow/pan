//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaDevice.cc   (implementation file)
//        ^^^^^^^^^^^
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
//////////////////////////////////////////////////////////////////////////

//#define DEBUG 1

#include "TaDevice.hh" 
#include "VaEvent.hh"
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
   fCrate = new Int_t[MAXKEYS];
   fReadOut = new Int_t[MAXKEYS];
   fIsUsed = new Int_t[MAXKEYS];
   fIsRotated = new Int_t[MAXKEYS];
   fAdcPed = new Double_t[4*ADCNUM];
   fDacSlope = new Double_t[4*ADCNUM];
   fDevNum = new Int_t[MAXKEYS];
   fChanNum = new Int_t[MAXKEYS];
   fScalPed = new Double_t[32*SCANUM];
   fAdcptr = new Int_t[MAXROC];
   fScalptr = new Int_t[MAXROC];
   fTbdptr = new Int_t[MAXROC];
   fTirptr = new Int_t[MAXROC];
   fDaqptr = new Int_t[MAXROC];
   memset(fRawKeys, 0, MAXKEYS*sizeof(Int_t));
   memset(fEvPointer, 0, MAXKEYS*sizeof(Int_t));
   memset(fCrate, 0, MAXKEYS*sizeof(Int_t));
   memset(fReadOut, 0, MAXKEYS*sizeof(Int_t));
   memset(fIsUsed, 0, MAXKEYS*sizeof(Int_t));
   memset(fIsRotated, 0, MAXKEYS*sizeof(Int_t));
   memset(fAdcPed, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDacSlope, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDevNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fChanNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fScalPed, 0, 32*SCANUM*sizeof(Int_t));
   memset(fAdcptr, 0, MAXROC*sizeof(Int_t));
   memset(fScalptr, 0, MAXROC*sizeof(Int_t));
   memset(fTbdptr, 0, MAXROC*sizeof(Int_t));
   memset(fTirptr, 0, MAXROC*sizeof(Int_t));
   memset(fDaqptr, 0, MAXROC*sizeof(Int_t));
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
// Some devices like 'bpm', 'bcm', 'lumi' can be tied
// to others like 'adc', 'scaler' according to rules.

   Int_t key,tiedkey,iadc,isca,ichan,i,k;
   string keystr;
   InitKeyList();
   TaKeyMap keymap;
   BpmDefRotate();  
   db.DataMapReStart();
   fgAdcHeader = db.GetHeader("adc");
   fgAdcMask = db.GetMask("adc");
   fgScalHeader = db.GetHeader("scaler");
   fgScalMask = db.GetMask("scaler");
   fgTbdHeader = db.GetHeader("timeboard");
   fgTbdMask = db.GetMask("timeboard");
   fgTirHeader = db.GetHeader("tir");
   fgTirMask = db.GetMask("tir");
   fgDaqHeader = db.GetHeader("daqflag");
   fgDaqMask = db.GetMask("daqflag");
// Try to recover from a database that doesn't define header, mask.
   if (fgAdcHeader == 0) {
     fgAdcHeader = 0xffadc000;
     cout << "WARNING:  Header for ADC was zero.";
     cout <<"  Using default  " << hex << fgAdcHeader << dec << endl;
   }
   if (fgAdcMask == 0) {
     fgAdcMask = 0xfffff000;
     cout << "WARNING:  Mask for ADC was zero.";
     cout <<"  Using default  " << hex << fgAdcMask << dec << endl;
   }
   if (fgScalHeader == 0) {
     fgScalHeader = 0xfffca000;
     cout << "WARNING:  Header for Scaler was zero.";
     cout <<"  Using default  " << hex << fgScalHeader << dec << endl;
   }
   if (fgScalMask == 0) {
     fgScalMask = 0xfffff000;
     cout << "WARNING:  Mask for Scaler was zero.";
     cout <<"  Using default  " << hex << fgScalMask << dec << endl;
   }
   if (fgTbdHeader == 0) {
     fgTbdHeader = 0xfffbd000;
     cout << "WARNING:  Header for TimeBd was zero.";
     cout <<"  Using default  " << hex << fgTbdHeader << dec << endl;
   }
   if (fgTbdMask == 0) {
     fgTbdMask = 0xfffff000;
     cout << "WARNING:  Mask for TimeBd was zero.";
     cout <<"  Using default  " << hex << fgTbdMask << dec << endl;
   }
   if (fgTirHeader == 0) {
     fgTirHeader = 0xffdaf000;
     cout << "WARNING:  Header for TIR was zero.";
     cout <<"  Using default  " << hex << fgTirHeader << dec << endl;
   }
   if (fgTirMask == 0) {
     fgTirMask = 0xfffff000;
     cout << "WARNING:  Mask for TIR was zero.";
     cout <<"  Using default  " << hex << fgTirMask << dec << endl;
   }
   if (fgDaqHeader == 0) {
     fgDaqHeader = 0xfdacf000;
     cout << "WARNING:  Header for DAQ flags was zero.";
     cout <<"  Using default  " << hex << fgDaqHeader << endl;
   }
   if (fgDaqMask == 0) {
     fgDaqMask = 0xfffff000;
     cout << "WARNING:  Mask for DAQ flags was zero.";
     cout <<"  Using default  " << hex << fgDaqMask << dec << endl;
   }
   fTiedKeys.clear();
   fNtied = 0;
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
     if (keymap.IsTiedDevice()) AddTiedDevices(keymap);
     vector<string> vkeys = keymap.GetKeys();
     for (vector<string>::iterator is = vkeys.begin(); 
        is != vkeys.end(); is++) {
           string keystr = *is;
           key = AddRawKey(keystr);
           if (key < 0) continue;
           fEvPointer[key] = keymap.GetEvOffset(keystr);
           fCrate[key] = keymap.GetCrate();
           fReadOut[key] = -1;
           fIsUsed[key] = 1;
           if (keymap.IsAdc(keystr)) fReadOut[key] = ADCREADOUT;
           if (keymap.IsScaler(keystr)) fReadOut[key] = SCALREADOUT;
           if (keymap.IsScaler(keystr)) fReadOut[key] = SCALREADOUT;
           if (keymap.IsTimeboard(keystr)) fReadOut[key] = TBDREADOUT;
           if (keymap.IsTir(keystr)) fReadOut[key] = TIRREADOUT;
           if (keymap.IsDaqFlag(keystr)) fReadOut[key] = DAQFLAG;
           fDevNum[key]  = keymap.GetDevNum(keystr);
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
      if (isrotate) cout << "  BPM "<<devicename<<" is rotated"<<endl;
   }
// Tie derived devices, if any, to fundamental types.
// (e.g. derived device 'bpm' can be tied to an 'adc').
   if (DECODE_DEBUG) cout << "Number tied "<<fNtied<<endl; 
   if (fNtied > 0) {
     for (i = 0; i < fNtied; i++) {
       vector<string> vtiedkey = fTiedKeys[i].GetKeys();
       k = 0;
       db.DataMapReStart();
       while ( db.NextDataMap() ) {
         string devicename = db.GetDataMapName();  
         keymap = db.GetKeyMap(devicename);
         vector<string> vkeys = keymap.GetKeys();
         for (vector<string>::iterator is = vkeys.begin(); 
           is != vkeys.end(); is++) {
           if ((unsigned int)k >= vtiedkey.size()) break;
           string keystr = *is;
           key = GetKey(keystr); 
           tiedkey = GetKey(vtiedkey[k]);
           if ((key != tiedkey) && (key > 0 && tiedkey > 0) &&
               (keymap.GetDevNum(keystr) == 
                fTiedKeys[i].GetDevNum(vtiedkey[k])) &&
               (keymap.GetChan(keystr) == 
                fTiedKeys[i].GetChan(vtiedkey[k])) &&
               (keymap.GetReadOut(keystr) == 
                fTiedKeys[i].GetReadOut(vtiedkey[k]))) {       
             k++;
             db.DataMapReStart();
             fCrate[tiedkey] = fCrate[key];
             fEvPointer[tiedkey] = fEvPointer[key];
             if (DECODE_DEBUG) {
	      cout << "Tying key `"<< GetKey(tiedkey);
              cout << "' to key `" << keystr<<"'"<<endl;
              cout << "crate = " << dec << fCrate[tiedkey];
              cout << "    evptr = " << fEvPointer[tiedkey]<<endl;
	     }
           }
	 }
       }
// If this happens you presumably have an error in the datamap 
// e.g. not all the fundamental (raw) channels are defined to which you 
// wanted to tie a derived type.  See ./doc/DATABASE.TXT
       if (k != (long)vtiedkey.size()) {
         cout << "TaDevice:: ERROR: Unable to tie all derived devices ?"<<endl;
         cout << "This means you have an error in datamap of database."<<endl;
         cout << "(A typo in the line of the thing it is tied to ?)"<<endl;
         cout << "Here is printout of the KeyMap that did not get tied:"<<endl;
         fTiedKeys[i].Print();
       }
     }
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
}

void TaDevice::AddTiedDevices(TaKeyMap& keymap) {
// To potentially add this device to the list of derived types. 
// Note, e.g. "bpm" can be tied to an "adc", but not vice versa.
  if (keymap.GetType() == "adc" || keymap.GetType() == "scaler") {
    cout << "TaDevice::ERROR:  Attempting to tie a raw device ";
    cout << "to something else."<<endl;
    cout << "This is an error in datamap of db file.   ";
    cout << "See ./doc/DATABASE.TXT"<<endl;
    cout << "To help find error, here is a printout" << endl;
    keymap.Print();
    return;
  }
  fTiedKeys.push_back(keymap);
  fNtied++;
  return;
};


void TaDevice::FindHeaders(const Int_t& roc, 
      const Int_t& ipt, const Int_t& data) {
// Find the pointer to the header for the various devices.
// The "fundamental" devices are ADC, SCALER, TIMEBOARD, TIR.
// roc==0 is an error and probably means CODA is not set up 
// correctly. Although roc==0 is logically possible in CODA
// it is forbidden because of database conventions.
  if (roc <= 0 || roc > MAXROC) {
    cout << "TaDevice::FindHeaders::ERROR:  ";
    cout << "illegal value of roc "<<roc<<endl;
    return;
  }
  if ((data & fgAdcMask) == fgAdcHeader) {
    fAdcptr[roc] = ipt;
    return;
  }     
  if ((data & fgScalMask) == fgScalHeader) {
    fScalptr[roc] = ipt;
    return;
  }     
  if ((data & fgTbdMask) == fgTbdHeader) {
    fTbdptr[roc] = ipt;
    return;
  }     
  if ((data & fgTirMask) == fgTirHeader) {
    fTirptr[roc] = ipt;
    return;
  }     
  if ((data & fgDaqMask) == fgDaqHeader) {
    fDaqptr[roc] = ipt;
    return;
  }     
  return;
}


void TaDevice::PrintHeaders() {
//  Printout for debugging multiroc decoding.
  cout << "Headers for adc, scaler, timebd, tir, daqflag "<<hex;
  cout << fgAdcHeader<<"   "<<fgScalHeader<<"   ";
  cout << fgTbdHeader<<"   "<<fgTirHeader;
  cout << "   " << fgDaqHeader << endl;
  cout << "Masks for adc, scaler, timebd, tir, daqflag "<<hex;
  cout << fgAdcMask<<"   "<<fgScalMask<<"   ";
  cout << fgTbdMask<<"   "<<fgTirMask;
  cout << "    "<<fgDaqMask<<endl;
  cout << "Pointers to data : "<<dec<<endl;
  for (Int_t iroc = 1; iroc < MAXROC; iroc++) {
   if (fAdcptr[iroc] > 0) {
       cout << "roc "<<iroc << "  ADCptr= "<<fAdcptr[iroc]<<endl;
   }
   if (fScalptr[iroc] > 0) {
       cout << "roc "<<iroc << "  Scaler ptr= "<<fScalptr[iroc]<<endl;
   }
   if (fTbdptr[iroc] > 0) {
       cout << "roc "<<iroc << "  Tbdptr= "<<fTbdptr[iroc]<<endl;
   }
   if (fTirptr[iroc] > 0) {
       cout << "roc "<<iroc << "  Tirptr= "<<fTirptr[iroc]<<endl;
   }
   if (fDaqptr[iroc] > 0) {
       cout << "roc "<<iroc << "  Daqptr= "<<fDaqptr[iroc]<<endl;
   }
  }
}


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
   fRotateList.push_back("bpm0L01");  
   fRotateList.push_back("bpm0L02");  
   fRotateList.push_back("bpm0L03");  
   fRotateList.push_back("bpm0L04");  
   fRotateList.push_back("bpm0L05");  
   fRotateList.push_back("bpm0L06");  
   fRotateList.push_back("bpm8");
   fRotateList.push_back("bpm10");
   fRotateList.push_back("bpm12");
   fRotateList.push_back("bpm1");
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
   fCrate[key] = 0;
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

Int_t TaDevice::GetCorrIndex(const Int_t& key) const {
  // Return a pointer to the corrected ADC or SCALER data 
  //  (before pedestal subtraction) corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ADCDACSUBOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = SCACLKDIVOFF + 32*GetDevNum(key) + GetChanNum(key);
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
// define the key in DevTypes.hh (genDevTypes.pl)

// BPM 8  raw antenna (xp, xm, yp, ym) and calibrated data (x, y)
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"bpm8xp", IBPM8XP));
  fKeyToIdx.insert(make_pair((string)"bpm8xm", IBPM8XM));
  fKeyToIdx.insert(make_pair((string)"bpm8yp", IBPM8YP));
  fKeyToIdx.insert(make_pair((string)"bpm8ym", IBPM8YM));
  fKeyToIdx.insert(make_pair((string)"bpm8xpc",IBPM8XPC));
  fKeyToIdx.insert(make_pair((string)"bpm8xmc",IBPM8XMC));
  fKeyToIdx.insert(make_pair((string)"bpm8ypc",IBPM8YPC));
  fKeyToIdx.insert(make_pair((string)"bpm8ymc",IBPM8YMC));
  fKeyToIdx.insert(make_pair((string)"bpm8x",  IBPM8X));
  fKeyToIdx.insert(make_pair((string)"bpm8y",  IBPM8Y));
  fKeyToIdx.insert(make_pair((string)"bpm8xws",IBPM8XWS));
  fKeyToIdx.insert(make_pair((string)"bpm8yws",IBPM8YWS));
  fKeyToIdx.insert(make_pair((string)"bpm8ws", IBPM8WS));
  fKeyToIdx.insert(make_pair((string)"bpm8mx", IBPM8MX));

// BPM 10
  fKeyToIdx.insert(make_pair((string)"bpm10xp", IBPM10XP));
  fKeyToIdx.insert(make_pair((string)"bpm10xm", IBPM10XM));
  fKeyToIdx.insert(make_pair((string)"bpm10yp", IBPM10YP));
  fKeyToIdx.insert(make_pair((string)"bpm10ym", IBPM10YM));
  fKeyToIdx.insert(make_pair((string)"bpm10xpc",IBPM10XPC));
  fKeyToIdx.insert(make_pair((string)"bpm10xmc",IBPM10XMC));
  fKeyToIdx.insert(make_pair((string)"bpm10ypc",IBPM10YPC));
  fKeyToIdx.insert(make_pair((string)"bpm10ymc",IBPM10YMC));
  fKeyToIdx.insert(make_pair((string)"bpm10x",  IBPM10X));
  fKeyToIdx.insert(make_pair((string)"bpm10y",  IBPM10Y));
  fKeyToIdx.insert(make_pair((string)"bpm10xws",IBPM10XWS));
  fKeyToIdx.insert(make_pair((string)"bpm10yws",IBPM10YWS));
  fKeyToIdx.insert(make_pair((string)"bpm10ws", IBPM10WS));
  fKeyToIdx.insert(make_pair((string)"bpm10mx", IBPM10MX));

// BPM 12
  fKeyToIdx.insert(make_pair((string)"bpm12xp", IBPM12XP));
  fKeyToIdx.insert(make_pair((string)"bpm12xm", IBPM12XM));
  fKeyToIdx.insert(make_pair((string)"bpm12yp", IBPM12YP));
  fKeyToIdx.insert(make_pair((string)"bpm12ym", IBPM12YM));
  fKeyToIdx.insert(make_pair((string)"bpm12xpc",IBPM12XPC));
  fKeyToIdx.insert(make_pair((string)"bpm12xmc",IBPM12XMC));
  fKeyToIdx.insert(make_pair((string)"bpm12ypc",IBPM12YPC));
  fKeyToIdx.insert(make_pair((string)"bpm12ymc",IBPM12YMC));
  fKeyToIdx.insert(make_pair((string)"bpm12x",  IBPM12X));
  fKeyToIdx.insert(make_pair((string)"bpm12y",  IBPM12Y));
  fKeyToIdx.insert(make_pair((string)"bpm12xws",IBPM12XWS));
  fKeyToIdx.insert(make_pair((string)"bpm12yws",IBPM12YWS));
  fKeyToIdx.insert(make_pair((string)"bpm12ws", IBPM12WS));
  fKeyToIdx.insert(make_pair((string)"bpm12mx", IBPM12MX));

// BPM 1
  fKeyToIdx.insert(make_pair((string)"bpm1xp", IBPM1XP));
  fKeyToIdx.insert(make_pair((string)"bpm1xm", IBPM1XM));
  fKeyToIdx.insert(make_pair((string)"bpm1yp", IBPM1YP));
  fKeyToIdx.insert(make_pair((string)"bpm1ym", IBPM1YM));
  fKeyToIdx.insert(make_pair((string)"bpm1xpc",IBPM1XPC));
  fKeyToIdx.insert(make_pair((string)"bpm1xmc",IBPM1XMC));
  fKeyToIdx.insert(make_pair((string)"bpm1ypc",IBPM1YPC));
  fKeyToIdx.insert(make_pair((string)"bpm1ymc",IBPM1YMC));
  fKeyToIdx.insert(make_pair((string)"bpm1x",  IBPM1X));
  fKeyToIdx.insert(make_pair((string)"bpm1y",  IBPM1Y));
  fKeyToIdx.insert(make_pair((string)"bpm1xws",IBPM1XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1yws",IBPM1YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1ws", IBPM1WS));
  fKeyToIdx.insert(make_pair((string)"bpm1mx", IBPM1MX));

// BPM 4A
  fKeyToIdx.insert(make_pair((string)"bpm4axp", IBPM4AXP));
  fKeyToIdx.insert(make_pair((string)"bpm4axm", IBPM4AXM));
  fKeyToIdx.insert(make_pair((string)"bpm4ayp", IBPM4AYP));
  fKeyToIdx.insert(make_pair((string)"bpm4aym", IBPM4AYM));
  fKeyToIdx.insert(make_pair((string)"bpm4axpc",IBPM4AXPC));
  fKeyToIdx.insert(make_pair((string)"bpm4axmc",IBPM4AXMC));
  fKeyToIdx.insert(make_pair((string)"bpm4aypc",IBPM4AYPC));
  fKeyToIdx.insert(make_pair((string)"bpm4aymc",IBPM4AYMC));
  fKeyToIdx.insert(make_pair((string)"bpm4ax",  IBPM4AX));
  fKeyToIdx.insert(make_pair((string)"bpm4ay",  IBPM4AY));
  fKeyToIdx.insert(make_pair((string)"bpm4axws",IBPM4AXWS));
  fKeyToIdx.insert(make_pair((string)"bpm4ayws",IBPM4AYWS));
  fKeyToIdx.insert(make_pair((string)"bpm4aws", IBPM4AWS));
  fKeyToIdx.insert(make_pair((string)"bpm4amx", IBPM4AMX));

// BPM 4B
  fKeyToIdx.insert(make_pair((string)"bpm4bxp", IBPM4BXP));
  fKeyToIdx.insert(make_pair((string)"bpm4bxm", IBPM4BXM));
  fKeyToIdx.insert(make_pair((string)"bpm4byp", IBPM4BYP));
  fKeyToIdx.insert(make_pair((string)"bpm4bym", IBPM4BYM));
  fKeyToIdx.insert(make_pair((string)"bpm4bxpc",IBPM4BXPC));
  fKeyToIdx.insert(make_pair((string)"bpm4bxmc",IBPM4BXMC));
  fKeyToIdx.insert(make_pair((string)"bpm4bypc",IBPM4BYPC));
  fKeyToIdx.insert(make_pair((string)"bpm4bymc",IBPM4BYMC));
  fKeyToIdx.insert(make_pair((string)"bpm4bx",  IBPM4BX));
  fKeyToIdx.insert(make_pair((string)"bpm4by",  IBPM4BY));
  fKeyToIdx.insert(make_pair((string)"bpm4bxws",IBPM4BXWS));
  fKeyToIdx.insert(make_pair((string)"bpm4byws",IBPM4BYWS));
  fKeyToIdx.insert(make_pair((string)"bpm4bws", IBPM4BWS));
  fKeyToIdx.insert(make_pair((string)"bpm4bmx", IBPM4BMX));

// Injector striplines
  fKeyToIdx.insert(make_pair((string)"bpm1I02xp", IBPM1I02XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xm", IBPM1I02XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I02yp", IBPM1I02YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ym", IBPM1I02YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xpc",IBPM1I02XPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xmc",IBPM1I02XMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ypc",IBPM1I02YPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ymc",IBPM1I02YMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I02x",  IBPM1I02X));
  fKeyToIdx.insert(make_pair((string)"bpm1I02y",  IBPM1I02Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I02xws",IBPM1I02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I02yws",IBPM1I02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I02ws", IBPM1I02WS));
  fKeyToIdx.insert(make_pair((string)"bpm1I02mx", IBPM1I02MX));

  fKeyToIdx.insert(make_pair((string)"bpm1I04xp", IBPM1I04XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xm", IBPM1I04XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I04yp", IBPM1I04YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ym", IBPM1I04YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xpc",IBPM1I04XPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xmc",IBPM1I04XMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ypc",IBPM1I04YPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ymc",IBPM1I04YMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I04x",  IBPM1I04X));
  fKeyToIdx.insert(make_pair((string)"bpm1I04y",  IBPM1I04Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I04xws",IBPM1I04XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I04yws",IBPM1I04YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I04ws", IBPM1I04WS));
  fKeyToIdx.insert(make_pair((string)"bpm1I04mx", IBPM1I04MX));

  fKeyToIdx.insert(make_pair((string)"bpm1I06xp", IBPM1I06XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xp", IBPM1I06XP));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xm", IBPM1I06XM));
  fKeyToIdx.insert(make_pair((string)"bpm1I06yp", IBPM1I06YP));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ym", IBPM1I06YM));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xpc",IBPM1I06XPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xmc",IBPM1I06XMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ypc",IBPM1I06YPC));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ymc",IBPM1I06YMC));
  fKeyToIdx.insert(make_pair((string)"bpm1I06x",  IBPM1I06X));
  fKeyToIdx.insert(make_pair((string)"bpm1I06y",  IBPM1I06Y));
  fKeyToIdx.insert(make_pair((string)"bpm1I06xws",IBPM1I06XWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I06yws",IBPM1I06YWS));
  fKeyToIdx.insert(make_pair((string)"bpm1I06ws", IBPM1I06WS));
  fKeyToIdx.insert(make_pair((string)"bpm1I06mx", IBPM1I06MX));

  fKeyToIdx.insert(make_pair((string)"bpm0I02xp", IBPM0I02XP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xm", IBPM0I02XM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02yp", IBPM0I02YP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ym", IBPM0I02YM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xpc",IBPM0I02XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xmc",IBPM0I02XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ypc",IBPM0I02YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ymc",IBPM0I02YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02x",  IBPM0I02X));
  fKeyToIdx.insert(make_pair((string)"bpm0I02y",  IBPM0I02Y));
  fKeyToIdx.insert(make_pair((string)"bpm0I02xws",IBPM0I02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02yws",IBPM0I02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02ws", IBPM0I02WS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02mx", IBPM0I02MX));

  fKeyToIdx.insert(make_pair((string)"bpm0I02Axp", IBPM0I02AXP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axm", IBPM0I02AXM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ayp", IBPM0I02AYP));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aym", IBPM0I02AYM));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axpc",IBPM0I02AXPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axmc",IBPM0I02AXMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aypc",IBPM0I02AYPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aymc",IBPM0I02AYMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ax",  IBPM0I02AX));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ay",  IBPM0I02AY));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Axws",IBPM0I02AXWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Ayws",IBPM0I02AYWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Aws", IBPM0I02AWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I02Amx", IBPM0I02AMX));

  fKeyToIdx.insert(make_pair((string)"bpm0I05xp", IBPM0I05XP));
  fKeyToIdx.insert(make_pair((string)"bpm0I05xm", IBPM0I05XM));
  fKeyToIdx.insert(make_pair((string)"bpm0I05yp", IBPM0I05YP));
  fKeyToIdx.insert(make_pair((string)"bpm0I05ym", IBPM0I05YM));
  fKeyToIdx.insert(make_pair((string)"bpm0I05xpc",IBPM0I05XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I05xmc",IBPM0I05XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I05ypc",IBPM0I05YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0I05ymc",IBPM0I05YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0I05x",  IBPM0I05X));
  fKeyToIdx.insert(make_pair((string)"bpm0I05y",  IBPM0I05Y));
  fKeyToIdx.insert(make_pair((string)"bpm0I05xws",IBPM0I05XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I05yws",IBPM0I05YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0I05ws", IBPM0I05WS));
  fKeyToIdx.insert(make_pair((string)"bpm0I05mx", IBPM0I05MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L01xp", IBPM0L01XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L01xm", IBPM0L01XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L01yp", IBPM0L01YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L01ym", IBPM0L01YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L01xpc",IBPM0L01XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L01xmc",IBPM0L01XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L01ypc",IBPM0L01YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L01ymc",IBPM0L01YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L01x",  IBPM0L01X));
  fKeyToIdx.insert(make_pair((string)"bpm0L01y",  IBPM0L01Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L01xws",IBPM0L01XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L01yws",IBPM0L01YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L01ws", IBPM0L01WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L01mx", IBPM0L01MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L02xp", IBPM0L02XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xm", IBPM0L02XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L02yp", IBPM0L02YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ym", IBPM0L02YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xpc",IBPM0L02XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xmc",IBPM0L02XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ypc",IBPM0L02YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ymc",IBPM0L02YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L02x",  IBPM0L02X));
  fKeyToIdx.insert(make_pair((string)"bpm0L02y",  IBPM0L02Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L02xws",IBPM0L02XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L02yws",IBPM0L02YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L02ws", IBPM0L02WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L02mx", IBPM0L02MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L03xp", IBPM0L03XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L03xm", IBPM0L03XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L03yp", IBPM0L03YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L03ym", IBPM0L03YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L03xpc",IBPM0L01XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L03xmc",IBPM0L01XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L03ypc",IBPM0L01YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L03ymc",IBPM0L01YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L03x",  IBPM0L03X));
  fKeyToIdx.insert(make_pair((string)"bpm0L03y",  IBPM0L03Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L03xws",IBPM0L03XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L03yws",IBPM0L03YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L03ws", IBPM0L03WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L03mx", IBPM0L03MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L04xp", IBPM0L04XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L04xm", IBPM0L04XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L04yp", IBPM0L04YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L04ym", IBPM0L04YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L04xpc",IBPM0L04XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L04xmc",IBPM0L04XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L04ypc",IBPM0L04YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L04ymc",IBPM0L04YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L04x",  IBPM0L04X));
  fKeyToIdx.insert(make_pair((string)"bpm0L04y",  IBPM0L04Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L04xws",IBPM0L04XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L04yws",IBPM0L04YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L04ws", IBPM0L04WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L04mx", IBPM0L04MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L05xp", IBPM0L05XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L05xm", IBPM0L05XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L05yp", IBPM0L05YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L05ym", IBPM0L05YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L05xpc",IBPM0L05XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L05xmc",IBPM0L05XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L05ypc",IBPM0L05YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L05ymc",IBPM0L05YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L05x",  IBPM0L05X));
  fKeyToIdx.insert(make_pair((string)"bpm0L05y",  IBPM0L05Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L05xws",IBPM0L05XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L05yws",IBPM0L05YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L05ws", IBPM0L05WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L05mx", IBPM0L05MX));

  fKeyToIdx.insert(make_pair((string)"bpm0L06xp", IBPM0L06XP));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xm", IBPM0L06XM));
  fKeyToIdx.insert(make_pair((string)"bpm0L06yp", IBPM0L06YP));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ym", IBPM0L06YM));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xpc",IBPM0L06XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xmc",IBPM0L06XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ypc",IBPM0L06YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ymc",IBPM0L06YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0L06x",  IBPM0L06X));
  fKeyToIdx.insert(make_pair((string)"bpm0L06y",  IBPM0L06Y));
  fKeyToIdx.insert(make_pair((string)"bpm0L06xws",IBPM0L06XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L06yws",IBPM0L06YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0L06ws", IBPM0L06WS));
  fKeyToIdx.insert(make_pair((string)"bpm0L06mx", IBPM0L06MX));

// G0 cavity monitors, raw data ("r") and calibrated 
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"bpmcav1xr",IBPMCAV1XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav1yr",IBPMCAV1YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav1xc",IBPMCAV1XC));
  fKeyToIdx.insert(make_pair((string)"bpmcav1yc",IBPMCAV1YC));
  fKeyToIdx.insert(make_pair((string)"bpmcav1x", IBPMCAV1X));
  fKeyToIdx.insert(make_pair((string)"bpmcav1y", IBPMCAV1Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav2xr",IBPMCAV2XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav2yr",IBPMCAV2YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav2xc",IBPMCAV2XC));
  fKeyToIdx.insert(make_pair((string)"bpmcav2yc",IBPMCAV2YC));
  fKeyToIdx.insert(make_pair((string)"bpmcav2x", IBPMCAV2X));
  fKeyToIdx.insert(make_pair((string)"bpmcav2y", IBPMCAV2Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav3xr",IBPMCAV3XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav3yr",IBPMCAV3YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav3xc",IBPMCAV3XC));
  fKeyToIdx.insert(make_pair((string)"bpmcav3yc",IBPMCAV3YC));
  fKeyToIdx.insert(make_pair((string)"bpmcav3x", IBPMCAV3X));
  fKeyToIdx.insert(make_pair((string)"bpmcav3y", IBPMCAV3Y));

  fKeyToIdx.insert(make_pair((string)"bpmcav4xr",IBPMCAV4XR));
  fKeyToIdx.insert(make_pair((string)"bpmcav4yr",IBPMCAV4YR));
  fKeyToIdx.insert(make_pair((string)"bpmcav4xc",IBPMCAV4XC));
  fKeyToIdx.insert(make_pair((string)"bpmcav4yc",IBPMCAV4YC));
  fKeyToIdx.insert(make_pair((string)"bpmcav4x", IBPMCAV4X));
  fKeyToIdx.insert(make_pair((string)"bpmcav4y", IBPMCAV4Y));

// Old cavity monitors (Happex1 era BCMs).  
// Raw ("r") and calibrated
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"bcm1r", IBCM1R));
  fKeyToIdx.insert(make_pair((string)"bcm1c", IBCM1C));
  fKeyToIdx.insert(make_pair((string)"bcm1",  IBCM1));

  fKeyToIdx.insert(make_pair((string)"bcm2r", IBCM2R));
  fKeyToIdx.insert(make_pair((string)"bcm2c", IBCM2C));
  fKeyToIdx.insert(make_pair((string)"bcm2",  IBCM2));

  fKeyToIdx.insert(make_pair((string)"bcm3r", IBCM3R));
  fKeyToIdx.insert(make_pair((string)"bcm3c", IBCM3C));
  fKeyToIdx.insert(make_pair((string)"bcm3" , IBCM3));

  fKeyToIdx.insert(make_pair((string)"bcm4r", IBCM4R));
  fKeyToIdx.insert(make_pair((string)"bcm4c", IBCM4C));
  fKeyToIdx.insert(make_pair((string)"bcm4" , IBCM4));

  fKeyToIdx.insert(make_pair((string)"bcm5r", IBCM5R));
  fKeyToIdx.insert(make_pair((string)"bcm5c", IBCM5C));
  fKeyToIdx.insert(make_pair((string)"bcm5" , IBCM5));

  fKeyToIdx.insert(make_pair((string)"bcm6r", IBCM6R));
  fKeyToIdx.insert(make_pair((string)"bcm6c", IBCM6C));
  fKeyToIdx.insert(make_pair((string)"bcm6" , IBCM6));

  fKeyToIdx.insert(make_pair((string)"bcm7r", IBCM7R));
  fKeyToIdx.insert(make_pair((string)"bcm7c", IBCM7C));
  fKeyToIdx.insert(make_pair((string)"bcm7" , IBCM7));

  fKeyToIdx.insert(make_pair((string)"bcm8r", IBCM8R));
  fKeyToIdx.insert(make_pair((string)"bcm8c", IBCM8C));
  fKeyToIdx.insert(make_pair((string)"bcm8" , IBCM8));

  fKeyToIdx.insert(make_pair((string)"bcm9r", IBCM9R));
  fKeyToIdx.insert(make_pair((string)"bcm9c", IBCM9C));
  fKeyToIdx.insert(make_pair((string)"bcm9" , IBCM9));

  fKeyToIdx.insert(make_pair((string)"bcm10r", IBCM10R));
  fKeyToIdx.insert(make_pair((string)"bcm10c", IBCM10C));
  fKeyToIdx.insert(make_pair((string)"bcm10" , IBCM10));

// G0 cavity BCM1  raw data ("r") and calibrated data
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"bcmcav1r",IBCMCAV1R));
  fKeyToIdx.insert(make_pair((string)"bcmcav1c",IBCMCAV1C));
  fKeyToIdx.insert(make_pair((string)"bcmcav1", IBCMCAV1));

  fKeyToIdx.insert(make_pair((string)"bcmcav2r", IBCMCAV2R));
  fKeyToIdx.insert(make_pair((string)"bcmcav2c", IBCMCAV2C));
  fKeyToIdx.insert(make_pair((string)"bcmcav2",  IBCMCAV2));

  fKeyToIdx.insert(make_pair((string)"bcmcav3r",IBCMCAV3R));
  fKeyToIdx.insert(make_pair((string)"bcmcav3c",IBCMCAV3C));
  fKeyToIdx.insert(make_pair((string)"bcmcav3",  IBCMCAV3));

  fKeyToIdx.insert(make_pair((string)"bcmcav4r",IBCMCAV4R));
  fKeyToIdx.insert(make_pair((string)"bcmcav4c",IBCMCAV4C));
  fKeyToIdx.insert(make_pair((string)"bcmcav4",  IBCMCAV4));

// Batteries
  fKeyToIdx.insert(make_pair((string)"batt1", IBATT1));
  fKeyToIdx.insert(make_pair((string)"batt2", IBATT2));
  fKeyToIdx.insert(make_pair((string)"batt3", IBATT3));
  fKeyToIdx.insert(make_pair((string)"batt4", IBATT4));
  fKeyToIdx.insert(make_pair((string)"batt5", IBATT5));
  fKeyToIdx.insert(make_pair((string)"batt6", IBATT6));
  fKeyToIdx.insert(make_pair((string)"batt7", IBATT7));
  fKeyToIdx.insert(make_pair((string)"batt8", IBATT8));
        
// Detectors, raw data ("r") and calibrated data
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"det1r", IDET1R));
  fKeyToIdx.insert(make_pair((string)"det1c", IDET1C));
  fKeyToIdx.insert(make_pair((string)"det1",  IDET1));
  fKeyToIdx.insert(make_pair((string)"det1s", IDET1S));

  fKeyToIdx.insert(make_pair((string)"det2r", IDET2R));
  fKeyToIdx.insert(make_pair((string)"det2c", IDET2C));
  fKeyToIdx.insert(make_pair((string)"det2",  IDET2));
  fKeyToIdx.insert(make_pair((string)"det2s", IDET2S));

  fKeyToIdx.insert(make_pair((string)"det3r", IDET3R));
  fKeyToIdx.insert(make_pair((string)"det3c", IDET3C));
  fKeyToIdx.insert(make_pair((string)"det3",  IDET3));
  fKeyToIdx.insert(make_pair((string)"det3s", IDET3S));

  fKeyToIdx.insert(make_pair((string)"det4r", IDET4R));
  fKeyToIdx.insert(make_pair((string)"det4c", IDET4C));
  fKeyToIdx.insert(make_pair((string)"det4",  IDET4));
  fKeyToIdx.insert(make_pair((string)"det4s", IDET4S));

// UMass Profile scanner (signal, X, Y, and
//       control voltages v1, v2, v3) 
// raw data (suffix "r") and calibrated.
// "c" = data before pedestal subtracted
// Prefix "l"=left spectrometer, "r"=right
  fKeyToIdx.insert(make_pair((string)"rprofr",IRPROFR));
  fKeyToIdx.insert(make_pair((string)"rprofxr",IRPROFXR));
  fKeyToIdx.insert(make_pair((string)"rprofyr",IRPROFYR));
  fKeyToIdx.insert(make_pair((string)"rprofv1r",IRPROFV1R));
  fKeyToIdx.insert(make_pair((string)"rprofv2r",IRPROFV2R));
  fKeyToIdx.insert(make_pair((string)"rprofv3r",IRPROFV3R));
  fKeyToIdx.insert(make_pair((string)"rprofc",IRPROFC));
  fKeyToIdx.insert(make_pair((string)"rprofxc",IRPROFXC));
  fKeyToIdx.insert(make_pair((string)"rprofyc",IRPROFYC));
  fKeyToIdx.insert(make_pair((string)"rprofv1c",IRPROFV1C));
  fKeyToIdx.insert(make_pair((string)"rprofv2c",IRPROFV2C));
  fKeyToIdx.insert(make_pair((string)"rprofv3c",IRPROFV3C));
  fKeyToIdx.insert(make_pair((string)"rprof",IRPROF));
  fKeyToIdx.insert(make_pair((string)"rprofx",IRPROFX));
  fKeyToIdx.insert(make_pair((string)"rprofy",IRPROFY));
  fKeyToIdx.insert(make_pair((string)"rprofv1",IRPROFV1));
  fKeyToIdx.insert(make_pair((string)"rprofv2",IRPROFV2));
  fKeyToIdx.insert(make_pair((string)"rprofv3",IRPROFV3));
  fKeyToIdx.insert(make_pair((string)"lprofr",ILPROFR));
  fKeyToIdx.insert(make_pair((string)"lprofxr",ILPROFXR));
  fKeyToIdx.insert(make_pair((string)"lprofyr",ILPROFYR));
  fKeyToIdx.insert(make_pair((string)"lprofv1r",ILPROFV1R));
  fKeyToIdx.insert(make_pair((string)"lprofv2r",ILPROFV2R));
  fKeyToIdx.insert(make_pair((string)"lprofv3r",ILPROFV3R));
  fKeyToIdx.insert(make_pair((string)"lprofc",ILPROFC));
  fKeyToIdx.insert(make_pair((string)"lprofxc",ILPROFXC));
  fKeyToIdx.insert(make_pair((string)"lprofyc",ILPROFYC));
  fKeyToIdx.insert(make_pair((string)"lprofv1c",ILPROFV1C));
  fKeyToIdx.insert(make_pair((string)"lprofv2c",ILPROFV2C));
  fKeyToIdx.insert(make_pair((string)"lprofv3c",ILPROFV3C));
  fKeyToIdx.insert(make_pair((string)"lprof",ILPROF));
  fKeyToIdx.insert(make_pair((string)"lprofx",ILPROFX));
  fKeyToIdx.insert(make_pair((string)"lprofy",ILPROFY));
  fKeyToIdx.insert(make_pair((string)"lprofv1",ILPROFV1));
  fKeyToIdx.insert(make_pair((string)"lprofv2",ILPROFV2));
  fKeyToIdx.insert(make_pair((string)"lprofv3",ILPROFV3));
  
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
  fKeyToIdx.insert(make_pair((string)"adc23_0",IADC23_0));
  fKeyToIdx.insert(make_pair((string)"adc23_1",IADC23_1));
  fKeyToIdx.insert(make_pair((string)"adc23_2",IADC23_2));
  fKeyToIdx.insert(make_pair((string)"adc23_3",IADC23_3));
  fKeyToIdx.insert(make_pair((string)"adc24_0",IADC24_0));
  fKeyToIdx.insert(make_pair((string)"adc24_1",IADC24_1));
  fKeyToIdx.insert(make_pair((string)"adc24_2",IADC24_2));
  fKeyToIdx.insert(make_pair((string)"adc24_3",IADC24_3));
  fKeyToIdx.insert(make_pair((string)"adc25_0",IADC25_0));
  fKeyToIdx.insert(make_pair((string)"adc25_1",IADC25_1));
  fKeyToIdx.insert(make_pair((string)"adc25_2",IADC25_2));
  fKeyToIdx.insert(make_pair((string)"adc25_3",IADC25_3));
  fKeyToIdx.insert(make_pair((string)"adc26_0",IADC26_0));
  fKeyToIdx.insert(make_pair((string)"adc26_1",IADC26_1));
  fKeyToIdx.insert(make_pair((string)"adc26_2",IADC26_2));
  fKeyToIdx.insert(make_pair((string)"adc26_3",IADC26_3));
  fKeyToIdx.insert(make_pair((string)"adc27_0",IADC27_0));
  fKeyToIdx.insert(make_pair((string)"adc27_1",IADC27_1));
  fKeyToIdx.insert(make_pair((string)"adc27_2",IADC27_2));
  fKeyToIdx.insert(make_pair((string)"adc27_3",IADC27_3));
  fKeyToIdx.insert(make_pair((string)"adc28_0",IADC28_0));
  fKeyToIdx.insert(make_pair((string)"adc28_1",IADC28_1));
  fKeyToIdx.insert(make_pair((string)"adc28_2",IADC28_2));
  fKeyToIdx.insert(make_pair((string)"adc28_3",IADC28_3));
  fKeyToIdx.insert(make_pair((string)"adc29_0",IADC29_0));
  fKeyToIdx.insert(make_pair((string)"adc29_1",IADC29_1));
  fKeyToIdx.insert(make_pair((string)"adc29_2",IADC29_2));
  fKeyToIdx.insert(make_pair((string)"adc29_3",IADC29_3));
  fKeyToIdx.insert(make_pair((string)"adc30_0",IADC30_0));
  fKeyToIdx.insert(make_pair((string)"adc30_1",IADC30_1));
  fKeyToIdx.insert(make_pair((string)"adc30_2",IADC30_2));
  fKeyToIdx.insert(make_pair((string)"adc30_3",IADC30_3));

// These are dacnoise subtracted data (no pedestal subtraction)
  fKeyToIdx.insert(make_pair((string)"adc0_0_dacsub",IADC0_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc0_1_dacsub",IADC0_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc0_2_dacsub",IADC0_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc0_3_dacsub",IADC0_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc1_0_dacsub",IADC1_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc1_1_dacsub",IADC1_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc1_2_dacsub",IADC1_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc1_3_dacsub",IADC1_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc2_0_dacsub",IADC2_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc2_1_dacsub",IADC2_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc2_2_dacsub",IADC2_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc2_3_dacsub",IADC2_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc3_0_dacsub",IADC3_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc3_1_dacsub",IADC3_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc3_2_dacsub",IADC3_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc3_3_dacsub",IADC3_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc4_0_dacsub",IADC4_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc4_1_dacsub",IADC4_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc4_2_dacsub",IADC4_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc4_3_dacsub",IADC4_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc5_0_dacsub",IADC5_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc5_1_dacsub",IADC5_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc5_2_dacsub",IADC5_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc5_3_dacsub",IADC5_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc6_0_dacsub",IADC6_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc6_1_dacsub",IADC6_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc6_2_dacsub",IADC6_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc6_3_dacsub",IADC6_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc7_0_dacsub",IADC7_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc7_1_dacsub",IADC7_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc7_2_dacsub",IADC7_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc7_3_dacsub",IADC7_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc8_0_dacsub",IADC8_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc8_1_dacsub",IADC8_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc8_2_dacsub",IADC8_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc8_3_dacsub",IADC8_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc9_0_dacsub",IADC9_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc9_1_dacsub",IADC9_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc9_2_dacsub",IADC9_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc9_3_dacsub",IADC9_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc10_0_dacsub",IADC10_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc10_1_dacsub",IADC10_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc10_2_dacsub",IADC10_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc10_3_dacsub",IADC10_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc11_0_dacsub",IADC11_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc11_1_dacsub",IADC11_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc11_2_dacsub",IADC11_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc11_3_dacsub",IADC11_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc12_0_dacsub",IADC12_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc12_1_dacsub",IADC12_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc12_2_dacsub",IADC12_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc12_3_dacsub",IADC12_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc13_0_dacsub",IADC13_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc13_1_dacsub",IADC13_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc13_2_dacsub",IADC13_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc13_3_dacsub",IADC13_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc14_0_dacsub",IADC14_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc14_1_dacsub",IADC14_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc14_2_dacsub",IADC14_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc14_3_dacsub",IADC14_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc15_0_dacsub",IADC15_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc15_1_dacsub",IADC15_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc15_2_dacsub",IADC15_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc15_3_dacsub",IADC15_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc16_0_dacsub",IADC16_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc16_1_dacsub",IADC16_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc16_2_dacsub",IADC16_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc16_3_dacsub",IADC16_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc17_0_dacsub",IADC17_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc17_1_dacsub",IADC17_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc17_2_dacsub",IADC17_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc17_3_dacsub",IADC17_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc18_0_dacsub",IADC18_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc18_1_dacsub",IADC18_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc18_2_dacsub",IADC18_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc18_3_dacsub",IADC18_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc19_0_dacsub",IADC19_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc19_1_dacsub",IADC19_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc19_2_dacsub",IADC19_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc19_3_dacsub",IADC19_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc20_0_dacsub",IADC20_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc20_1_dacsub",IADC20_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc20_2_dacsub",IADC20_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc20_3_dacsub",IADC20_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc21_0_dacsub",IADC21_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc21_1_dacsub",IADC21_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc21_2_dacsub",IADC21_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc21_3_dacsub",IADC21_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc22_0_dacsub",IADC22_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc22_1_dacsub",IADC22_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc22_2_dacsub",IADC22_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc22_3_dacsub",IADC22_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc23_0_dacsub",IADC23_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc23_1_dacsub",IADC23_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc23_2_dacsub",IADC23_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc23_3_dacsub",IADC23_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc24_0_dacsub",IADC24_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc24_1_dacsub",IADC24_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc24_2_dacsub",IADC24_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc24_3_dacsub",IADC24_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc25_0_dacsub",IADC25_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc25_1_dacsub",IADC25_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc25_2_dacsub",IADC25_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc25_3_dacsub",IADC25_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc26_0_dacsub",IADC26_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc26_1_dacsub",IADC26_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc26_2_dacsub",IADC26_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc26_3_dacsub",IADC26_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc27_0_dacsub",IADC27_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc27_1_dacsub",IADC27_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc27_2_dacsub",IADC27_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc27_3_dacsub",IADC27_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc28_0_dacsub",IADC28_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc28_1_dacsub",IADC28_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc28_2_dacsub",IADC28_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc28_3_dacsub",IADC28_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc29_0_dacsub",IADC29_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc29_1_dacsub",IADC29_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc29_2_dacsub",IADC29_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc29_3_dacsub",IADC29_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc30_0_dacsub",IADC30_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc30_1_dacsub",IADC30_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc30_2_dacsub",IADC30_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adc30_3_dacsub",IADC30_3_DACSUB));

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
  fKeyToIdx.insert(make_pair((string)"adc23_0_cal",IADC23_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc23_1_cal",IADC23_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc23_2_cal",IADC23_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc23_3_cal",IADC23_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc24_0_cal",IADC24_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc24_1_cal",IADC24_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc24_2_cal",IADC24_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc24_3_cal",IADC24_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc25_0_cal",IADC25_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc25_1_cal",IADC25_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc25_2_cal",IADC25_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc25_3_cal",IADC25_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc26_0_cal",IADC26_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc26_1_cal",IADC26_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc26_2_cal",IADC26_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc26_3_cal",IADC26_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc27_0_cal",IADC27_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc27_1_cal",IADC27_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc27_2_cal",IADC27_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc27_3_cal",IADC27_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc28_0_cal",IADC28_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc28_1_cal",IADC28_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc28_2_cal",IADC28_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc28_3_cal",IADC28_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc29_0_cal",IADC29_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc29_1_cal",IADC29_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc29_2_cal",IADC29_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc29_3_cal",IADC29_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adc30_0_cal",IADC30_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adc30_1_cal",IADC30_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adc30_2_cal",IADC30_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adc30_3_cal",IADC30_3_CAL));

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
  fKeyToIdx.insert(make_pair((string)"dac23",IDAC23));
  fKeyToIdx.insert(make_pair((string)"dac24",IDAC24));
  fKeyToIdx.insert(make_pair((string)"dac25",IDAC25));
  fKeyToIdx.insert(make_pair((string)"dac26",IDAC26));
  fKeyToIdx.insert(make_pair((string)"dac27",IDAC27));
  fKeyToIdx.insert(make_pair((string)"dac28",IDAC28));
  fKeyToIdx.insert(make_pair((string)"dac29",IDAC29));
  fKeyToIdx.insert(make_pair((string)"dac30",IDAC30));

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
  fKeyToIdx.insert(make_pair((string)"csr23",ICSR23));
  fKeyToIdx.insert(make_pair((string)"csr24",ICSR24));
  fKeyToIdx.insert(make_pair((string)"csr25",ICSR25));
  fKeyToIdx.insert(make_pair((string)"csr26",ICSR26));
  fKeyToIdx.insert(make_pair((string)"csr27",ICSR27));
  fKeyToIdx.insert(make_pair((string)"csr28",ICSR28));
  fKeyToIdx.insert(make_pair((string)"csr29",ICSR29));
  fKeyToIdx.insert(make_pair((string)"csr30",ICSR30));

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

  fKeyToIdx.insert(make_pair((string)"scaler4_0",ISCALER4_0));
  fKeyToIdx.insert(make_pair((string)"scaler4_1",ISCALER4_1));
  fKeyToIdx.insert(make_pair((string)"scaler4_2",ISCALER4_2));
  fKeyToIdx.insert(make_pair((string)"scaler4_3",ISCALER4_3));
  fKeyToIdx.insert(make_pair((string)"scaler4_4",ISCALER4_4));
  fKeyToIdx.insert(make_pair((string)"scaler4_5",ISCALER4_5));
  fKeyToIdx.insert(make_pair((string)"scaler4_6",ISCALER4_6));
  fKeyToIdx.insert(make_pair((string)"scaler4_7",ISCALER4_7));
  fKeyToIdx.insert(make_pair((string)"scaler4_8",ISCALER4_8));
  fKeyToIdx.insert(make_pair((string)"scaler4_9",ISCALER4_9));
  fKeyToIdx.insert(make_pair((string)"scaler4_10",ISCALER4_10));
  fKeyToIdx.insert(make_pair((string)"scaler4_11",ISCALER4_11));
  fKeyToIdx.insert(make_pair((string)"scaler4_12",ISCALER4_12));
  fKeyToIdx.insert(make_pair((string)"scaler4_13",ISCALER4_13));
  fKeyToIdx.insert(make_pair((string)"scaler4_14",ISCALER4_14));
  fKeyToIdx.insert(make_pair((string)"scaler4_15",ISCALER4_15));
  fKeyToIdx.insert(make_pair((string)"scaler4_16",ISCALER4_16));
  fKeyToIdx.insert(make_pair((string)"scaler4_17",ISCALER4_17));
  fKeyToIdx.insert(make_pair((string)"scaler4_18",ISCALER4_18));
  fKeyToIdx.insert(make_pair((string)"scaler4_19",ISCALER4_19));
  fKeyToIdx.insert(make_pair((string)"scaler4_20",ISCALER4_20));
  fKeyToIdx.insert(make_pair((string)"scaler4_21",ISCALER4_21));
  fKeyToIdx.insert(make_pair((string)"scaler4_22",ISCALER4_22));
  fKeyToIdx.insert(make_pair((string)"scaler4_23",ISCALER4_23));
  fKeyToIdx.insert(make_pair((string)"scaler4_24",ISCALER4_24));
  fKeyToIdx.insert(make_pair((string)"scaler4_25",ISCALER4_25));
  fKeyToIdx.insert(make_pair((string)"scaler4_26",ISCALER4_26));
  fKeyToIdx.insert(make_pair((string)"scaler4_27",ISCALER4_27));
  fKeyToIdx.insert(make_pair((string)"scaler4_28",ISCALER4_28));
  fKeyToIdx.insert(make_pair((string)"scaler4_29",ISCALER4_29));
  fKeyToIdx.insert(make_pair((string)"scaler4_30",ISCALER4_30));
  fKeyToIdx.insert(make_pair((string)"scaler4_31",ISCALER4_31));

  fKeyToIdx.insert(make_pair((string)"scaler5_0",ISCALER5_0));
  fKeyToIdx.insert(make_pair((string)"scaler5_1",ISCALER5_1));
  fKeyToIdx.insert(make_pair((string)"scaler5_2",ISCALER5_2));
  fKeyToIdx.insert(make_pair((string)"scaler5_3",ISCALER5_3));
  fKeyToIdx.insert(make_pair((string)"scaler5_4",ISCALER5_4));
  fKeyToIdx.insert(make_pair((string)"scaler5_5",ISCALER5_5));
  fKeyToIdx.insert(make_pair((string)"scaler5_6",ISCALER5_6));
  fKeyToIdx.insert(make_pair((string)"scaler5_7",ISCALER5_7));
  fKeyToIdx.insert(make_pair((string)"scaler5_8",ISCALER5_8));
  fKeyToIdx.insert(make_pair((string)"scaler5_9",ISCALER5_9));
  fKeyToIdx.insert(make_pair((string)"scaler5_10",ISCALER5_10));
  fKeyToIdx.insert(make_pair((string)"scaler5_11",ISCALER5_11));
  fKeyToIdx.insert(make_pair((string)"scaler5_12",ISCALER5_12));
  fKeyToIdx.insert(make_pair((string)"scaler5_13",ISCALER5_13));
  fKeyToIdx.insert(make_pair((string)"scaler5_14",ISCALER5_14));
  fKeyToIdx.insert(make_pair((string)"scaler5_15",ISCALER5_15));
  fKeyToIdx.insert(make_pair((string)"scaler5_16",ISCALER5_16));
  fKeyToIdx.insert(make_pair((string)"scaler5_17",ISCALER5_17));
  fKeyToIdx.insert(make_pair((string)"scaler5_18",ISCALER5_18));
  fKeyToIdx.insert(make_pair((string)"scaler5_19",ISCALER5_19));
  fKeyToIdx.insert(make_pair((string)"scaler5_20",ISCALER5_20));
  fKeyToIdx.insert(make_pair((string)"scaler5_21",ISCALER5_21));
  fKeyToIdx.insert(make_pair((string)"scaler5_22",ISCALER5_22));
  fKeyToIdx.insert(make_pair((string)"scaler5_23",ISCALER5_23));
  fKeyToIdx.insert(make_pair((string)"scaler5_24",ISCALER5_24));
  fKeyToIdx.insert(make_pair((string)"scaler5_25",ISCALER5_25));
  fKeyToIdx.insert(make_pair((string)"scaler5_26",ISCALER5_26));
  fKeyToIdx.insert(make_pair((string)"scaler5_27",ISCALER5_27));
  fKeyToIdx.insert(make_pair((string)"scaler5_28",ISCALER5_28));
  fKeyToIdx.insert(make_pair((string)"scaler5_29",ISCALER5_29));
  fKeyToIdx.insert(make_pair((string)"scaler5_30",ISCALER5_30));
  fKeyToIdx.insert(make_pair((string)"scaler5_31",ISCALER5_31));

  fKeyToIdx.insert(make_pair((string)"scaler6_0",ISCALER6_0));
  fKeyToIdx.insert(make_pair((string)"scaler6_1",ISCALER6_1));
  fKeyToIdx.insert(make_pair((string)"scaler6_2",ISCALER6_2));
  fKeyToIdx.insert(make_pair((string)"scaler6_3",ISCALER6_3));
  fKeyToIdx.insert(make_pair((string)"scaler6_4",ISCALER6_4));
  fKeyToIdx.insert(make_pair((string)"scaler6_5",ISCALER6_5));
  fKeyToIdx.insert(make_pair((string)"scaler6_6",ISCALER6_6));
  fKeyToIdx.insert(make_pair((string)"scaler6_7",ISCALER6_7));
  fKeyToIdx.insert(make_pair((string)"scaler6_8",ISCALER6_8));
  fKeyToIdx.insert(make_pair((string)"scaler6_9",ISCALER6_9));
  fKeyToIdx.insert(make_pair((string)"scaler6_10",ISCALER6_10));
  fKeyToIdx.insert(make_pair((string)"scaler6_11",ISCALER6_11));
  fKeyToIdx.insert(make_pair((string)"scaler6_12",ISCALER6_12));
  fKeyToIdx.insert(make_pair((string)"scaler6_13",ISCALER6_13));
  fKeyToIdx.insert(make_pair((string)"scaler6_14",ISCALER6_14));
  fKeyToIdx.insert(make_pair((string)"scaler6_15",ISCALER6_15));
  fKeyToIdx.insert(make_pair((string)"scaler6_16",ISCALER6_16));
  fKeyToIdx.insert(make_pair((string)"scaler6_17",ISCALER6_17));
  fKeyToIdx.insert(make_pair((string)"scaler6_18",ISCALER6_18));
  fKeyToIdx.insert(make_pair((string)"scaler6_19",ISCALER6_19));
  fKeyToIdx.insert(make_pair((string)"scaler6_20",ISCALER6_20));
  fKeyToIdx.insert(make_pair((string)"scaler6_21",ISCALER6_21));
  fKeyToIdx.insert(make_pair((string)"scaler6_22",ISCALER6_22));
  fKeyToIdx.insert(make_pair((string)"scaler6_23",ISCALER6_23));
  fKeyToIdx.insert(make_pair((string)"scaler6_24",ISCALER6_24));
  fKeyToIdx.insert(make_pair((string)"scaler6_25",ISCALER6_25));
  fKeyToIdx.insert(make_pair((string)"scaler6_26",ISCALER6_26));
  fKeyToIdx.insert(make_pair((string)"scaler6_27",ISCALER6_27));
  fKeyToIdx.insert(make_pair((string)"scaler6_28",ISCALER6_28));
  fKeyToIdx.insert(make_pair((string)"scaler6_29",ISCALER6_29));
  fKeyToIdx.insert(make_pair((string)"scaler6_30",ISCALER6_30));
  fKeyToIdx.insert(make_pair((string)"scaler6_31",ISCALER6_31));

  // Clock Divided Scaler Data for V2F (before ped subtracted)
  fKeyToIdx.insert(make_pair((string)"scaler0_0_clkdiv",ISCALER0_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_1_clkdiv",ISCALER0_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_2_clkdiv",ISCALER0_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_3_clkdiv",ISCALER0_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_4_clkdiv",ISCALER0_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_5_clkdiv",ISCALER0_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_6_clkdiv",ISCALER0_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_7_clkdiv",ISCALER0_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_8_clkdiv",ISCALER0_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_9_clkdiv",ISCALER0_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_10_clkdiv",ISCALER0_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_11_clkdiv",ISCALER0_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_12_clkdiv",ISCALER0_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_13_clkdiv",ISCALER0_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_14_clkdiv",ISCALER0_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_15_clkdiv",ISCALER0_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_16_clkdiv",ISCALER0_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_17_clkdiv",ISCALER0_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_18_clkdiv",ISCALER0_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_19_clkdiv",ISCALER0_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_20_clkdiv",ISCALER0_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_21_clkdiv",ISCALER0_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_22_clkdiv",ISCALER0_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_23_clkdiv",ISCALER0_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_24_clkdiv",ISCALER0_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_25_clkdiv",ISCALER0_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_26_clkdiv",ISCALER0_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_27_clkdiv",ISCALER0_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_28_clkdiv",ISCALER0_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_29_clkdiv",ISCALER0_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_30_clkdiv",ISCALER0_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler0_31_clkdiv",ISCALER0_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler1_0_clkdiv",ISCALER1_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_1_clkdiv",ISCALER1_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_2_clkdiv",ISCALER1_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_3_clkdiv",ISCALER1_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_4_clkdiv",ISCALER1_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_5_clkdiv",ISCALER1_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_6_clkdiv",ISCALER1_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_7_clkdiv",ISCALER1_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_8_clkdiv",ISCALER1_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_9_clkdiv",ISCALER1_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_10_clkdiv",ISCALER1_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_11_clkdiv",ISCALER1_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_12_clkdiv",ISCALER1_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_13_clkdiv",ISCALER1_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_14_clkdiv",ISCALER1_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_15_clkdiv",ISCALER1_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_16_clkdiv",ISCALER1_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_17_clkdiv",ISCALER1_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_18_clkdiv",ISCALER1_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_19_clkdiv",ISCALER1_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_20_clkdiv",ISCALER1_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_21_clkdiv",ISCALER1_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_22_clkdiv",ISCALER1_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_23_clkdiv",ISCALER1_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_24_clkdiv",ISCALER1_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_25_clkdiv",ISCALER1_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_26_clkdiv",ISCALER1_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_27_clkdiv",ISCALER1_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_28_clkdiv",ISCALER1_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_29_clkdiv",ISCALER1_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_30_clkdiv",ISCALER1_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler1_31_clkdiv",ISCALER1_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler2_0_clkdiv",ISCALER2_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_1_clkdiv",ISCALER2_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_2_clkdiv",ISCALER2_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_3_clkdiv",ISCALER2_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_4_clkdiv",ISCALER2_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_5_clkdiv",ISCALER2_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_6_clkdiv",ISCALER2_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_7_clkdiv",ISCALER2_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_8_clkdiv",ISCALER2_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_9_clkdiv",ISCALER2_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_10_clkdiv",ISCALER2_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_11_clkdiv",ISCALER2_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_12_clkdiv",ISCALER2_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_13_clkdiv",ISCALER2_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_14_clkdiv",ISCALER2_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_15_clkdiv",ISCALER2_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_16_clkdiv",ISCALER2_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_17_clkdiv",ISCALER2_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_18_clkdiv",ISCALER2_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_19_clkdiv",ISCALER2_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_20_clkdiv",ISCALER2_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_21_clkdiv",ISCALER2_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_22_clkdiv",ISCALER2_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_23_clkdiv",ISCALER2_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_24_clkdiv",ISCALER2_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_25_clkdiv",ISCALER2_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_26_clkdiv",ISCALER2_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_27_clkdiv",ISCALER2_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_28_clkdiv",ISCALER2_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_29_clkdiv",ISCALER2_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_30_clkdiv",ISCALER2_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler2_31_clkdiv",ISCALER2_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler3_0_clkdiv",ISCALER3_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_1_clkdiv",ISCALER3_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_2_clkdiv",ISCALER3_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_3_clkdiv",ISCALER3_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_4_clkdiv",ISCALER3_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_5_clkdiv",ISCALER3_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_6_clkdiv",ISCALER3_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_7_clkdiv",ISCALER3_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_8_clkdiv",ISCALER3_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_9_clkdiv",ISCALER3_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_10_clkdiv",ISCALER3_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_11_clkdiv",ISCALER3_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_12_clkdiv",ISCALER3_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_13_clkdiv",ISCALER3_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_14_clkdiv",ISCALER3_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_15_clkdiv",ISCALER3_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_16_clkdiv",ISCALER3_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_17_clkdiv",ISCALER3_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_18_clkdiv",ISCALER3_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_19_clkdiv",ISCALER3_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_20_clkdiv",ISCALER3_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_21_clkdiv",ISCALER3_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_22_clkdiv",ISCALER3_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_23_clkdiv",ISCALER3_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_24_clkdiv",ISCALER3_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_25_clkdiv",ISCALER3_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_26_clkdiv",ISCALER3_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_27_clkdiv",ISCALER3_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_28_clkdiv",ISCALER3_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_29_clkdiv",ISCALER3_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_30_clkdiv",ISCALER3_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler3_31_clkdiv",ISCALER3_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler4_0_clkdiv",ISCALER4_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_1_clkdiv",ISCALER4_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_2_clkdiv",ISCALER4_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_3_clkdiv",ISCALER4_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_4_clkdiv",ISCALER4_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_5_clkdiv",ISCALER4_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_6_clkdiv",ISCALER4_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_7_clkdiv",ISCALER4_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_8_clkdiv",ISCALER4_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_9_clkdiv",ISCALER4_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_10_clkdiv",ISCALER4_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_11_clkdiv",ISCALER4_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_12_clkdiv",ISCALER4_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_13_clkdiv",ISCALER4_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_14_clkdiv",ISCALER4_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_15_clkdiv",ISCALER4_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_16_clkdiv",ISCALER4_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_17_clkdiv",ISCALER4_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_18_clkdiv",ISCALER4_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_19_clkdiv",ISCALER4_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_20_clkdiv",ISCALER4_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_21_clkdiv",ISCALER4_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_22_clkdiv",ISCALER4_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_23_clkdiv",ISCALER4_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_24_clkdiv",ISCALER4_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_25_clkdiv",ISCALER4_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_26_clkdiv",ISCALER4_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_27_clkdiv",ISCALER4_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_28_clkdiv",ISCALER4_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_29_clkdiv",ISCALER4_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_30_clkdiv",ISCALER4_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler4_31_clkdiv",ISCALER4_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler5_0_clkdiv",ISCALER5_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_1_clkdiv",ISCALER5_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_2_clkdiv",ISCALER5_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_3_clkdiv",ISCALER5_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_4_clkdiv",ISCALER5_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_5_clkdiv",ISCALER5_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_6_clkdiv",ISCALER5_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_7_clkdiv",ISCALER5_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_8_clkdiv",ISCALER5_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_9_clkdiv",ISCALER5_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_10_clkdiv",ISCALER5_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_11_clkdiv",ISCALER5_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_12_clkdiv",ISCALER5_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_13_clkdiv",ISCALER5_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_14_clkdiv",ISCALER5_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_15_clkdiv",ISCALER5_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_16_clkdiv",ISCALER5_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_17_clkdiv",ISCALER5_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_18_clkdiv",ISCALER5_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_19_clkdiv",ISCALER5_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_20_clkdiv",ISCALER5_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_21_clkdiv",ISCALER5_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_22_clkdiv",ISCALER5_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_23_clkdiv",ISCALER5_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_24_clkdiv",ISCALER5_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_25_clkdiv",ISCALER5_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_26_clkdiv",ISCALER5_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_27_clkdiv",ISCALER5_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_28_clkdiv",ISCALER5_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_29_clkdiv",ISCALER5_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_30_clkdiv",ISCALER5_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler5_31_clkdiv",ISCALER5_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler6_0_clkdiv",ISCALER6_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_1_clkdiv",ISCALER6_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_2_clkdiv",ISCALER6_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_3_clkdiv",ISCALER6_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_4_clkdiv",ISCALER6_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_5_clkdiv",ISCALER6_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_6_clkdiv",ISCALER6_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_7_clkdiv",ISCALER6_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_8_clkdiv",ISCALER6_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_9_clkdiv",ISCALER6_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_10_clkdiv",ISCALER6_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_11_clkdiv",ISCALER6_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_12_clkdiv",ISCALER6_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_13_clkdiv",ISCALER6_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_14_clkdiv",ISCALER6_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_15_clkdiv",ISCALER6_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_16_clkdiv",ISCALER6_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_17_clkdiv",ISCALER6_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_18_clkdiv",ISCALER6_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_19_clkdiv",ISCALER6_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_20_clkdiv",ISCALER6_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_21_clkdiv",ISCALER6_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_22_clkdiv",ISCALER6_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_23_clkdiv",ISCALER6_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_24_clkdiv",ISCALER6_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_25_clkdiv",ISCALER6_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_26_clkdiv",ISCALER6_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_27_clkdiv",ISCALER6_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_28_clkdiv",ISCALER6_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_29_clkdiv",ISCALER6_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_30_clkdiv",ISCALER6_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler6_31_clkdiv",ISCALER6_31_CLKDIV));

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

  fKeyToIdx.insert(make_pair((string)"scaler4_0_cal",ISCALER4_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_1_cal",ISCALER4_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_2_cal",ISCALER4_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_3_cal",ISCALER4_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_4_cal",ISCALER4_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_5_cal",ISCALER4_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_6_cal",ISCALER4_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_7_cal",ISCALER4_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_8_cal",ISCALER4_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_9_cal",ISCALER4_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_10_cal",ISCALER4_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_11_cal",ISCALER4_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_12_cal",ISCALER4_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_13_cal",ISCALER4_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_14_cal",ISCALER4_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_15_cal",ISCALER4_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_16_cal",ISCALER4_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_17_cal",ISCALER4_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_18_cal",ISCALER4_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_19_cal",ISCALER4_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_20_cal",ISCALER4_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_21_cal",ISCALER4_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_22_cal",ISCALER4_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_23_cal",ISCALER4_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_24_cal",ISCALER4_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_25_cal",ISCALER4_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_26_cal",ISCALER4_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_27_cal",ISCALER4_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_28_cal",ISCALER4_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_29_cal",ISCALER4_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_30_cal",ISCALER4_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler4_31_cal",ISCALER4_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler5_0_cal",ISCALER5_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_1_cal",ISCALER5_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_2_cal",ISCALER5_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_3_cal",ISCALER5_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_4_cal",ISCALER5_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_5_cal",ISCALER5_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_6_cal",ISCALER5_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_7_cal",ISCALER5_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_8_cal",ISCALER5_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_9_cal",ISCALER5_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_10_cal",ISCALER5_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_11_cal",ISCALER5_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_12_cal",ISCALER5_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_13_cal",ISCALER5_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_14_cal",ISCALER5_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_15_cal",ISCALER5_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_16_cal",ISCALER5_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_17_cal",ISCALER5_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_18_cal",ISCALER5_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_19_cal",ISCALER5_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_20_cal",ISCALER5_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_21_cal",ISCALER5_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_22_cal",ISCALER5_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_23_cal",ISCALER5_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_24_cal",ISCALER5_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_25_cal",ISCALER5_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_26_cal",ISCALER5_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_27_cal",ISCALER5_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_28_cal",ISCALER5_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_29_cal",ISCALER5_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_30_cal",ISCALER5_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler5_31_cal",ISCALER5_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler6_0_cal",ISCALER6_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_1_cal",ISCALER6_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_2_cal",ISCALER6_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_3_cal",ISCALER6_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_4_cal",ISCALER6_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_5_cal",ISCALER6_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_6_cal",ISCALER6_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_7_cal",ISCALER6_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_8_cal",ISCALER6_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_9_cal",ISCALER6_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_10_cal",ISCALER6_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_11_cal",ISCALER6_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_12_cal",ISCALER6_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_13_cal",ISCALER6_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_14_cal",ISCALER6_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_15_cal",ISCALER6_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_16_cal",ISCALER6_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_17_cal",ISCALER6_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_18_cal",ISCALER6_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_19_cal",ISCALER6_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_20_cal",ISCALER6_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_21_cal",ISCALER6_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_22_cal",ISCALER6_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_23_cal",ISCALER6_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_24_cal",ISCALER6_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_25_cal",ISCALER6_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_26_cal",ISCALER6_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_27_cal",ISCALER6_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_28_cal",ISCALER6_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_29_cal",ISCALER6_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_30_cal",ISCALER6_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler6_31_cal",ISCALER6_31_CAL));


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

// Scan data
  fKeyToIdx.insert(make_pair((string)"scanclean",ISCANCLEAN));
  fKeyToIdx.insert(make_pair((string)"scandata1",ISCANDATA1));
  fKeyToIdx.insert(make_pair((string)"scandata2",ISCANDATA2));
  fKeyToIdx.insert(make_pair((string)"scandata3",ISCANDATA3));
  fKeyToIdx.insert(make_pair((string)"scandata4",ISCANDATA4));

// DAQ flags.  
  fKeyToIdx.insert(make_pair((string)"daq1flag",IDAQ1FLAG));
  fKeyToIdx.insert(make_pair((string)"daq1flag1",IDAQ1FLAG1));
  fKeyToIdx.insert(make_pair((string)"daq1flag2",IDAQ1FLAG2));
  fKeyToIdx.insert(make_pair((string)"daq1flag3",IDAQ1FLAG3));
  fKeyToIdx.insert(make_pair((string)"daq2flag",IDAQ2FLAG));
  fKeyToIdx.insert(make_pair((string)"daq2flag1",IDAQ2FLAG1));
  fKeyToIdx.insert(make_pair((string)"daq2flag2",IDAQ2FLAG2));
  fKeyToIdx.insert(make_pair((string)"daq2flag3",IDAQ2FLAG3));
  fKeyToIdx.insert(make_pair((string)"daq3flag",IDAQ3FLAG));
  fKeyToIdx.insert(make_pair((string)"daq3flag1",IDAQ3FLAG1));
  fKeyToIdx.insert(make_pair((string)"daq3flag2",IDAQ3FLAG2));
  fKeyToIdx.insert(make_pair((string)"daq3flag3",IDAQ3FLAG3));
  fKeyToIdx.insert(make_pair((string)"daq4flag",IDAQ4FLAG));
  fKeyToIdx.insert(make_pair((string)"daq4flag1",IDAQ4FLAG1));
  fKeyToIdx.insert(make_pair((string)"daq4flag2",IDAQ4FLAG2));
  fKeyToIdx.insert(make_pair((string)"daq4flag3",IDAQ4FLAG3));

// BMW words BMW words are also devtype = daqflag
  fKeyToIdx.insert(make_pair((string)"bmwcln",IBMWCLN));
  fKeyToIdx.insert(make_pair((string)"bmwobj",IBMWOBJ ));
  fKeyToIdx.insert(make_pair((string)"bmwval",IBMWVAL));
  fKeyToIdx.insert(make_pair((string)"bmwcyc",IBMWCYC));

// Luminosity Monitors, raw data ("r") and calibrated data
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"lumi1r", ILUMI1R));
  fKeyToIdx.insert(make_pair((string)"lumi1c", ILUMI1C));
  fKeyToIdx.insert(make_pair((string)"lumi1",  ILUMI1));
  fKeyToIdx.insert(make_pair((string)"lumi2r", ILUMI2R));
  fKeyToIdx.insert(make_pair((string)"lumi2c", ILUMI2C));
  fKeyToIdx.insert(make_pair((string)"lumi2",  ILUMI2));
  fKeyToIdx.insert(make_pair((string)"lumi3r", ILUMI3R));
  fKeyToIdx.insert(make_pair((string)"lumi3c", ILUMI3C));
  fKeyToIdx.insert(make_pair((string)"lumi3",  ILUMI3));
  fKeyToIdx.insert(make_pair((string)"lumi4r", ILUMI4R));
  fKeyToIdx.insert(make_pair((string)"lumi4c", ILUMI4C));
  fKeyToIdx.insert(make_pair((string)"lumi4",  ILUMI4));

  fKeyToIdx.insert(make_pair((string)"flumi1r", IFLUMI1R));
  fKeyToIdx.insert(make_pair((string)"flumi1c", IFLUMI1C));
  fKeyToIdx.insert(make_pair((string)"flumi1",  IFLUMI1));
  fKeyToIdx.insert(make_pair((string)"flumi2r", IFLUMI2R));
  fKeyToIdx.insert(make_pair((string)"flumi2c", IFLUMI2C));
  fKeyToIdx.insert(make_pair((string)"flumi2",  IFLUMI2));
  fKeyToIdx.insert(make_pair((string)"flumi3r", IFLUMI3R));
  fKeyToIdx.insert(make_pair((string)"flumi3c", IFLUMI3C));
  fKeyToIdx.insert(make_pair((string)"flumi3",  IFLUMI3));

  fKeyToIdx.insert(make_pair((string)"blumi1r", IBLUMI1R));
  fKeyToIdx.insert(make_pair((string)"blumi1c", IBLUMI1C));
  fKeyToIdx.insert(make_pair((string)"blumi1",  IBLUMI1));
  fKeyToIdx.insert(make_pair((string)"blumi2r", IBLUMI2R));
  fKeyToIdx.insert(make_pair((string)"blumi2c", IBLUMI2C));
  fKeyToIdx.insert(make_pair((string)"blumi2",  IBLUMI2));
  fKeyToIdx.insert(make_pair((string)"blumi3r", IBLUMI3R));
  fKeyToIdx.insert(make_pair((string)"blumi3c", IBLUMI3C));
  fKeyToIdx.insert(make_pair((string)"blumi3",  IBLUMI3));
  fKeyToIdx.insert(make_pair((string)"blumi4r", IBLUMI4R));
  fKeyToIdx.insert(make_pair((string)"blumi4c", IBLUMI4C));
  fKeyToIdx.insert(make_pair((string)"blumi4",  IBLUMI4));
  fKeyToIdx.insert(make_pair((string)"blumi5r", IBLUMI5R));
  fKeyToIdx.insert(make_pair((string)"blumi5c", IBLUMI5C));
  fKeyToIdx.insert(make_pair((string)"blumi5",  IBLUMI5));
  fKeyToIdx.insert(make_pair((string)"blumi6r", IBLUMI6R));
  fKeyToIdx.insert(make_pair((string)"blumi6c", IBLUMI6C));
  fKeyToIdx.insert(make_pair((string)"blumi6",  IBLUMI6));
  fKeyToIdx.insert(make_pair((string)"blumi7r", IBLUMI7R));
  fKeyToIdx.insert(make_pair((string)"blumi7c", IBLUMI7C));
  fKeyToIdx.insert(make_pair((string)"blumi7",  IBLUMI7));
  fKeyToIdx.insert(make_pair((string)"blumi8r", IBLUMI8R));
  fKeyToIdx.insert(make_pair((string)"blumi8c", IBLUMI8C));
  fKeyToIdx.insert(make_pair((string)"blumi8",  IBLUMI8));


  fKeyToIdx.insert(make_pair((string)"v2f_clk0",IV2F_CLK0));
  fKeyToIdx.insert(make_pair((string)"v2f_clk1",IV2F_CLK1));
  fKeyToIdx.insert(make_pair((string)"v2f_clk2",IV2F_CLK2));
  fKeyToIdx.insert(make_pair((string)"v2f_clk3",IV2F_CLK3));
  fKeyToIdx.insert(make_pair((string)"v2f_clk4",IV2F_CLK4));
  fKeyToIdx.insert(make_pair((string)"v2f_clk5",IV2F_CLK5));
  fKeyToIdx.insert(make_pair((string)"v2f_clk6",IV2F_CLK6));

// quad photodiodes  (pp, pm, mp, mm) and calibrated data (x, y, sum)
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"qpd1pp",IQPD1PP));
  fKeyToIdx.insert(make_pair((string)"qpd1pm",IQPD1PM));
  fKeyToIdx.insert(make_pair((string)"qpd1mp",IQPD1MP));
  fKeyToIdx.insert(make_pair((string)"qpd1mm",IQPD1MM));
  fKeyToIdx.insert(make_pair((string)"qpd1ppc",IQPD1PPC));
  fKeyToIdx.insert(make_pair((string)"qpd1pmc",IQPD1PMC));
  fKeyToIdx.insert(make_pair((string)"qpd1mpc",IQPD1MPC));
  fKeyToIdx.insert(make_pair((string)"qpd1mmc",IQPD1MMC));
  fKeyToIdx.insert(make_pair((string)"qpd1x", IQPD1X));
  fKeyToIdx.insert(make_pair((string)"qpd1y", IQPD1Y));
  fKeyToIdx.insert(make_pair((string)"qpd1sum", IQPD1SUM));

// scanner
// "c" = data before pedestal subtracted
  fKeyToIdx.insert(make_pair((string)"scanlxenc",ISCANLXENC));
  fKeyToIdx.insert(make_pair((string)"scanlyenc",ISCANLYENC));
  fKeyToIdx.insert(make_pair((string)"scanlxencc",ISCANLXENCC));
  fKeyToIdx.insert(make_pair((string)"scanlyencc",ISCANLYENCC));
  fKeyToIdx.insert(make_pair((string)"scanlx",ISCANLX));
  fKeyToIdx.insert(make_pair((string)"scanly",ISCANLY));
  fKeyToIdx.insert(make_pair((string)"scanldet",ISCANLDET));
  fKeyToIdx.insert(make_pair((string)"scanldetc",ISCANLDETC));

  fKeyToIdx.insert(make_pair((string)"scanrxenc",ISCANRXENC));
  fKeyToIdx.insert(make_pair((string)"scanryenc",ISCANRYENC));
  fKeyToIdx.insert(make_pair((string)"scanrxencc",ISCANRXENCC));
  fKeyToIdx.insert(make_pair((string)"scanryencc",ISCANRYENCC));
  fKeyToIdx.insert(make_pair((string)"scanrx",ISCANRX));
  fKeyToIdx.insert(make_pair((string)"scanry",ISCANRY));
  fKeyToIdx.insert(make_pair((string)"scanrdet",ISCANRDET));
  fKeyToIdx.insert(make_pair((string)"scanrdetc",ISCANRDETC));

  fKeyToIdx.insert(make_pair((string)"isync0",IISYNC0));
  fKeyToIdx.insert(make_pair((string)"chsync0",ICHSYNC0));
  fKeyToIdx.insert(make_pair((string)"chsync1",ICHSYNC1));
  fKeyToIdx.insert(make_pair((string)"chsync2",ICHSYNC2));
  fKeyToIdx.insert(make_pair((string)"rsync1",IRSYNC1));
  fKeyToIdx.insert(make_pair((string)"rsync2",IRSYNC2));
  fKeyToIdx.insert(make_pair((string)"lsync1",ILSYNC1));
  fKeyToIdx.insert(make_pair((string)"lsync2",ILSYNC2));

};

void TaDevice::Create(const TaDevice& rhs)
{
   fNumRaw = rhs.fNumRaw;
   fKeyToIdx = rhs.fKeyToIdx;
   fRawKeys = new Int_t[MAXKEYS];
   memcpy(fRawKeys, rhs.fRawKeys, MAXKEYS*sizeof(Int_t));
   fEvPointer = new Int_t[MAXKEYS];
   memcpy(fEvPointer, rhs.fEvPointer, MAXKEYS*sizeof(Int_t));
   fCrate = new Int_t[MAXKEYS];
   memcpy(fCrate, rhs.fCrate, MAXKEYS*sizeof(Int_t));
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
   fAdcptr = new Int_t[MAXROC];
   memcpy(fAdcptr, rhs.fAdcptr, MAXROC*sizeof(Int_t));
   fScalptr = new Int_t[MAXROC];
   memcpy(fScalptr, rhs.fScalptr, MAXROC*sizeof(Int_t));
   fTbdptr = new Int_t[MAXROC];
   memcpy(fTbdptr, rhs.fTbdptr, MAXROC*sizeof(Int_t));
   fTirptr = new Int_t[MAXROC];
   memcpy(fTirptr, rhs.fTirptr, MAXROC*sizeof(Int_t));
   fDaqptr = new Int_t[MAXROC];
   memcpy(fDaqptr, rhs.fDaqptr, MAXROC*sizeof(Int_t));
};

void TaDevice::Uncreate()
{
   delete [] fRawKeys;
   delete [] fEvPointer;
   delete [] fCrate;
   delete [] fReadOut;
   delete [] fIsUsed;
   delete [] fIsRotated;
   delete [] fAdcPed;
   delete [] fDacSlope;
   delete [] fDevNum;
   delete [] fChanNum;
   delete [] fAdcptr;
   delete [] fScalptr;
   delete [] fTbdptr;
   delete [] fTirptr;
   delete [] fDaqptr;
};


