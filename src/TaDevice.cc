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
   fAdcxPed = new Double_t[4*ADCXNUM];
   fDacSlope = new Double_t[4*ADCNUM];
   fDacxSlope = new Double_t[4*ADCXNUM];
   fVqwkPed = new Double_t[8*VQWKNUM];
   fDevNum = new Int_t[MAXKEYS];
   fChanNum = new Int_t[MAXKEYS];
   fScalPed = new Double_t[32*SCANUM];
   fVqwkptr = new Int_t[MAXROC];
   fAdcptr = new Int_t[MAXROC];
   fAdcxptr = new Int_t[MAXROC];
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
   memset(fVqwkPed, 0, 8*VQWKNUM*sizeof(Double_t));
   memset(fAdcPed, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fAdcxPed, 0, 4*ADCXNUM*sizeof(Double_t));
   memset(fDacSlope, 0, 4*ADCNUM*sizeof(Double_t));
   memset(fDacxSlope, 0, 4*ADCXNUM*sizeof(Double_t));
   memset(fDevNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fChanNum, 0, MAXKEYS*sizeof(Int_t));
   memset(fScalPed, 0, 32*SCANUM*sizeof(Int_t));
   memset(fVqwkptr, 0, MAXROC*sizeof(Int_t));
   memset(fAdcptr, 0, MAXROC*sizeof(Int_t));
   memset(fAdcxptr, 0, MAXROC*sizeof(Int_t));
   memset(fScalptr, 0, MAXROC*sizeof(Int_t));
   memset(fTbdptr, 0, MAXROC*sizeof(Int_t));
   memset(fTirptr, 0, MAXROC*sizeof(Int_t));
   memset(fDaqptr, 0, MAXROC*sizeof(Int_t));
}

TaDevice::~TaDevice() {
  Uncreate();
  fKeyToIdx.clear();
  fRevKeyMap.clear();
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
// to others like 'vqwk', 'adc', 'adcx', 'scaler' according to rules.

   Int_t key,tiedkey,ivqwk,iadc, iadcx, isca,ichan,i,k;
   string keystr;
   InitKeyList();
   TaKeyMap keymap;
   BpmDefRotate();  
   db.DataMapReStart();
   fgVqwkHeader = db.GetHeader("vqwk");
   fgVqwkMask = db.GetMask("vqwk");
   fgAdcHeader = db.GetHeader("adc");
   fgAdcMask = db.GetMask("adc");
   fgAdcxHeader = db.GetHeader("adcx");
   fgAdcxMask = db.GetMask("adcx");
   fgScalHeader = db.GetHeader("scaler");
   fgScalMask = db.GetMask("scaler");
   fgPvdScaHeader = db.GetHeader("pvdsca");
   fgPvdScaMask = db.GetMask("pvdsca");
   fgTbdHeader = db.GetHeader("timeboard");
   fgTbdMask = db.GetMask("timeboard");
   fgTirHeader = db.GetHeader("tir");
   fgTirMask = db.GetMask("tir");
   fgDaqHeader = db.GetHeader("daqflag");
   fgDaqMask = db.GetMask("daqflag");
// Try to recover from a database that doesn't define header, mask.
   if (fgVqwkHeader == 0) {
     fgVqwkHeader = 0xffadd000;
     cout << "WARNING:  Header for VQWK was zero.";
     cout <<"  Using default  " << hex << fgVqwkHeader << dec << endl;
   }
   if (fgVqwkMask == 0) {
     fgVqwkMask = 0xfffff000;  
     cout << "WARNING:  Mask for Vqwk was zero."; 
     cout <<"  Using default  " << hex << fgVqwkMask << dec << endl;
   }
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
   if (fgAdcxHeader == 0) {
     fgAdcxHeader = 0xfadc1800;
     cout << "WARNING:  Header for ADCX was zero.";
     cout <<"  Using default  " << hex << fgAdcxHeader << dec << endl;
   }
   if (fgAdcxMask == 0) {
     fgAdcxMask = 0xffffff00;
     cout << "WARNING:  Mask for ADCX was zero.";
     cout <<"  Using default  " << hex << fgAdcxMask << dec << endl;
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
   if (fgPvdScaHeader == 0) {
     fgPvdScaHeader = 0xff380100;
     cout << "WARNING:  Header for PVDIS Scaler was zero.";
     cout <<"  Using default  " << hex << fgPvdScaHeader << dec << endl;
   }
   if (fgPvdScaMask == 0) {
     fgPvdScaMask = 0xffffffff;
     cout << "WARNING:  Mask for PVDIS Scaler was zero.";
     cout <<"  Using default  " << hex << fgPvdScaMask << dec << endl;
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
           if (keymap.IsAdcx(keystr)) fReadOut[key] = ADCXREADOUT;
           if (keymap.IsVqwk(keystr)) fReadOut[key] = VQWKREADOUT;
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
             fRevKeyMap.insert(make_pair(tiedkey, key));
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
   for (iadcx = 0; iadcx < ADCXNUM; iadcx++) {
     for (ichan = 0; ichan < 4; ichan++) {
        fAdcxPed[iadcx*4 + ichan] = db.GetAdcxPed(iadcx, ichan);
// Careful !! there is no concept of dacnoise subtraction for ADCx.
// fDacxSlope ?=? db.GetDacxNoise(iadcx, ichan, "slope"); 
        fDacxSlope[iadcx*4 + ichan] = 0;   
 	
     }
   }
   for (ivqwk = 0; ivqwk < VQWKNUM; ivqwk++) {
     for (ichan = 0; ichan < 8; ichan++) {
        fVqwkPed[ivqwk*8 + ichan] = db.GetVqwkPed(ivqwk, ichan);
     }
   }

   for (isca = 0; isca < SCANUM; isca++) {
     for (ichan = 0;  ichan < 32; ichan++) {
       fScalPed[isca*32 + ichan] = db.GetScalPed(isca, ichan);
     }
   }

// Initialize the list for PVDIS.  Must be done after tying keys.

   InitPvdisList();


}

void TaDevice::AddTiedDevices(TaKeyMap& keymap) {
// To potentially add this device to the list of derived types. 
// Note, e.g. "bpm" can be tied to an "adc", but not vice versa.
  if (keymap.GetType() == "adc" || 
      keymap.GetType() == "adcx" || 
      keymap.GetType() == "vqwk" || 
      keymap.GetType() == "scaler") {
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
// The "fundamental" devices are ADC, ADCX, SCALER, VQWK, TIMEBOARD, TIR.
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
  if ((data & fgAdcxMask) == fgAdcxHeader) {
    fAdcxptr[roc] = ipt;
    return;
  }     
  if ((data & fgVqwkMask) == fgVqwkHeader) {
    fVqwkptr[roc] = ipt;
    return;
  }     
  if ((data & fgScalMask) == fgScalHeader) {
    fScalptr[roc] = ipt;
    return;
  }     
  if ((data & fgPvdScaMask) == fgPvdScaHeader) {
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
  cout << "Headers for adc, adcx, vqwk, scaler, timebd, tir, daqflag "<<hex;
  cout << fgAdcHeader << "   " << fgAdcxHeader << "   " <<fgVqwkHeader<<"   ";
  cout << fgScalHeader << "   ";
  cout << fgTbdHeader<<"   "<<fgTirHeader;
  cout << "   " << fgDaqHeader << endl;
  cout << "Masks for adc, adcx, vqwk, scaler, timebd, tir, daqflag "<<hex;
  cout << fgAdcMask << "   " << fgAdcxMask << "   " <<fgVqwkMask<<"   ";
  cout <<fgScalMask<<"   ";
  cout << fgTbdMask<<"   "<<fgTirMask;
  cout << "    "<<fgDaqMask<<endl;
  cout << "Pointers to data : "<<dec<<endl;
  for (Int_t iroc = 1; iroc < MAXROC; iroc++) {
   if (fAdcptr[iroc] > 0) {
       cout << "roc "<<iroc << "  ADCptr= "<<fAdcptr[iroc]<<endl;
   }
   if (fAdcxptr[iroc] > 0) {
       cout << "roc "<<iroc << "  ADCXptr= "<<fAdcxptr[iroc]<<endl;
   }
   if (fVqwkptr[iroc] > 0) {
       cout << "roc "<<iroc << "  Vqwkptr= "<<fVqwkptr[iroc]<<endl;
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
   fRotateList.push_back("bpm0R05");  
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
  if (fReadOut[key] == ADCXREADOUT) {
     index = key - ADCXOFF;
     if (index >= 0 && index < 4*ADCXNUM) return fAdcxPed[index];
  }  
  if (fReadOut[key] == VQWKREADOUT) {
    index = (key - VQWKOFF)/7; 
    // make an integer counter for
    // the 7-word blocks for each VQWK channel
    //    cout << Form("Get Integer for channel %d, key %d, offset %d",index,key,VQWKOFF) << endl;
     if (index >= 0 && index < 7*8*VQWKNUM) return fVqwkPed[index];
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = key - SCAOFF;
     if (index >= 0 && index < 32*SCANUM) return fScalPed[index];
  }  
  return 0;
}

Double_t TaDevice::GetDacSlopeA (const Int_t& key) const 
{
  // This should replace GetDacSlope, whose parameter is an offset from
  // ADCOFF and works only with old ADCs.
  Int_t index;
  if (fReadOut[key] == ADCREADOUT) {
     index = key - ADCOFF;
     if (index >= 0 && index < 4*ADCNUM) return fDacSlope[index];
  }  
  if (fReadOut[key] == ADCXREADOUT) {
     index = key - ADCXOFF;
     if (index >= 0 && index < 4*ADCXNUM) return fDacxSlope[index];
  }  
  return 0;
}

Int_t TaDevice::GetRawIndex(const Int_t& key) const {
// Return a pointer to the raw ADC, ADCX, VQWK or SCALER data corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ADCOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == ADCXREADOUT) {
     index = ADCXOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == VQWKREADOUT) {
    // raw index for vqwk corresponds to total integration period is 7th word (of 7)
    index = VQWKOFF + 6+ 7*(8*GetDevNum(key) + GetChanNum(key));
     return index;
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = SCAOFF + 32*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  return index;
}

Int_t TaDevice::GetCalIndex(const Int_t& key) const {
// Return a pointer to the calibrated ADC, ADCX, VQWK or SCALER data corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ACCOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == ADCXREADOUT) {
     index = ACCXOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == VQWKREADOUT) {
    // cal index should point to full integrate period= 5th word (of 5)
    index = VQWKCOFF + 4+ 5*(8*GetDevNum(key) + GetChanNum(key));
     return index;
  }  
  if (fReadOut[key] == SCALREADOUT) {
     index = SCCOFF + 32*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  return index;  
}

Int_t TaDevice::GetCorrIndex(const Int_t& key) const {
  // Return a pointer to the corrected ADC or ADCX or SCALER data 
  //  (before pedestal subtraction) corresponding to key
  Int_t index = -1;
  if (fReadOut[key] == ADCREADOUT) {
     index = ADCDACSUBOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  if (fReadOut[key] == ADCXREADOUT) {
     index = ADCXDACSUBOFF + 4*GetDevNum(key) + GetChanNum(key);
     return index;
  }  
  //  if (fReadOut[key] == VQWKREADOUT) {
  //   index = 8*GetDevNum(key) + GetChanNum(key);
  //   return index;
  // }  
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

  fKeyToIdx.insert(make_pair((string)"bpm0R05xp", IBPM0R05XP));
  fKeyToIdx.insert(make_pair((string)"bpm0R05xm", IBPM0R05XM));
  fKeyToIdx.insert(make_pair((string)"bpm0R05yp", IBPM0R05YP));
  fKeyToIdx.insert(make_pair((string)"bpm0R05ym", IBPM0R05YM));
  fKeyToIdx.insert(make_pair((string)"bpm0R05xpc",IBPM0R05XPC));
  fKeyToIdx.insert(make_pair((string)"bpm0R05xmc",IBPM0R05XMC));
  fKeyToIdx.insert(make_pair((string)"bpm0R05ypc",IBPM0R05YPC));
  fKeyToIdx.insert(make_pair((string)"bpm0R05ymc",IBPM0R05YMC));
  fKeyToIdx.insert(make_pair((string)"bpm0R05x",  IBPM0R05X));
  fKeyToIdx.insert(make_pair((string)"bpm0R05y",  IBPM0R05Y));
  fKeyToIdx.insert(make_pair((string)"bpm0R05xws",IBPM0R05XWS));
  fKeyToIdx.insert(make_pair((string)"bpm0R05yws",IBPM0R05YWS));
  fKeyToIdx.insert(make_pair((string)"bpm0R05ws", IBPM0R05WS));
  fKeyToIdx.insert(make_pair((string)"bpm0R05mx", IBPM0R05MX));

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


// Detectors combinations
  fKeyToIdx.insert(make_pair((string)"det_l", IDET_L));
  fKeyToIdx.insert(make_pair((string)"det_r", IDET_R));
  fKeyToIdx.insert(make_pair((string)"det_lo", IDET_LO));
  fKeyToIdx.insert(make_pair((string)"det_hi", IDET_HI));
  fKeyToIdx.insert(make_pair((string)"det_all", IDET_ALL));


// DIS detectors and devices

  //total shower R-HRS
  fKeyToIdx.insert(make_pair((string)"disrts1r",IDISRTS1R));
  fKeyToIdx.insert(make_pair((string)"disrts1",IDISRTS1));
  fKeyToIdx.insert(make_pair((string)"disrts2r",IDISRTS2R));
  fKeyToIdx.insert(make_pair((string)"disrts2",IDISRTS2));
  fKeyToIdx.insert(make_pair((string)"disrts3r",IDISRTS3R));
  fKeyToIdx.insert(make_pair((string)"disrts3",IDISRTS3));
  fKeyToIdx.insert(make_pair((string)"disrts4r",IDISRTS4R));
  fKeyToIdx.insert(make_pair((string)"disrts4",IDISRTS4));
  fKeyToIdx.insert(make_pair((string)"disrts5r",IDISRTS5R));
  fKeyToIdx.insert(make_pair((string)"disrts5",IDISRTS5));
  fKeyToIdx.insert(make_pair((string)"disrts6r",IDISRTS6R));
  fKeyToIdx.insert(make_pair((string)"disrts6",IDISRTS6));
  fKeyToIdx.insert(make_pair((string)"disrts7r",IDISRTS7R));
  fKeyToIdx.insert(make_pair((string)"disrts7",IDISRTS7));
  fKeyToIdx.insert(make_pair((string)"disrts8r",IDISRTS8R));
  fKeyToIdx.insert(make_pair((string)"disrts8",IDISRTS8));

  //total shower L-HRS
  fKeyToIdx.insert(make_pair((string)"dislts1r",IDISLTS1R));
  fKeyToIdx.insert(make_pair((string)"dislts1",IDISLTS1));
  fKeyToIdx.insert(make_pair((string)"dislts2r",IDISLTS2R));
  fKeyToIdx.insert(make_pair((string)"dislts2",IDISLTS2));
  fKeyToIdx.insert(make_pair((string)"dislts3r",IDISLTS3R));
  fKeyToIdx.insert(make_pair((string)"dislts3",IDISLTS3));
  fKeyToIdx.insert(make_pair((string)"dislts4r",IDISLTS4R));
  fKeyToIdx.insert(make_pair((string)"dislts4",IDISLTS4));
  fKeyToIdx.insert(make_pair((string)"dislts5r",IDISLTS5R));
  fKeyToIdx.insert(make_pair((string)"dislts5",IDISLTS5));
  fKeyToIdx.insert(make_pair((string)"dislts6r",IDISLTS6R));
  fKeyToIdx.insert(make_pair((string)"dislts6",IDISLTS6));
  fKeyToIdx.insert(make_pair((string)"dislts7r",IDISLTS7R));
  fKeyToIdx.insert(make_pair((string)"dislts7",IDISLTS7));
  fKeyToIdx.insert(make_pair((string)"dislts8r",IDISLTS8R));
  fKeyToIdx.insert(make_pair((string)"dislts8",IDISLTS8));
  
  //preshower R-HRS
  fKeyToIdx.insert(make_pair((string)"disrps1r",IDISRPS1R));
  fKeyToIdx.insert(make_pair((string)"disrps1",IDISRPS1));
  fKeyToIdx.insert(make_pair((string)"disrps2r",IDISRPS2R));
  fKeyToIdx.insert(make_pair((string)"disrps2",IDISRPS2));
  fKeyToIdx.insert(make_pair((string)"disrps3r",IDISRPS3R));
  fKeyToIdx.insert(make_pair((string)"disrps3",IDISRPS3));
  fKeyToIdx.insert(make_pair((string)"disrps4r",IDISRPS4R));
  fKeyToIdx.insert(make_pair((string)"disrps4",IDISRPS4));
  fKeyToIdx.insert(make_pair((string)"disrps5r",IDISRPS5R));
  fKeyToIdx.insert(make_pair((string)"disrps5",IDISRPS5));
  fKeyToIdx.insert(make_pair((string)"disrps6r",IDISRPS6R));
  fKeyToIdx.insert(make_pair((string)"disrps6",IDISRPS6));
  fKeyToIdx.insert(make_pair((string)"disrps7r",IDISRPS7R));
  fKeyToIdx.insert(make_pair((string)"disrps7",IDISRPS7));
  fKeyToIdx.insert(make_pair((string)"disrps8r",IDISRPS8R));
  fKeyToIdx.insert(make_pair((string)"disrps8",IDISRPS8));

  //preshower L-HRS
  fKeyToIdx.insert(make_pair((string)"dislps1r",IDISLPS1R));
  fKeyToIdx.insert(make_pair((string)"dislps1",IDISLPS1));
  fKeyToIdx.insert(make_pair((string)"dislps2r",IDISLPS2R));
  fKeyToIdx.insert(make_pair((string)"dislps2",IDISLPS2));
  fKeyToIdx.insert(make_pair((string)"dislps3r",IDISLPS3R));
  fKeyToIdx.insert(make_pair((string)"dislps3",IDISLPS3));
  fKeyToIdx.insert(make_pair((string)"dislps4r",IDISLPS4R));
  fKeyToIdx.insert(make_pair((string)"dislps4",IDISLPS4));
  fKeyToIdx.insert(make_pair((string)"dislps5r",IDISLPS5R));
  fKeyToIdx.insert(make_pair((string)"dislps5",IDISLPS5));
  fKeyToIdx.insert(make_pair((string)"dislps6r",IDISLPS6R));
  fKeyToIdx.insert(make_pair((string)"dislps6",IDISLPS6));
  fKeyToIdx.insert(make_pair((string)"dislps7r",IDISLPS7R));
  fKeyToIdx.insert(make_pair((string)"dislps7",IDISLPS7));
  fKeyToIdx.insert(make_pair((string)"dislps8r",IDISLPS8R));
  fKeyToIdx.insert(make_pair((string)"dislps8",IDISLPS8));


// DIS triggers

//ELECTRON NARROW L-HSR
  fKeyToIdx.insert(make_pair((string)"dislen1r",IDISLEN1R));
  fKeyToIdx.insert(make_pair((string)"dislen1",IDISLEN1));
  fKeyToIdx.insert(make_pair((string)"dislen2r",IDISLEN2R));
  fKeyToIdx.insert(make_pair((string)"dislen2",IDISLEN2));
  fKeyToIdx.insert(make_pair((string)"dislen3r",IDISLEN3R));
  fKeyToIdx.insert(make_pair((string)"dislen3",IDISLEN3));
  fKeyToIdx.insert(make_pair((string)"dislen4r",IDISLEN4R));
  fKeyToIdx.insert(make_pair((string)"dislen4",IDISLEN4));
  fKeyToIdx.insert(make_pair((string)"dislen5r",IDISLEN5R));
  fKeyToIdx.insert(make_pair((string)"dislen5",IDISLEN5));
  fKeyToIdx.insert(make_pair((string)"dislen6r",IDISLEN6R));
  fKeyToIdx.insert(make_pair((string)"dislen6",IDISLEN6));
  fKeyToIdx.insert(make_pair((string)"dislen7r",IDISLEN7R));
  fKeyToIdx.insert(make_pair((string)"dislen7",IDISLEN7));
  fKeyToIdx.insert(make_pair((string)"dislen8r",IDISLEN8R));
  fKeyToIdx.insert(make_pair((string)"dislen8",IDISLEN8));
  //ELECTRON NARROW R-HRS
  fKeyToIdx.insert(make_pair((string)"disren1r",IDISREN1R));
  fKeyToIdx.insert(make_pair((string)"disren1",IDISREN1));
  fKeyToIdx.insert(make_pair((string)"disren2r",IDISREN2R));
  fKeyToIdx.insert(make_pair((string)"disren2",IDISREN2));
  fKeyToIdx.insert(make_pair((string)"disren3r",IDISREN3R));
  fKeyToIdx.insert(make_pair((string)"disren3",IDISREN3));
  fKeyToIdx.insert(make_pair((string)"disren4r",IDISREN4R));
  fKeyToIdx.insert(make_pair((string)"disren4",IDISREN4));
  fKeyToIdx.insert(make_pair((string)"disren5r",IDISREN5R));
  fKeyToIdx.insert(make_pair((string)"disren5",IDISREN5));
  fKeyToIdx.insert(make_pair((string)"disren6r",IDISREN6R));
  fKeyToIdx.insert(make_pair((string)"disren6",IDISREN6));
  fKeyToIdx.insert(make_pair((string)"disren7r",IDISREN7R));
  fKeyToIdx.insert(make_pair((string)"disren7",IDISREN7));
  fKeyToIdx.insert(make_pair((string)"disren8r",IDISREN8R));
  fKeyToIdx.insert(make_pair((string)"disren8",IDISREN8));
  //ELECTRON WIDE L-HRS
  fKeyToIdx.insert(make_pair((string)"dislew1r",IDISLEW1R));
  fKeyToIdx.insert(make_pair((string)"dislew1",IDISLEW1));
  fKeyToIdx.insert(make_pair((string)"dislew2r",IDISLEW2R));
  fKeyToIdx.insert(make_pair((string)"dislew2",IDISLEW2));
  fKeyToIdx.insert(make_pair((string)"dislew3r",IDISLEW3R));
  fKeyToIdx.insert(make_pair((string)"dislew3",IDISLEW3));
  fKeyToIdx.insert(make_pair((string)"dislew4r",IDISLEW4R));
  fKeyToIdx.insert(make_pair((string)"dislew4",IDISLEW4));
  fKeyToIdx.insert(make_pair((string)"dislew5r",IDISLEW5R));
  fKeyToIdx.insert(make_pair((string)"dislew5",IDISLEW5));
  fKeyToIdx.insert(make_pair((string)"dislew6r",IDISLEW6R));
  fKeyToIdx.insert(make_pair((string)"dislew6",IDISLEW6));
  fKeyToIdx.insert(make_pair((string)"dislew7r",IDISLEW7R));
  fKeyToIdx.insert(make_pair((string)"dislew7",IDISLEW7));
  fKeyToIdx.insert(make_pair((string)"dislew8r",IDISLEW8R));
  fKeyToIdx.insert(make_pair((string)"dislew8",IDISLEW8));
  //ELECTRON WIDE R-HRS
  fKeyToIdx.insert(make_pair((string)"disrew1r",IDISREW1R));
  fKeyToIdx.insert(make_pair((string)"disrew1",IDISREW1));
  fKeyToIdx.insert(make_pair((string)"disrew2r",IDISREW2R));
  fKeyToIdx.insert(make_pair((string)"disrew2",IDISREW2));
  fKeyToIdx.insert(make_pair((string)"disrew3r",IDISREW3R));
  fKeyToIdx.insert(make_pair((string)"disrew3",IDISREW3));
  fKeyToIdx.insert(make_pair((string)"disrew4r",IDISREW4R));
  fKeyToIdx.insert(make_pair((string)"disrew4",IDISREW4));
  fKeyToIdx.insert(make_pair((string)"disrew5r",IDISREW5R));
  fKeyToIdx.insert(make_pair((string)"disrew5",IDISREW5));
  fKeyToIdx.insert(make_pair((string)"disrew6r",IDISREW6R));
  fKeyToIdx.insert(make_pair((string)"disrew6",IDISREW6));
  fKeyToIdx.insert(make_pair((string)"disrew7r",IDISREW7R));
  fKeyToIdx.insert(make_pair((string)"disrew7",IDISREW7));
  fKeyToIdx.insert(make_pair((string)"disrew8r",IDISREW8R));
  fKeyToIdx.insert(make_pair((string)"disrew8",IDISREW8));
  //PION WIDE L-HRS
  fKeyToIdx.insert(make_pair((string)"dislpw1r",IDISLPW1R));
  fKeyToIdx.insert(make_pair((string)"dislpw1",IDISLPW1));
  fKeyToIdx.insert(make_pair((string)"dislpw2r",IDISLPW2R));
  fKeyToIdx.insert(make_pair((string)"dislpw2",IDISLPW2));
  fKeyToIdx.insert(make_pair((string)"dislpw3r",IDISLPW3R));
  fKeyToIdx.insert(make_pair((string)"dislpw3",IDISLPW3));
  fKeyToIdx.insert(make_pair((string)"dislpw4r",IDISLPW4R));
  fKeyToIdx.insert(make_pair((string)"dislpw4",IDISLPW4));
  fKeyToIdx.insert(make_pair((string)"dislpw5r",IDISLPW5R));
  fKeyToIdx.insert(make_pair((string)"dislpw5",IDISLPW5));
  fKeyToIdx.insert(make_pair((string)"dislpw6r",IDISLPW6R));
  fKeyToIdx.insert(make_pair((string)"dislpw6",IDISLPW6));
  fKeyToIdx.insert(make_pair((string)"dislpw7r",IDISLPW7R));
  fKeyToIdx.insert(make_pair((string)"dislpw7",IDISLPW7));
  fKeyToIdx.insert(make_pair((string)"dislpw8r",IDISLPW8R));
  fKeyToIdx.insert(make_pair((string)"dislpw8",IDISLPW8));
  //PION WIDE R-HRS
  fKeyToIdx.insert(make_pair((string)"disrpw1r",IDISRPW1R));
  fKeyToIdx.insert(make_pair((string)"disrpw1",IDISRPW1));
  fKeyToIdx.insert(make_pair((string)"disrpw2r",IDISRPW2R));
  fKeyToIdx.insert(make_pair((string)"disrpw2",IDISRPW2));
  fKeyToIdx.insert(make_pair((string)"disrpw3r",IDISRPW3R));
  fKeyToIdx.insert(make_pair((string)"disrpw3",IDISRPW3));
  fKeyToIdx.insert(make_pair((string)"disrpw4r",IDISRPW4R));
  fKeyToIdx.insert(make_pair((string)"disrpw4",IDISRPW4));
  fKeyToIdx.insert(make_pair((string)"disrpw5r",IDISRPW5R));
  fKeyToIdx.insert(make_pair((string)"disrpw5",IDISRPW5));
  fKeyToIdx.insert(make_pair((string)"disrpw6r",IDISRPW6R));
  fKeyToIdx.insert(make_pair((string)"disrpw6",IDISRPW6));
  fKeyToIdx.insert(make_pair((string)"disrpw7r",IDISRPW7R));
  fKeyToIdx.insert(make_pair((string)"disrpw7",IDISRPW7));
  fKeyToIdx.insert(make_pair((string)"disrpw8r",IDISRPW8R));
  fKeyToIdx.insert(make_pair((string)"disrpw8",IDISRPW8));
  //PION NARROW L-HRS
  fKeyToIdx.insert(make_pair((string)"dislpn1r",IDISLPN1R));
  fKeyToIdx.insert(make_pair((string)"dislpn1",IDISLPN1));
  fKeyToIdx.insert(make_pair((string)"dislpn2r",IDISLPN2R));
  fKeyToIdx.insert(make_pair((string)"dislpn2",IDISLPN2));
  fKeyToIdx.insert(make_pair((string)"dislpn3r",IDISLPN3R));
  fKeyToIdx.insert(make_pair((string)"dislpn3",IDISLPN3));
  fKeyToIdx.insert(make_pair((string)"dislpn4r",IDISLPN4R));
  fKeyToIdx.insert(make_pair((string)"dislpn4",IDISLPN4));
  fKeyToIdx.insert(make_pair((string)"dislpn5r",IDISLPN5R));
  fKeyToIdx.insert(make_pair((string)"dislpn5",IDISLPN5));
  fKeyToIdx.insert(make_pair((string)"dislpn6r",IDISLPN6R));
  fKeyToIdx.insert(make_pair((string)"dislpn6",IDISLPN6));
  fKeyToIdx.insert(make_pair((string)"dislpn7r",IDISLPN7R));
  fKeyToIdx.insert(make_pair((string)"dislpn7",IDISLPN7));
  fKeyToIdx.insert(make_pair((string)"dislpn8r",IDISLPN8R));
  fKeyToIdx.insert(make_pair((string)"dislpn8",IDISLPN8));
  //PION NARROW R-HRS
  fKeyToIdx.insert(make_pair((string)"disrpn1r",IDISRPN1R));
  fKeyToIdx.insert(make_pair((string)"disrpn1",IDISRPN1));
  fKeyToIdx.insert(make_pair((string)"disrpn2r",IDISRPN2R));
  fKeyToIdx.insert(make_pair((string)"disrpn2",IDISRPN2));
  fKeyToIdx.insert(make_pair((string)"disrpn3r",IDISRPN3R));
  fKeyToIdx.insert(make_pair((string)"disrpn3",IDISRPN3));
  fKeyToIdx.insert(make_pair((string)"disrpn4r",IDISRPN4R));
  fKeyToIdx.insert(make_pair((string)"disrpn4",IDISRPN4));
  fKeyToIdx.insert(make_pair((string)"disrpn5r",IDISRPN5R));
  fKeyToIdx.insert(make_pair((string)"disrpn5",IDISRPN5));
  fKeyToIdx.insert(make_pair((string)"disrpn6r",IDISRPN6R));
  fKeyToIdx.insert(make_pair((string)"disrpn6",IDISRPN6));
  fKeyToIdx.insert(make_pair((string)"disrpn7r",IDISRPN7R));
  fKeyToIdx.insert(make_pair((string)"disrpn7",IDISRPN7));
  fKeyToIdx.insert(make_pair((string)"disrpn8r",IDISRPN8R));
  fKeyToIdx.insert(make_pair((string)"disrpn8",IDISRPN8));
  //TAGGER L-HRS
  fKeyToIdx.insert(make_pair((string)"disltg1r",IDISLTG1R));
  fKeyToIdx.insert(make_pair((string)"disltg1",IDISLTG1));
  fKeyToIdx.insert(make_pair((string)"disltg2r",IDISLTG2R));
  fKeyToIdx.insert(make_pair((string)"disltg2",IDISLTG2));
  fKeyToIdx.insert(make_pair((string)"disltg3r",IDISLTG3R));
  fKeyToIdx.insert(make_pair((string)"disltg3",IDISLTG3));
  fKeyToIdx.insert(make_pair((string)"disltg4r",IDISLTG4R));
  fKeyToIdx.insert(make_pair((string)"disltg4",IDISLTG4));
  fKeyToIdx.insert(make_pair((string)"disltg5r",IDISLTG5R));
  fKeyToIdx.insert(make_pair((string)"disltg5",IDISLTG5));
  fKeyToIdx.insert(make_pair((string)"disltg6r",IDISLTG6R));
  fKeyToIdx.insert(make_pair((string)"disltg6",IDISLTG6));
  fKeyToIdx.insert(make_pair((string)"disltg7r",IDISLTG7R));
  fKeyToIdx.insert(make_pair((string)"disltg7",IDISLTG7));
  fKeyToIdx.insert(make_pair((string)"disltg8r",IDISLTG8R));
  fKeyToIdx.insert(make_pair((string)"disltg8",IDISLTG8));
  //TAGGER R-HRS
  fKeyToIdx.insert(make_pair((string)"disrtg1r",IDISRTG1R));
  fKeyToIdx.insert(make_pair((string)"disrtg1",IDISRTG1));
  fKeyToIdx.insert(make_pair((string)"disrtg2r",IDISRTG2R));
  fKeyToIdx.insert(make_pair((string)"disrtg2",IDISRTG2));
  fKeyToIdx.insert(make_pair((string)"disrtg3r",IDISRTG3R));
  fKeyToIdx.insert(make_pair((string)"disrtg3",IDISRTG3));
  fKeyToIdx.insert(make_pair((string)"disrtg4r",IDISRTG4R));
  fKeyToIdx.insert(make_pair((string)"disrtg4",IDISRTG4));
  fKeyToIdx.insert(make_pair((string)"disrtg5r",IDISRTG5R));
  fKeyToIdx.insert(make_pair((string)"disrtg5",IDISRTG5));
  fKeyToIdx.insert(make_pair((string)"disrtg6r",IDISRTG6R));
  fKeyToIdx.insert(make_pair((string)"disrtg6",IDISRTG6));
  fKeyToIdx.insert(make_pair((string)"disrtg7r",IDISRTG7R));
  fKeyToIdx.insert(make_pair((string)"disrtg7",IDISRTG7));
  fKeyToIdx.insert(make_pair((string)"disrtg8r",IDISRTG8R));
  fKeyToIdx.insert(make_pair((string)"disrtg8",IDISRTG8));
  //MIXED STUFF L-HRS
  fKeyToIdx.insert(make_pair((string)"dislmxd1r",IDISLMXD1R));
  fKeyToIdx.insert(make_pair((string)"dislmxd1",IDISLMXD1));
  fKeyToIdx.insert(make_pair((string)"dislmxd2r",IDISLMXD2R));
  fKeyToIdx.insert(make_pair((string)"dislmxd2",IDISLMXD2));
  fKeyToIdx.insert(make_pair((string)"dislmxd3r",IDISLMXD3R));
  fKeyToIdx.insert(make_pair((string)"dislmxd3",IDISLMXD3));
  fKeyToIdx.insert(make_pair((string)"dislmxd4r",IDISLMXD4R));
  fKeyToIdx.insert(make_pair((string)"dislmxd4",IDISLMXD4));
  fKeyToIdx.insert(make_pair((string)"dislmxd5r",IDISLMXD5R));
  fKeyToIdx.insert(make_pair((string)"dislmxd5",IDISLMXD5));
  fKeyToIdx.insert(make_pair((string)"dislmxd6r",IDISLMXD6R));
  fKeyToIdx.insert(make_pair((string)"dislmxd6",IDISLMXD6));
  fKeyToIdx.insert(make_pair((string)"dislmxd7r",IDISLMXD7R));
  fKeyToIdx.insert(make_pair((string)"dislmxd7",IDISLMXD7));
  fKeyToIdx.insert(make_pair((string)"dislmxd8r",IDISLMXD8R));
  fKeyToIdx.insert(make_pair((string)"dislmxd8",IDISLMXD8));
  //MIXED STUFF R-HRS
  fKeyToIdx.insert(make_pair((string)"disrmxd1r",IDISRMXD1R));
  fKeyToIdx.insert(make_pair((string)"disrmxd1",IDISRMXD1));
  fKeyToIdx.insert(make_pair((string)"disrmxd2r",IDISRMXD2R));
  fKeyToIdx.insert(make_pair((string)"disrmxd2",IDISRMXD2));
  fKeyToIdx.insert(make_pair((string)"disrmxd3r",IDISRMXD3R));
  fKeyToIdx.insert(make_pair((string)"disrmxd3",IDISRMXD3));
  fKeyToIdx.insert(make_pair((string)"disrmxd4r",IDISRMXD4R));
  fKeyToIdx.insert(make_pair((string)"disrmxd4",IDISRMXD4));
  fKeyToIdx.insert(make_pair((string)"disrmxd5r",IDISRMXD5R));
  fKeyToIdx.insert(make_pair((string)"disrmxd5",IDISRMXD5));
  fKeyToIdx.insert(make_pair((string)"disrmxd6r",IDISRMXD6R));
  fKeyToIdx.insert(make_pair((string)"disrmxd6",IDISRMXD6));
  fKeyToIdx.insert(make_pair((string)"disrmxd7r",IDISRMXD7R));
  fKeyToIdx.insert(make_pair((string)"disrmxd7",IDISRMXD7));
  fKeyToIdx.insert(make_pair((string)"disrmxd8r",IDISRMXD8R));
  fKeyToIdx.insert(make_pair((string)"disrmxd8",IDISRMXD8));
//total shower COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrtsc1r",IDISRTSC1R));
  fKeyToIdx.insert(make_pair((string)"disrtsc1",IDISRTSC1));
  fKeyToIdx.insert(make_pair((string)"disrtsc2r",IDISRTSC2R));
  fKeyToIdx.insert(make_pair((string)"disrtsc2",IDISRTSC2));
  fKeyToIdx.insert(make_pair((string)"disrtsc3r",IDISRTSC3R));
  fKeyToIdx.insert(make_pair((string)"disrtsc3",IDISRTSC3));
  fKeyToIdx.insert(make_pair((string)"disrtsc4r",IDISRTSC4R));
  fKeyToIdx.insert(make_pair((string)"disrtsc4",IDISRTSC4));
  fKeyToIdx.insert(make_pair((string)"disrtsc5r",IDISRTSC5R));
  fKeyToIdx.insert(make_pair((string)"disrtsc5",IDISRTSC5));
  fKeyToIdx.insert(make_pair((string)"disrtsc6r",IDISRTSC6R));
  fKeyToIdx.insert(make_pair((string)"disrtsc6",IDISRTSC6));
  fKeyToIdx.insert(make_pair((string)"disrtsc7r",IDISRTSC7R));
  fKeyToIdx.insert(make_pair((string)"disrtsc7",IDISRTSC7));
  fKeyToIdx.insert(make_pair((string)"disrtsc8r",IDISRTSC8R));
  fKeyToIdx.insert(make_pair((string)"disrtsc8",IDISRTSC8));
  //total shower COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"disltsc1r",IDISLTSC1R));
  fKeyToIdx.insert(make_pair((string)"disltsc1",IDISLTSC1));
  fKeyToIdx.insert(make_pair((string)"disltsc2r",IDISLTSC2R));
  fKeyToIdx.insert(make_pair((string)"disltsc2",IDISLTSC2));
  fKeyToIdx.insert(make_pair((string)"disltsc3r",IDISLTSC3R));
  fKeyToIdx.insert(make_pair((string)"disltsc3",IDISLTSC3));
  fKeyToIdx.insert(make_pair((string)"disltsc4r",IDISLTSC4R));
  fKeyToIdx.insert(make_pair((string)"disltsc4",IDISLTSC4));
  fKeyToIdx.insert(make_pair((string)"disltsc5r",IDISLTSC5R));
  fKeyToIdx.insert(make_pair((string)"disltsc5",IDISLTSC5));
  fKeyToIdx.insert(make_pair((string)"disltsc6r",IDISLTSC6R));
  fKeyToIdx.insert(make_pair((string)"disltsc6",IDISLTSC6));
  fKeyToIdx.insert(make_pair((string)"disltsc7r",IDISLTSC7R));
  fKeyToIdx.insert(make_pair((string)"disltsc7",IDISLTSC7));
  fKeyToIdx.insert(make_pair((string)"disltsc8r",IDISLTSC8R));
  fKeyToIdx.insert(make_pair((string)"disltsc8",IDISLTSC8));
  //preshower COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrpsc1r",IDISRPSC1R));
  fKeyToIdx.insert(make_pair((string)"disrpsc1",IDISRPSC1));
  fKeyToIdx.insert(make_pair((string)"disrpsc2r",IDISRPSC2R));
  fKeyToIdx.insert(make_pair((string)"disrpsc2",IDISRPSC2));
  fKeyToIdx.insert(make_pair((string)"disrpsc3r",IDISRPSC3R));
  fKeyToIdx.insert(make_pair((string)"disrpsc3",IDISRPSC3));
  fKeyToIdx.insert(make_pair((string)"disrpsc4r",IDISRPSC4R));
  fKeyToIdx.insert(make_pair((string)"disrpsc4",IDISRPSC4));
  fKeyToIdx.insert(make_pair((string)"disrpsc5r",IDISRPSC5R));
  fKeyToIdx.insert(make_pair((string)"disrpsc5",IDISRPSC5));
  fKeyToIdx.insert(make_pair((string)"disrpsc6r",IDISRPSC6R));
  fKeyToIdx.insert(make_pair((string)"disrpsc6",IDISRPSC6));
  fKeyToIdx.insert(make_pair((string)"disrpsc7r",IDISRPSC7R));
  fKeyToIdx.insert(make_pair((string)"disrpsc7",IDISRPSC7));
  fKeyToIdx.insert(make_pair((string)"disrpsc8r",IDISRPSC8R));
  fKeyToIdx.insert(make_pair((string)"disrpsc8",IDISRPSC8));
  //preshower COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"dislpsc1r",IDISLPSC1R));
  fKeyToIdx.insert(make_pair((string)"dislpsc1",IDISLPSC1));
  fKeyToIdx.insert(make_pair((string)"dislpsc2r",IDISLPSC2R));
  fKeyToIdx.insert(make_pair((string)"dislpsc2",IDISLPSC2));
  fKeyToIdx.insert(make_pair((string)"dislpsc3r",IDISLPSC3R));
  fKeyToIdx.insert(make_pair((string)"dislpsc3",IDISLPSC3));
  fKeyToIdx.insert(make_pair((string)"dislpsc4r",IDISLPSC4R));
  fKeyToIdx.insert(make_pair((string)"dislpsc4",IDISLPSC4));
  fKeyToIdx.insert(make_pair((string)"dislpsc5r",IDISLPSC5R));
  fKeyToIdx.insert(make_pair((string)"dislpsc5",IDISLPSC5));
  fKeyToIdx.insert(make_pair((string)"dislpsc6r",IDISLPSC6R));
  fKeyToIdx.insert(make_pair((string)"dislpsc6",IDISLPSC6));
  fKeyToIdx.insert(make_pair((string)"dislpsc7r",IDISLPSC7R));
  fKeyToIdx.insert(make_pair((string)"dislpsc7",IDISLPSC7));
  fKeyToIdx.insert(make_pair((string)"dislpsc8r",IDISLPSC8R));
  fKeyToIdx.insert(make_pair((string)"dislpsc8",IDISLPSC8));
  //ELECTRON NARROW COPY L-HSR
  fKeyToIdx.insert(make_pair((string)"dislenc1r",IDISLENC1R));
  fKeyToIdx.insert(make_pair((string)"dislenc1",IDISLENC1));
  fKeyToIdx.insert(make_pair((string)"dislenc2r",IDISLENC2R));
  fKeyToIdx.insert(make_pair((string)"dislenc2",IDISLENC2));
  fKeyToIdx.insert(make_pair((string)"dislenc3r",IDISLENC3R));
  fKeyToIdx.insert(make_pair((string)"dislenc3",IDISLENC3));
  fKeyToIdx.insert(make_pair((string)"dislenc4r",IDISLENC4R));
  fKeyToIdx.insert(make_pair((string)"dislenc4",IDISLENC4));
  fKeyToIdx.insert(make_pair((string)"dislenc5r",IDISLENC5R));
  fKeyToIdx.insert(make_pair((string)"dislenc5",IDISLENC5));
  fKeyToIdx.insert(make_pair((string)"dislenc6r",IDISLENC6R));
  fKeyToIdx.insert(make_pair((string)"dislenc6",IDISLENC6));
  fKeyToIdx.insert(make_pair((string)"dislenc7r",IDISLENC7R));
  fKeyToIdx.insert(make_pair((string)"dislenc7",IDISLENC7));
  fKeyToIdx.insert(make_pair((string)"dislenc8r",IDISLENC8R));
  fKeyToIdx.insert(make_pair((string)"dislenc8",IDISLENC8));
  //ELECTRON NARROW COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrenc1r",IDISRENC1R));
  fKeyToIdx.insert(make_pair((string)"disrenc1",IDISRENC1));
  fKeyToIdx.insert(make_pair((string)"disrenc2r",IDISRENC2R));
  fKeyToIdx.insert(make_pair((string)"disrenc2",IDISRENC2));
  fKeyToIdx.insert(make_pair((string)"disrenc3r",IDISRENC3R));
  fKeyToIdx.insert(make_pair((string)"disrenc3",IDISRENC3));
  fKeyToIdx.insert(make_pair((string)"disrenc4r",IDISRENC4R));
  fKeyToIdx.insert(make_pair((string)"disrenc4",IDISRENC4));
  fKeyToIdx.insert(make_pair((string)"disrenc5r",IDISRENC5R));
  fKeyToIdx.insert(make_pair((string)"disrenc5",IDISRENC5));
  fKeyToIdx.insert(make_pair((string)"disrenc6r",IDISRENC6R));
  fKeyToIdx.insert(make_pair((string)"disrenc6",IDISRENC6));
  fKeyToIdx.insert(make_pair((string)"disrenc7r",IDISRENC7R));
  fKeyToIdx.insert(make_pair((string)"disrenc7",IDISRENC7));
  fKeyToIdx.insert(make_pair((string)"disrenc8r",IDISRENC8R));
  fKeyToIdx.insert(make_pair((string)"disrenc8",IDISRENC8));
  //ELECTRON WIDE COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"dislewc1r",IDISLEWC1R));
  fKeyToIdx.insert(make_pair((string)"dislewc1",IDISLEWC1));
  fKeyToIdx.insert(make_pair((string)"dislewc2r",IDISLEWC2R));
  fKeyToIdx.insert(make_pair((string)"dislewc2",IDISLEWC2));
  fKeyToIdx.insert(make_pair((string)"dislewc3r",IDISLEWC3R));
  fKeyToIdx.insert(make_pair((string)"dislewc3",IDISLEWC3));
  fKeyToIdx.insert(make_pair((string)"dislewc4r",IDISLEWC4R));
  fKeyToIdx.insert(make_pair((string)"dislewc4",IDISLEWC4));
  fKeyToIdx.insert(make_pair((string)"dislewc5r",IDISLEWC5R));
  fKeyToIdx.insert(make_pair((string)"dislewc5",IDISLEWC5));
  fKeyToIdx.insert(make_pair((string)"dislewc6r",IDISLEWC6R));
  fKeyToIdx.insert(make_pair((string)"dislewc6",IDISLEWC6));
  fKeyToIdx.insert(make_pair((string)"dislewc7r",IDISLEWC7R));
  fKeyToIdx.insert(make_pair((string)"dislewc7",IDISLEWC7));
  fKeyToIdx.insert(make_pair((string)"dislewc8r",IDISLEWC8R));
  fKeyToIdx.insert(make_pair((string)"dislewc8",IDISLEWC8));
  //ELECTRON WIDE COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrewc1r",IDISREWC1R));
  fKeyToIdx.insert(make_pair((string)"disrewc1",IDISREWC1));
  fKeyToIdx.insert(make_pair((string)"disrewc2r",IDISREWC2R));
  fKeyToIdx.insert(make_pair((string)"disrewc2",IDISREWC2));
  fKeyToIdx.insert(make_pair((string)"disrewc3r",IDISREWC3R));
  fKeyToIdx.insert(make_pair((string)"disrewc3",IDISREWC3));
  fKeyToIdx.insert(make_pair((string)"disrewc4r",IDISREWC4R));
  fKeyToIdx.insert(make_pair((string)"disrewc4",IDISREWC4));
  fKeyToIdx.insert(make_pair((string)"disrewc5r",IDISREWC5R));
  fKeyToIdx.insert(make_pair((string)"disrewc5",IDISREWC5));
  fKeyToIdx.insert(make_pair((string)"disrewc6r",IDISREWC6R));
  fKeyToIdx.insert(make_pair((string)"disrewc6",IDISREWC6));
  fKeyToIdx.insert(make_pair((string)"disrewc7r",IDISREWC7R));
  fKeyToIdx.insert(make_pair((string)"disrewc7",IDISREWC7));
  fKeyToIdx.insert(make_pair((string)"disrewc8r",IDISREWC8R));
  fKeyToIdx.insert(make_pair((string)"disrewc8",IDISREWC8));
  //PION WIDE COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"dislpwc1r",IDISLPWC1R));
  fKeyToIdx.insert(make_pair((string)"dislpwc1",IDISLPWC1));
  fKeyToIdx.insert(make_pair((string)"dislpwc2r",IDISLPWC2R));
  fKeyToIdx.insert(make_pair((string)"dislpwc2",IDISLPWC2));
  fKeyToIdx.insert(make_pair((string)"dislpwc3r",IDISLPWC3R));
  fKeyToIdx.insert(make_pair((string)"dislpwc3",IDISLPWC3));
  fKeyToIdx.insert(make_pair((string)"dislpwc4r",IDISLPWC4R));
  fKeyToIdx.insert(make_pair((string)"dislpwc4",IDISLPWC4));
  fKeyToIdx.insert(make_pair((string)"dislpwc5r",IDISLPWC5R));
  fKeyToIdx.insert(make_pair((string)"dislpwc5",IDISLPWC5));
  fKeyToIdx.insert(make_pair((string)"dislpwc6r",IDISLPWC6R));
  fKeyToIdx.insert(make_pair((string)"dislpwc6",IDISLPWC6));
  fKeyToIdx.insert(make_pair((string)"dislpwc7r",IDISLPWC7R));
  fKeyToIdx.insert(make_pair((string)"dislpwc7",IDISLPWC7));
  fKeyToIdx.insert(make_pair((string)"dislpwc8r",IDISLPWC8R));
  fKeyToIdx.insert(make_pair((string)"dislpwc8",IDISLPWC8));
  //PION WIDE COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrpwc1r",IDISRPWC1R));
  fKeyToIdx.insert(make_pair((string)"disrpwc1",IDISRPWC1));
  fKeyToIdx.insert(make_pair((string)"disrpwc2r",IDISRPWC2R));
  fKeyToIdx.insert(make_pair((string)"disrpwc2",IDISRPWC2));
  fKeyToIdx.insert(make_pair((string)"disrpwc3r",IDISRPWC3R));
  fKeyToIdx.insert(make_pair((string)"disrpwc3",IDISRPWC3));
  fKeyToIdx.insert(make_pair((string)"disrpwc4r",IDISRPWC4R));
  fKeyToIdx.insert(make_pair((string)"disrpwc4",IDISRPWC4));
  fKeyToIdx.insert(make_pair((string)"disrpwc5r",IDISRPWC5R));
  fKeyToIdx.insert(make_pair((string)"disrpwc5",IDISRPWC5));
  fKeyToIdx.insert(make_pair((string)"disrpwc6r",IDISRPWC6R));
  fKeyToIdx.insert(make_pair((string)"disrpwc6",IDISRPWC6));
  fKeyToIdx.insert(make_pair((string)"disrpwc7r",IDISRPWC7R));
  fKeyToIdx.insert(make_pair((string)"disrpwc7",IDISRPWC7));
  fKeyToIdx.insert(make_pair((string)"disrpwc8r",IDISRPWC8R));
  fKeyToIdx.insert(make_pair((string)"disrpwc8",IDISRPWC8));
  //PION NARROW COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"dislpnc1r",IDISLPNC1R));
  fKeyToIdx.insert(make_pair((string)"dislpnc1",IDISLPNC1));
  fKeyToIdx.insert(make_pair((string)"dislpnc2r",IDISLPNC2R));
  fKeyToIdx.insert(make_pair((string)"dislpnc2",IDISLPNC2));
  fKeyToIdx.insert(make_pair((string)"dislpnc3r",IDISLPNC3R));
  fKeyToIdx.insert(make_pair((string)"dislpnc3",IDISLPNC3));
  fKeyToIdx.insert(make_pair((string)"dislpnc4r",IDISLPNC4R));
  fKeyToIdx.insert(make_pair((string)"dislpnc4",IDISLPNC4));
  fKeyToIdx.insert(make_pair((string)"dislpnc5r",IDISLPNC5R));
  fKeyToIdx.insert(make_pair((string)"dislpnc5",IDISLPNC5));
  fKeyToIdx.insert(make_pair((string)"dislpnc6r",IDISLPNC6R));
  fKeyToIdx.insert(make_pair((string)"dislpnc6",IDISLPNC6));
  fKeyToIdx.insert(make_pair((string)"dislpnc7r",IDISLPNC7R));
  fKeyToIdx.insert(make_pair((string)"dislpnc7",IDISLPNC7));
  fKeyToIdx.insert(make_pair((string)"dislpnc8r",IDISLPNC8R));
  fKeyToIdx.insert(make_pair((string)"dislpnc8",IDISLPNC8));
  //PION NARROW COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrpnc1r",IDISRPNC1R));
  fKeyToIdx.insert(make_pair((string)"disrpnc1",IDISRPNC1));
  fKeyToIdx.insert(make_pair((string)"disrpnc2r",IDISRPNC2R));
  fKeyToIdx.insert(make_pair((string)"disrpnc2",IDISRPNC2));
  fKeyToIdx.insert(make_pair((string)"disrpnc3r",IDISRPNC3R));
  fKeyToIdx.insert(make_pair((string)"disrpnc3",IDISRPNC3));
  fKeyToIdx.insert(make_pair((string)"disrpnc4r",IDISRPNC4R));
  fKeyToIdx.insert(make_pair((string)"disrpnc4",IDISRPNC4));
  fKeyToIdx.insert(make_pair((string)"disrpnc5r",IDISRPNC5R));
  fKeyToIdx.insert(make_pair((string)"disrpnc5",IDISRPNC5));
  fKeyToIdx.insert(make_pair((string)"disrpnc6r",IDISRPNC6R));
  fKeyToIdx.insert(make_pair((string)"disrpnc6",IDISRPNC6));
  fKeyToIdx.insert(make_pair((string)"disrpnc7r",IDISRPNC7R));
  fKeyToIdx.insert(make_pair((string)"disrpnc7",IDISRPNC7));
  fKeyToIdx.insert(make_pair((string)"disrpnc8r",IDISRPNC8R));
  fKeyToIdx.insert(make_pair((string)"disrpnc8",IDISRPNC8));
  //TAGGER COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"disltgc1r",IDISLTGC1R));
  fKeyToIdx.insert(make_pair((string)"disltgc1",IDISLTGC1));
  fKeyToIdx.insert(make_pair((string)"disltgc2r",IDISLTGC2R));
  fKeyToIdx.insert(make_pair((string)"disltgc2",IDISLTGC2));
  fKeyToIdx.insert(make_pair((string)"disltgc3r",IDISLTGC3R));
  fKeyToIdx.insert(make_pair((string)"disltgc3",IDISLTGC3));
  fKeyToIdx.insert(make_pair((string)"disltgc4r",IDISLTGC4R));
  fKeyToIdx.insert(make_pair((string)"disltgc4",IDISLTGC4));
  fKeyToIdx.insert(make_pair((string)"disltgc5r",IDISLTGC5R));
  fKeyToIdx.insert(make_pair((string)"disltgc5",IDISLTGC5));
  fKeyToIdx.insert(make_pair((string)"disltg6cr",IDISLTGC6R));
  fKeyToIdx.insert(make_pair((string)"disltgc6",IDISLTGC6));
  fKeyToIdx.insert(make_pair((string)"disltgc7r",IDISLTGC7R));
  fKeyToIdx.insert(make_pair((string)"disltgc7",IDISLTGC7));
  fKeyToIdx.insert(make_pair((string)"disltgc8r",IDISLTGC8R));
  fKeyToIdx.insert(make_pair((string)"disltgc8",IDISLTGC8));
  //TAGGER COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrtgc1r",IDISRTGC1R));
  fKeyToIdx.insert(make_pair((string)"disrtgc1",IDISRTGC1));
  fKeyToIdx.insert(make_pair((string)"disrtgc2r",IDISRTGC2R));
  fKeyToIdx.insert(make_pair((string)"disrtgc2",IDISRTGC2));
  fKeyToIdx.insert(make_pair((string)"disrtgc3r",IDISRTGC3R));
  fKeyToIdx.insert(make_pair((string)"disrtgc3",IDISRTGC3));
  fKeyToIdx.insert(make_pair((string)"disrtgc4r",IDISRTGC4R));
  fKeyToIdx.insert(make_pair((string)"disrtgc4",IDISRTGC4));
  fKeyToIdx.insert(make_pair((string)"disrtgc5r",IDISRTGC5R));
  fKeyToIdx.insert(make_pair((string)"disrtgc5",IDISRTGC5));
  fKeyToIdx.insert(make_pair((string)"disrtgc6r",IDISRTGC6R));
  fKeyToIdx.insert(make_pair((string)"disrtgc6",IDISRTGC6));
  fKeyToIdx.insert(make_pair((string)"disrtgc7r",IDISRTGC7R));
  fKeyToIdx.insert(make_pair((string)"disrtgc7",IDISRTGC7));
  fKeyToIdx.insert(make_pair((string)"disrtgc8r",IDISRTGC8R));
  fKeyToIdx.insert(make_pair((string)"disrtgc8",IDISRTGC8));
  //MIXED STUFF COPY L-HRS
  fKeyToIdx.insert(make_pair((string)"dislmxdc1r",IDISLMXDC1R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc1",IDISLMXDC1));
  fKeyToIdx.insert(make_pair((string)"dislmxdc2r",IDISLMXDC2R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc2",IDISLMXDC2));
  fKeyToIdx.insert(make_pair((string)"dislmxdc3r",IDISLMXDC3R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc3",IDISLMXDC3));
  fKeyToIdx.insert(make_pair((string)"dislmxdc4r",IDISLMXDC4R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc4",IDISLMXDC4));
  fKeyToIdx.insert(make_pair((string)"dislmxdc5r",IDISLMXDC5R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc5",IDISLMXDC5));
  fKeyToIdx.insert(make_pair((string)"dislmxdc6r",IDISLMXDC6R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc6",IDISLMXDC6));
  fKeyToIdx.insert(make_pair((string)"dislmxdc7r",IDISLMXDC7R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc7",IDISLMXDC7));
  fKeyToIdx.insert(make_pair((string)"dislmxdc8r",IDISLMXDC8R));
  fKeyToIdx.insert(make_pair((string)"dislmxdc8",IDISLMXDC8));
  //MIXED STUFF COPY R-HRS
  fKeyToIdx.insert(make_pair((string)"disrmxdc1r",IDISRMXDC1R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc1",IDISRMXDC1));
  fKeyToIdx.insert(make_pair((string)"disrmxdc2r",IDISRMXDC2R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc2",IDISRMXDC2));
  fKeyToIdx.insert(make_pair((string)"disrmxdc3r",IDISRMXDC3R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc3",IDISRMXDC3));
  fKeyToIdx.insert(make_pair((string)"disrmxdc4r",IDISRMXDC4R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc4",IDISRMXDC4));
  fKeyToIdx.insert(make_pair((string)"disrmxdc5r",IDISRMXDC5R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc5",IDISRMXDC5));
  fKeyToIdx.insert(make_pair((string)"disrmxdc6r",IDISRMXDC6R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc6",IDISRMXDC6));
  fKeyToIdx.insert(make_pair((string)"disrmxdc7r",IDISRMXDC7R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc7",IDISRMXDC7));
  fKeyToIdx.insert(make_pair((string)"disrmxdc8r",IDISRMXDC8R));
  fKeyToIdx.insert(make_pair((string)"disrmxdc8",IDISRMXDC8));


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

// Same thing all over for new ADCs
// ADCs first index is ADC slot, second is channel
// These are RAW data
  fKeyToIdx.insert(make_pair((string)"adcx0_0",IADCX0_0));
  fKeyToIdx.insert(make_pair((string)"adcx0_1",IADCX0_1));
  fKeyToIdx.insert(make_pair((string)"adcx0_2",IADCX0_2));
  fKeyToIdx.insert(make_pair((string)"adcx0_3",IADCX0_3));
  fKeyToIdx.insert(make_pair((string)"adcx1_0",IADCX1_0));
  fKeyToIdx.insert(make_pair((string)"adcx1_1",IADCX1_1));
  fKeyToIdx.insert(make_pair((string)"adcx1_2",IADCX1_2));
  fKeyToIdx.insert(make_pair((string)"adcx1_3",IADCX1_3));
  fKeyToIdx.insert(make_pair((string)"adcx2_0",IADCX2_0));
  fKeyToIdx.insert(make_pair((string)"adcx2_1",IADCX2_1));
  fKeyToIdx.insert(make_pair((string)"adcx2_2",IADCX2_2));
  fKeyToIdx.insert(make_pair((string)"adcx2_3",IADCX2_3));
  fKeyToIdx.insert(make_pair((string)"adcx3_0",IADCX3_0));
  fKeyToIdx.insert(make_pair((string)"adcx3_1",IADCX3_1));
  fKeyToIdx.insert(make_pair((string)"adcx3_2",IADCX3_2));
  fKeyToIdx.insert(make_pair((string)"adcx3_3",IADCX3_3));
  fKeyToIdx.insert(make_pair((string)"adcx4_0",IADCX4_0));
  fKeyToIdx.insert(make_pair((string)"adcx4_1",IADCX4_1));
  fKeyToIdx.insert(make_pair((string)"adcx4_2",IADCX4_2));
  fKeyToIdx.insert(make_pair((string)"adcx4_3",IADCX4_3));
  fKeyToIdx.insert(make_pair((string)"adcx5_0",IADCX5_0));
  fKeyToIdx.insert(make_pair((string)"adcx5_1",IADCX5_1));
  fKeyToIdx.insert(make_pair((string)"adcx5_2",IADCX5_2));
  fKeyToIdx.insert(make_pair((string)"adcx5_3",IADCX5_3));
  fKeyToIdx.insert(make_pair((string)"adcx6_0",IADCX6_0));
  fKeyToIdx.insert(make_pair((string)"adcx6_1",IADCX6_1));
  fKeyToIdx.insert(make_pair((string)"adcx6_2",IADCX6_2));
  fKeyToIdx.insert(make_pair((string)"adcx6_3",IADCX6_3));
  fKeyToIdx.insert(make_pair((string)"adcx7_0",IADCX7_0));
  fKeyToIdx.insert(make_pair((string)"adcx7_1",IADCX7_1));
  fKeyToIdx.insert(make_pair((string)"adcx7_2",IADCX7_2));
  fKeyToIdx.insert(make_pair((string)"adcx7_3",IADCX7_3));
  fKeyToIdx.insert(make_pair((string)"adcx8_0",IADCX8_0));
  fKeyToIdx.insert(make_pair((string)"adcx8_1",IADCX8_1));
  fKeyToIdx.insert(make_pair((string)"adcx8_2",IADCX8_2));
  fKeyToIdx.insert(make_pair((string)"adcx8_3",IADCX8_3));
  fKeyToIdx.insert(make_pair((string)"adcx9_0",IADCX9_0));
  fKeyToIdx.insert(make_pair((string)"adcx9_1",IADCX9_1));
  fKeyToIdx.insert(make_pair((string)"adcx9_2",IADCX9_2));
  fKeyToIdx.insert(make_pair((string)"adcx9_3",IADCX9_3));
  fKeyToIdx.insert(make_pair((string)"adcx10_0",IADCX10_0));
  fKeyToIdx.insert(make_pair((string)"adcx10_1",IADCX10_1));
  fKeyToIdx.insert(make_pair((string)"adcx10_2",IADCX10_2));
  fKeyToIdx.insert(make_pair((string)"adcx10_3",IADCX10_3));
  fKeyToIdx.insert(make_pair((string)"adcx11_0",IADCX11_0));
  fKeyToIdx.insert(make_pair((string)"adcx11_1",IADCX11_1));
  fKeyToIdx.insert(make_pair((string)"adcx11_2",IADCX11_2));
  fKeyToIdx.insert(make_pair((string)"adcx11_3",IADCX11_3));
  fKeyToIdx.insert(make_pair((string)"adcx12_0",IADCX12_0));
  fKeyToIdx.insert(make_pair((string)"adcx12_1",IADCX12_1));
  fKeyToIdx.insert(make_pair((string)"adcx12_2",IADCX12_2));
  fKeyToIdx.insert(make_pair((string)"adcx12_3",IADCX12_3));
  fKeyToIdx.insert(make_pair((string)"adcx13_0",IADCX13_0));
  fKeyToIdx.insert(make_pair((string)"adcx13_1",IADCX13_1));
  fKeyToIdx.insert(make_pair((string)"adcx13_2",IADCX13_2));
  fKeyToIdx.insert(make_pair((string)"adcx13_3",IADCX13_3));
  fKeyToIdx.insert(make_pair((string)"adcx14_0",IADCX14_0));
  fKeyToIdx.insert(make_pair((string)"adcx14_1",IADCX14_1));
  fKeyToIdx.insert(make_pair((string)"adcx14_2",IADCX14_2));
  fKeyToIdx.insert(make_pair((string)"adcx14_3",IADCX14_3));
  fKeyToIdx.insert(make_pair((string)"adcx15_0",IADCX15_0));
  fKeyToIdx.insert(make_pair((string)"adcx15_1",IADCX15_1));
  fKeyToIdx.insert(make_pair((string)"adcx15_2",IADCX15_2));
  fKeyToIdx.insert(make_pair((string)"adcx15_3",IADCX15_3));
  fKeyToIdx.insert(make_pair((string)"adcx16_0",IADCX16_0));
  fKeyToIdx.insert(make_pair((string)"adcx16_1",IADCX16_1));
  fKeyToIdx.insert(make_pair((string)"adcx16_2",IADCX16_2));
  fKeyToIdx.insert(make_pair((string)"adcx16_3",IADCX16_3));
  fKeyToIdx.insert(make_pair((string)"adcx17_0",IADCX17_0));
  fKeyToIdx.insert(make_pair((string)"adcx17_1",IADCX17_1));
  fKeyToIdx.insert(make_pair((string)"adcx17_2",IADCX17_2));
  fKeyToIdx.insert(make_pair((string)"adcx17_3",IADCX17_3));
  fKeyToIdx.insert(make_pair((string)"adcx18_0",IADCX18_0));
  fKeyToIdx.insert(make_pair((string)"adcx18_1",IADCX18_1));
  fKeyToIdx.insert(make_pair((string)"adcx18_2",IADCX18_2));
  fKeyToIdx.insert(make_pair((string)"adcx18_3",IADCX18_3));
  fKeyToIdx.insert(make_pair((string)"adcx19_0",IADCX19_0));
  fKeyToIdx.insert(make_pair((string)"adcx19_1",IADCX19_1));
  fKeyToIdx.insert(make_pair((string)"adcx19_2",IADCX19_2));
  fKeyToIdx.insert(make_pair((string)"adcx19_3",IADCX19_3));
  fKeyToIdx.insert(make_pair((string)"adcx20_0",IADCX20_0));
  fKeyToIdx.insert(make_pair((string)"adcx20_1",IADCX20_1));
  fKeyToIdx.insert(make_pair((string)"adcx20_2",IADCX20_2));
  fKeyToIdx.insert(make_pair((string)"adcx20_3",IADCX20_3));
  fKeyToIdx.insert(make_pair((string)"adcx21_0",IADCX21_0));
  fKeyToIdx.insert(make_pair((string)"adcx21_1",IADCX21_1));
  fKeyToIdx.insert(make_pair((string)"adcx21_2",IADCX21_2));
  fKeyToIdx.insert(make_pair((string)"adcx21_3",IADCX21_3));
  fKeyToIdx.insert(make_pair((string)"adcx22_0",IADCX22_0));
  fKeyToIdx.insert(make_pair((string)"adcx22_1",IADCX22_1));
  fKeyToIdx.insert(make_pair((string)"adcx22_2",IADCX22_2));
  fKeyToIdx.insert(make_pair((string)"adcx22_3",IADCX22_3));
  fKeyToIdx.insert(make_pair((string)"adcx23_0",IADCX23_0));
  fKeyToIdx.insert(make_pair((string)"adcx23_1",IADCX23_1));
  fKeyToIdx.insert(make_pair((string)"adcx23_2",IADCX23_2));
  fKeyToIdx.insert(make_pair((string)"adcx23_3",IADCX23_3));
  fKeyToIdx.insert(make_pair((string)"adcx24_0",IADCX24_0));
  fKeyToIdx.insert(make_pair((string)"adcx24_1",IADCX24_1));
  fKeyToIdx.insert(make_pair((string)"adcx24_2",IADCX24_2));
  fKeyToIdx.insert(make_pair((string)"adcx24_3",IADCX24_3));
  fKeyToIdx.insert(make_pair((string)"adcx25_0",IADCX25_0));
  fKeyToIdx.insert(make_pair((string)"adcx25_1",IADCX25_1));
  fKeyToIdx.insert(make_pair((string)"adcx25_2",IADCX25_2));
  fKeyToIdx.insert(make_pair((string)"adcx25_3",IADCX25_3));
  fKeyToIdx.insert(make_pair((string)"adcx26_0",IADCX26_0));
  fKeyToIdx.insert(make_pair((string)"adcx26_1",IADCX26_1));
  fKeyToIdx.insert(make_pair((string)"adcx26_2",IADCX26_2));
  fKeyToIdx.insert(make_pair((string)"adcx26_3",IADCX26_3));
  fKeyToIdx.insert(make_pair((string)"adcx27_0",IADCX27_0));
  fKeyToIdx.insert(make_pair((string)"adcx27_1",IADCX27_1));
  fKeyToIdx.insert(make_pair((string)"adcx27_2",IADCX27_2));
  fKeyToIdx.insert(make_pair((string)"adcx27_3",IADCX27_3));
  fKeyToIdx.insert(make_pair((string)"adcx28_0",IADCX28_0));
  fKeyToIdx.insert(make_pair((string)"adcx28_1",IADCX28_1));
  fKeyToIdx.insert(make_pair((string)"adcx28_2",IADCX28_2));
  fKeyToIdx.insert(make_pair((string)"adcx28_3",IADCX28_3));
  fKeyToIdx.insert(make_pair((string)"adcx29_0",IADCX29_0));
  fKeyToIdx.insert(make_pair((string)"adcx29_1",IADCX29_1));
  fKeyToIdx.insert(make_pair((string)"adcx29_2",IADCX29_2));
  fKeyToIdx.insert(make_pair((string)"adcx29_3",IADCX29_3));
  fKeyToIdx.insert(make_pair((string)"adcx30_0",IADCX30_0));
  fKeyToIdx.insert(make_pair((string)"adcx30_1",IADCX30_1));
  fKeyToIdx.insert(make_pair((string)"adcx30_2",IADCX30_2));
  fKeyToIdx.insert(make_pair((string)"adcx30_3",IADCX30_3));

// These are dacnoise subtracted data (no pedestal subtraction)
  fKeyToIdx.insert(make_pair((string)"adcx0_0_dacsub",IADCX0_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_dacsub",IADCX0_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_dacsub",IADCX0_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_dacsub",IADCX0_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_dacsub",IADCX1_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_dacsub",IADCX1_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_dacsub",IADCX1_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_dacsub",IADCX1_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_dacsub",IADCX2_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_dacsub",IADCX2_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_dacsub",IADCX2_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_dacsub",IADCX2_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_dacsub",IADCX3_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_dacsub",IADCX3_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_dacsub",IADCX3_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_dacsub",IADCX3_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_dacsub",IADCX4_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_dacsub",IADCX4_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_dacsub",IADCX4_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_dacsub",IADCX4_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_dacsub",IADCX5_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_dacsub",IADCX5_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_dacsub",IADCX5_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_dacsub",IADCX5_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_dacsub",IADCX6_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_dacsub",IADCX6_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_dacsub",IADCX6_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_dacsub",IADCX6_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_dacsub",IADCX7_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_dacsub",IADCX7_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_dacsub",IADCX7_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_dacsub",IADCX7_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_dacsub",IADCX8_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_dacsub",IADCX8_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_dacsub",IADCX8_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_dacsub",IADCX8_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_dacsub",IADCX9_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_dacsub",IADCX9_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_dacsub",IADCX9_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_dacsub",IADCX9_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_dacsub",IADCX10_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_dacsub",IADCX10_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_dacsub",IADCX10_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_dacsub",IADCX10_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_dacsub",IADCX11_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_dacsub",IADCX11_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_dacsub",IADCX11_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_dacsub",IADCX11_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_dacsub",IADCX12_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_dacsub",IADCX12_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_dacsub",IADCX12_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_dacsub",IADCX12_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_dacsub",IADCX13_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_dacsub",IADCX13_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_dacsub",IADCX13_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_dacsub",IADCX13_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_dacsub",IADCX14_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_dacsub",IADCX14_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_dacsub",IADCX14_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_dacsub",IADCX14_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_dacsub",IADCX15_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_dacsub",IADCX15_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_dacsub",IADCX15_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_dacsub",IADCX15_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_dacsub",IADCX16_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_dacsub",IADCX16_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_dacsub",IADCX16_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_dacsub",IADCX16_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_dacsub",IADCX17_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_dacsub",IADCX17_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_dacsub",IADCX17_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_dacsub",IADCX17_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_dacsub",IADCX18_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_dacsub",IADCX18_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_dacsub",IADCX18_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_dacsub",IADCX18_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_dacsub",IADCX19_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_dacsub",IADCX19_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_dacsub",IADCX19_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_dacsub",IADCX19_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_dacsub",IADCX20_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_dacsub",IADCX20_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_dacsub",IADCX20_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_dacsub",IADCX20_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_dacsub",IADCX21_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_dacsub",IADCX21_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_dacsub",IADCX21_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_dacsub",IADCX21_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_dacsub",IADCX22_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_dacsub",IADCX22_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_dacsub",IADCX22_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_dacsub",IADCX22_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_dacsub",IADCX23_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_dacsub",IADCX23_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_dacsub",IADCX23_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_dacsub",IADCX23_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_dacsub",IADCX24_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_dacsub",IADCX24_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_dacsub",IADCX24_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_dacsub",IADCX24_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_dacsub",IADCX25_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_dacsub",IADCX25_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_dacsub",IADCX25_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_dacsub",IADCX25_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_dacsub",IADCX26_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_dacsub",IADCX26_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_dacsub",IADCX26_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_dacsub",IADCX26_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_dacsub",IADCX27_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_dacsub",IADCX27_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_dacsub",IADCX27_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_dacsub",IADCX27_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_dacsub",IADCX28_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_dacsub",IADCX28_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_dacsub",IADCX28_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_dacsub",IADCX28_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_dacsub",IADCX29_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_dacsub",IADCX29_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_dacsub",IADCX29_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_dacsub",IADCX29_3_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_dacsub",IADCX30_0_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_dacsub",IADCX30_1_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_dacsub",IADCX30_2_DACSUB));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_dacsub",IADCX30_3_DACSUB));

// These are CALIBRATED data (dac noise and pedestal subtracted)
  fKeyToIdx.insert(make_pair((string)"adcx0_0_cal",IADCX0_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_cal",IADCX0_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_cal",IADCX0_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_cal",IADCX0_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_cal",IADCX1_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_cal",IADCX1_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_cal",IADCX1_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_cal",IADCX1_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_cal",IADCX2_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_cal",IADCX2_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_cal",IADCX2_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_cal",IADCX2_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_cal",IADCX3_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_cal",IADCX3_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_cal",IADCX3_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_cal",IADCX3_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_cal",IADCX4_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_cal",IADCX4_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_cal",IADCX4_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_cal",IADCX4_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_cal",IADCX5_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_cal",IADCX5_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_cal",IADCX5_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_cal",IADCX5_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_cal",IADCX6_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_cal",IADCX6_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_cal",IADCX6_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_cal",IADCX6_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_cal",IADCX7_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_cal",IADCX7_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_cal",IADCX7_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_cal",IADCX7_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_cal",IADCX8_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_cal",IADCX8_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_cal",IADCX8_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_cal",IADCX8_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_cal",IADCX9_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_cal",IADCX9_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_cal",IADCX9_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_cal",IADCX9_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_cal",IADCX10_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_cal",IADCX10_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_cal",IADCX10_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_cal",IADCX10_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_cal",IADCX11_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_cal",IADCX11_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_cal",IADCX11_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_cal",IADCX11_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_cal",IADCX12_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_cal",IADCX12_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_cal",IADCX12_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_cal",IADCX12_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_cal",IADCX13_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_cal",IADCX13_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_cal",IADCX13_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_cal",IADCX13_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_cal",IADCX14_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_cal",IADCX14_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_cal",IADCX14_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_cal",IADCX14_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_cal",IADCX15_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_cal",IADCX15_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_cal",IADCX15_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_cal",IADCX15_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_cal",IADCX16_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_cal",IADCX16_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_cal",IADCX16_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_cal",IADCX16_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_cal",IADCX17_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_cal",IADCX17_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_cal",IADCX17_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_cal",IADCX17_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_cal",IADCX18_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_cal",IADCX18_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_cal",IADCX18_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_cal",IADCX18_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_cal",IADCX19_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_cal",IADCX19_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_cal",IADCX19_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_cal",IADCX19_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_cal",IADCX20_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_cal",IADCX20_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_cal",IADCX20_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_cal",IADCX20_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_cal",IADCX21_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_cal",IADCX21_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_cal",IADCX21_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_cal",IADCX21_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_cal",IADCX22_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_cal",IADCX22_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_cal",IADCX22_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_cal",IADCX22_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_cal",IADCX23_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_cal",IADCX23_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_cal",IADCX23_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_cal",IADCX23_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_cal",IADCX24_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_cal",IADCX24_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_cal",IADCX24_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_cal",IADCX24_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_cal",IADCX25_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_cal",IADCX25_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_cal",IADCX25_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_cal",IADCX25_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_cal",IADCX26_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_cal",IADCX26_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_cal",IADCX26_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_cal",IADCX26_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_cal",IADCX27_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_cal",IADCX27_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_cal",IADCX27_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_cal",IADCX27_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_cal",IADCX28_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_cal",IADCX28_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_cal",IADCX28_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_cal",IADCX28_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_cal",IADCX29_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_cal",IADCX29_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_cal",IADCX29_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_cal",IADCX29_3_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_cal",IADCX30_0_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_cal",IADCX30_1_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_cal",IADCX30_2_CAL));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_cal",IADCX30_3_CAL));

// These are baseline sample data
  fKeyToIdx.insert(make_pair((string)"adcx0_0_b0",IADCX0_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_b1",IADCX0_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_b2",IADCX0_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_b3",IADCX0_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_b0",IADCX0_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_b1",IADCX0_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_b2",IADCX0_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_b3",IADCX0_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_b0",IADCX0_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_b1",IADCX0_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_b2",IADCX0_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_b3",IADCX0_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_b0",IADCX0_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_b1",IADCX0_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_b2",IADCX0_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_b3",IADCX0_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_b0",IADCX1_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_b1",IADCX1_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_b2",IADCX1_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_b3",IADCX1_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_b0",IADCX1_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_b1",IADCX1_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_b2",IADCX1_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_b3",IADCX1_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_b0",IADCX1_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_b1",IADCX1_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_b2",IADCX1_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_b3",IADCX1_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_b0",IADCX1_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_b1",IADCX1_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_b2",IADCX1_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_b3",IADCX1_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_b0",IADCX2_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_b1",IADCX2_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_b2",IADCX2_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_b3",IADCX2_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_b0",IADCX2_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_b1",IADCX2_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_b2",IADCX2_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_b3",IADCX2_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_b0",IADCX2_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_b1",IADCX2_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_b2",IADCX2_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_b3",IADCX2_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_b0",IADCX2_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_b1",IADCX2_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_b2",IADCX2_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_b3",IADCX2_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_b0",IADCX3_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_b1",IADCX3_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_b2",IADCX3_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_b3",IADCX3_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_b0",IADCX3_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_b1",IADCX3_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_b2",IADCX3_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_b3",IADCX3_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_b0",IADCX3_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_b1",IADCX3_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_b2",IADCX3_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_b3",IADCX3_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_b0",IADCX3_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_b1",IADCX3_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_b2",IADCX3_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_b3",IADCX3_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_b0",IADCX4_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_b1",IADCX4_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_b2",IADCX4_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_b3",IADCX4_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_b0",IADCX4_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_b1",IADCX4_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_b2",IADCX4_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_b3",IADCX4_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_b0",IADCX4_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_b1",IADCX4_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_b2",IADCX4_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_b3",IADCX4_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_b0",IADCX4_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_b1",IADCX4_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_b2",IADCX4_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_b3",IADCX4_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_b0",IADCX5_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_b1",IADCX5_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_b2",IADCX5_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_b3",IADCX5_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_b0",IADCX5_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_b1",IADCX5_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_b2",IADCX5_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_b3",IADCX5_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_b0",IADCX5_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_b1",IADCX5_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_b2",IADCX5_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_b3",IADCX5_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_b0",IADCX5_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_b1",IADCX5_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_b2",IADCX5_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_b3",IADCX5_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_b0",IADCX6_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_b1",IADCX6_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_b2",IADCX6_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_b3",IADCX6_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_b0",IADCX6_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_b1",IADCX6_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_b2",IADCX6_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_b3",IADCX6_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_b0",IADCX6_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_b1",IADCX6_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_b2",IADCX6_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_b3",IADCX6_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_b0",IADCX6_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_b1",IADCX6_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_b2",IADCX6_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_b3",IADCX6_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_b0",IADCX7_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_b1",IADCX7_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_b2",IADCX7_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_b3",IADCX7_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_b0",IADCX7_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_b1",IADCX7_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_b2",IADCX7_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_b3",IADCX7_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_b0",IADCX7_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_b1",IADCX7_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_b2",IADCX7_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_b3",IADCX7_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_b0",IADCX7_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_b1",IADCX7_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_b2",IADCX7_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_b3",IADCX7_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_b0",IADCX8_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_b1",IADCX8_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_b2",IADCX8_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_b3",IADCX8_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_b0",IADCX8_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_b1",IADCX8_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_b2",IADCX8_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_b3",IADCX8_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_b0",IADCX8_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_b1",IADCX8_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_b2",IADCX8_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_b3",IADCX8_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_b0",IADCX8_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_b1",IADCX8_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_b2",IADCX8_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_b3",IADCX8_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_b0",IADCX9_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_b1",IADCX9_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_b2",IADCX9_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_b3",IADCX9_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_b0",IADCX9_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_b1",IADCX9_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_b2",IADCX9_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_b3",IADCX9_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_b0",IADCX9_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_b1",IADCX9_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_b2",IADCX9_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_b3",IADCX9_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_b0",IADCX9_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_b1",IADCX9_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_b2",IADCX9_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_b3",IADCX9_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_b0",IADCX10_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_b1",IADCX10_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_b2",IADCX10_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_b3",IADCX10_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_b0",IADCX10_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_b1",IADCX10_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_b2",IADCX10_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_b3",IADCX10_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_b0",IADCX10_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_b1",IADCX10_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_b2",IADCX10_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_b3",IADCX10_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_b0",IADCX10_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_b1",IADCX10_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_b2",IADCX10_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_b3",IADCX10_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_b0",IADCX11_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_b1",IADCX11_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_b2",IADCX11_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_b3",IADCX11_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_b0",IADCX11_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_b1",IADCX11_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_b2",IADCX11_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_b3",IADCX11_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_b0",IADCX11_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_b1",IADCX11_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_b2",IADCX11_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_b3",IADCX11_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_b0",IADCX11_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_b1",IADCX11_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_b2",IADCX11_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_b3",IADCX11_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_b0",IADCX12_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_b1",IADCX12_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_b2",IADCX12_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_b3",IADCX12_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_b0",IADCX12_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_b1",IADCX12_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_b2",IADCX12_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_b3",IADCX12_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_b0",IADCX12_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_b1",IADCX12_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_b2",IADCX12_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_b3",IADCX12_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_b0",IADCX12_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_b1",IADCX12_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_b2",IADCX12_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_b3",IADCX12_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_b0",IADCX13_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_b1",IADCX13_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_b2",IADCX13_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_b3",IADCX13_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_b0",IADCX13_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_b1",IADCX13_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_b2",IADCX13_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_b3",IADCX13_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_b0",IADCX13_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_b1",IADCX13_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_b2",IADCX13_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_b3",IADCX13_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_b0",IADCX13_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_b1",IADCX13_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_b2",IADCX13_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_b3",IADCX13_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_b0",IADCX14_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_b1",IADCX14_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_b2",IADCX14_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_b3",IADCX14_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_b0",IADCX14_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_b1",IADCX14_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_b2",IADCX14_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_b3",IADCX14_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_b0",IADCX14_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_b1",IADCX14_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_b2",IADCX14_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_b3",IADCX14_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_b0",IADCX14_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_b1",IADCX14_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_b2",IADCX14_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_b3",IADCX14_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_b0",IADCX15_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_b1",IADCX15_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_b2",IADCX15_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_b3",IADCX15_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_b0",IADCX15_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_b1",IADCX15_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_b2",IADCX15_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_b3",IADCX15_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_b0",IADCX15_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_b1",IADCX15_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_b2",IADCX15_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_b3",IADCX15_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_b0",IADCX15_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_b1",IADCX15_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_b2",IADCX15_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_b3",IADCX15_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_b0",IADCX16_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_b1",IADCX16_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_b2",IADCX16_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_b3",IADCX16_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_b0",IADCX16_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_b1",IADCX16_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_b2",IADCX16_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_b3",IADCX16_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_b0",IADCX16_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_b1",IADCX16_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_b2",IADCX16_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_b3",IADCX16_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_b0",IADCX16_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_b1",IADCX16_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_b2",IADCX16_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_b3",IADCX16_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_b0",IADCX17_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_b1",IADCX17_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_b2",IADCX17_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_b3",IADCX17_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_b0",IADCX17_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_b1",IADCX17_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_b2",IADCX17_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_b3",IADCX17_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_b0",IADCX17_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_b1",IADCX17_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_b2",IADCX17_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_b3",IADCX17_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_b0",IADCX17_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_b1",IADCX17_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_b2",IADCX17_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_b3",IADCX17_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_b0",IADCX18_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_b1",IADCX18_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_b2",IADCX18_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_b3",IADCX18_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_b0",IADCX18_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_b1",IADCX18_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_b2",IADCX18_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_b3",IADCX18_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_b0",IADCX18_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_b1",IADCX18_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_b2",IADCX18_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_b3",IADCX18_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_b0",IADCX18_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_b1",IADCX18_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_b2",IADCX18_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_b3",IADCX18_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_b0",IADCX19_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_b1",IADCX19_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_b2",IADCX19_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_b3",IADCX19_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_b0",IADCX19_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_b1",IADCX19_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_b2",IADCX19_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_b3",IADCX19_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_b0",IADCX19_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_b1",IADCX19_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_b2",IADCX19_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_b3",IADCX19_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_b0",IADCX19_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_b1",IADCX19_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_b2",IADCX19_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_b3",IADCX19_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_b0",IADCX20_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_b1",IADCX20_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_b2",IADCX20_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_b3",IADCX20_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_b0",IADCX20_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_b1",IADCX20_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_b2",IADCX20_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_b3",IADCX20_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_b0",IADCX20_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_b1",IADCX20_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_b2",IADCX20_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_b3",IADCX20_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_b0",IADCX20_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_b1",IADCX20_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_b2",IADCX20_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_b3",IADCX20_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_b0",IADCX21_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_b1",IADCX21_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_b2",IADCX21_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_b3",IADCX21_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_b0",IADCX21_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_b1",IADCX21_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_b2",IADCX21_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_b3",IADCX21_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_b0",IADCX21_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_b1",IADCX21_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_b2",IADCX21_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_b3",IADCX21_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_b0",IADCX21_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_b1",IADCX21_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_b2",IADCX21_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_b3",IADCX21_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_b0",IADCX22_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_b1",IADCX22_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_b2",IADCX22_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_b3",IADCX22_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_b0",IADCX22_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_b1",IADCX22_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_b2",IADCX22_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_b3",IADCX22_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_b0",IADCX22_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_b1",IADCX22_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_b2",IADCX22_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_b3",IADCX22_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_b0",IADCX22_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_b1",IADCX22_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_b2",IADCX22_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_b3",IADCX22_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_b0",IADCX23_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_b1",IADCX23_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_b2",IADCX23_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_b3",IADCX23_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_b0",IADCX23_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_b1",IADCX23_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_b2",IADCX23_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_b3",IADCX23_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_b0",IADCX23_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_b1",IADCX23_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_b2",IADCX23_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_b3",IADCX23_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_b0",IADCX23_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_b1",IADCX23_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_b2",IADCX23_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_b3",IADCX23_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_b0",IADCX24_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_b1",IADCX24_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_b2",IADCX24_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_b3",IADCX24_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_b0",IADCX24_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_b1",IADCX24_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_b2",IADCX24_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_b3",IADCX24_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_b0",IADCX24_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_b1",IADCX24_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_b2",IADCX24_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_b3",IADCX24_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_b0",IADCX24_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_b1",IADCX24_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_b2",IADCX24_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_b3",IADCX24_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_b0",IADCX25_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_b1",IADCX25_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_b2",IADCX25_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_b3",IADCX25_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_b0",IADCX25_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_b1",IADCX25_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_b2",IADCX25_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_b3",IADCX25_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_b0",IADCX25_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_b1",IADCX25_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_b2",IADCX25_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_b3",IADCX25_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_b0",IADCX25_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_b1",IADCX25_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_b2",IADCX25_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_b3",IADCX25_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_b0",IADCX26_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_b1",IADCX26_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_b2",IADCX26_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_b3",IADCX26_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_b0",IADCX26_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_b1",IADCX26_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_b2",IADCX26_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_b3",IADCX26_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_b0",IADCX26_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_b1",IADCX26_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_b2",IADCX26_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_b3",IADCX26_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_b0",IADCX26_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_b1",IADCX26_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_b2",IADCX26_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_b3",IADCX26_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_b0",IADCX27_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_b1",IADCX27_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_b2",IADCX27_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_b3",IADCX27_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_b0",IADCX27_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_b1",IADCX27_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_b2",IADCX27_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_b3",IADCX27_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_b0",IADCX27_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_b1",IADCX27_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_b2",IADCX27_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_b3",IADCX27_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_b0",IADCX27_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_b1",IADCX27_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_b2",IADCX27_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_b3",IADCX27_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_b0",IADCX28_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_b1",IADCX28_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_b2",IADCX28_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_b3",IADCX28_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_b0",IADCX28_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_b1",IADCX28_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_b2",IADCX28_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_b3",IADCX28_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_b0",IADCX28_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_b1",IADCX28_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_b2",IADCX28_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_b3",IADCX28_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_b0",IADCX28_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_b1",IADCX28_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_b2",IADCX28_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_b3",IADCX28_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_b0",IADCX29_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_b1",IADCX29_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_b2",IADCX29_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_b3",IADCX29_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_b0",IADCX29_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_b1",IADCX29_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_b2",IADCX29_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_b3",IADCX29_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_b0",IADCX29_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_b1",IADCX29_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_b2",IADCX29_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_b3",IADCX29_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_b0",IADCX29_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_b1",IADCX29_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_b2",IADCX29_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_b3",IADCX29_3_B3));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_b0",IADCX30_0_B0));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_b1",IADCX30_0_B1));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_b2",IADCX30_0_B2));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_b3",IADCX30_0_B3));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_b0",IADCX30_1_B0));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_b1",IADCX30_1_B1));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_b2",IADCX30_1_B2));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_b3",IADCX30_1_B3));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_b0",IADCX30_2_B0));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_b1",IADCX30_2_B1));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_b2",IADCX30_2_B2));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_b3",IADCX30_2_B3));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_b0",IADCX30_3_B0));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_b1",IADCX30_3_B1));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_b2",IADCX30_3_B2));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_b3",IADCX30_3_B3));

// These are baseline sample data
  fKeyToIdx.insert(make_pair((string)"adcx0_0_p0",IADCX0_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_p1",IADCX0_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_p2",IADCX0_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx0_0_p3",IADCX0_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_p0",IADCX0_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_p1",IADCX0_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_p2",IADCX0_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx0_1_p3",IADCX0_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_p0",IADCX0_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_p1",IADCX0_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_p2",IADCX0_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx0_2_p3",IADCX0_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_p0",IADCX0_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_p1",IADCX0_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_p2",IADCX0_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx0_3_p3",IADCX0_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_p0",IADCX1_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_p1",IADCX1_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_p2",IADCX1_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx1_0_p3",IADCX1_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_p0",IADCX1_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_p1",IADCX1_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_p2",IADCX1_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx1_1_p3",IADCX1_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_p0",IADCX1_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_p1",IADCX1_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_p2",IADCX1_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx1_2_p3",IADCX1_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_p0",IADCX1_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_p1",IADCX1_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_p2",IADCX1_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx1_3_p3",IADCX1_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_p0",IADCX2_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_p1",IADCX2_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_p2",IADCX2_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx2_0_p3",IADCX2_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_p0",IADCX2_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_p1",IADCX2_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_p2",IADCX2_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx2_1_p3",IADCX2_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_p0",IADCX2_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_p1",IADCX2_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_p2",IADCX2_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx2_2_p3",IADCX2_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_p0",IADCX2_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_p1",IADCX2_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_p2",IADCX2_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx2_3_p3",IADCX2_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_p0",IADCX3_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_p1",IADCX3_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_p2",IADCX3_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx3_0_p3",IADCX3_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_p0",IADCX3_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_p1",IADCX3_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_p2",IADCX3_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx3_1_p3",IADCX3_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_p0",IADCX3_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_p1",IADCX3_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_p2",IADCX3_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx3_2_p3",IADCX3_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_p0",IADCX3_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_p1",IADCX3_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_p2",IADCX3_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx3_3_p3",IADCX3_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_p0",IADCX4_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_p1",IADCX4_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_p2",IADCX4_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx4_0_p3",IADCX4_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_p0",IADCX4_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_p1",IADCX4_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_p2",IADCX4_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx4_1_p3",IADCX4_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_p0",IADCX4_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_p1",IADCX4_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_p2",IADCX4_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx4_2_p3",IADCX4_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_p0",IADCX4_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_p1",IADCX4_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_p2",IADCX4_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx4_3_p3",IADCX4_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_p0",IADCX5_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_p1",IADCX5_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_p2",IADCX5_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx5_0_p3",IADCX5_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_p0",IADCX5_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_p1",IADCX5_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_p2",IADCX5_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx5_1_p3",IADCX5_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_p0",IADCX5_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_p1",IADCX5_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_p2",IADCX5_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx5_2_p3",IADCX5_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_p0",IADCX5_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_p1",IADCX5_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_p2",IADCX5_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx5_3_p3",IADCX5_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_p0",IADCX6_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_p1",IADCX6_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_p2",IADCX6_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx6_0_p3",IADCX6_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_p0",IADCX6_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_p1",IADCX6_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_p2",IADCX6_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx6_1_p3",IADCX6_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_p0",IADCX6_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_p1",IADCX6_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_p2",IADCX6_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx6_2_p3",IADCX6_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_p0",IADCX6_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_p1",IADCX6_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_p2",IADCX6_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx6_3_p3",IADCX6_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_p0",IADCX7_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_p1",IADCX7_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_p2",IADCX7_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx7_0_p3",IADCX7_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_p0",IADCX7_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_p1",IADCX7_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_p2",IADCX7_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx7_1_p3",IADCX7_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_p0",IADCX7_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_p1",IADCX7_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_p2",IADCX7_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx7_2_p3",IADCX7_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_p0",IADCX7_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_p1",IADCX7_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_p2",IADCX7_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx7_3_p3",IADCX7_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_p0",IADCX8_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_p1",IADCX8_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_p2",IADCX8_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx8_0_p3",IADCX8_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_p0",IADCX8_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_p1",IADCX8_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_p2",IADCX8_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx8_1_p3",IADCX8_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_p0",IADCX8_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_p1",IADCX8_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_p2",IADCX8_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx8_2_p3",IADCX8_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_p0",IADCX8_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_p1",IADCX8_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_p2",IADCX8_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx8_3_p3",IADCX8_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_p0",IADCX9_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_p1",IADCX9_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_p2",IADCX9_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx9_0_p3",IADCX9_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_p0",IADCX9_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_p1",IADCX9_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_p2",IADCX9_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx9_1_p3",IADCX9_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_p0",IADCX9_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_p1",IADCX9_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_p2",IADCX9_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx9_2_p3",IADCX9_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_p0",IADCX9_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_p1",IADCX9_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_p2",IADCX9_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx9_3_p3",IADCX9_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_p0",IADCX10_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_p1",IADCX10_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_p2",IADCX10_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx10_0_p3",IADCX10_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_p0",IADCX10_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_p1",IADCX10_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_p2",IADCX10_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx10_1_p3",IADCX10_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_p0",IADCX10_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_p1",IADCX10_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_p2",IADCX10_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx10_2_p3",IADCX10_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_p0",IADCX10_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_p1",IADCX10_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_p2",IADCX10_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx10_3_p3",IADCX10_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_p0",IADCX11_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_p1",IADCX11_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_p2",IADCX11_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx11_0_p3",IADCX11_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_p0",IADCX11_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_p1",IADCX11_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_p2",IADCX11_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx11_1_p3",IADCX11_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_p0",IADCX11_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_p1",IADCX11_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_p2",IADCX11_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx11_2_p3",IADCX11_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_p0",IADCX11_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_p1",IADCX11_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_p2",IADCX11_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx11_3_p3",IADCX11_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_p0",IADCX12_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_p1",IADCX12_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_p2",IADCX12_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx12_0_p3",IADCX12_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_p0",IADCX12_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_p1",IADCX12_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_p2",IADCX12_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx12_1_p3",IADCX12_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_p0",IADCX12_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_p1",IADCX12_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_p2",IADCX12_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx12_2_p3",IADCX12_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_p0",IADCX12_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_p1",IADCX12_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_p2",IADCX12_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx12_3_p3",IADCX12_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_p0",IADCX13_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_p1",IADCX13_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_p2",IADCX13_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx13_0_p3",IADCX13_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_p0",IADCX13_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_p1",IADCX13_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_p2",IADCX13_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx13_1_p3",IADCX13_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_p0",IADCX13_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_p1",IADCX13_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_p2",IADCX13_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx13_2_p3",IADCX13_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_p0",IADCX13_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_p1",IADCX13_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_p2",IADCX13_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx13_3_p3",IADCX13_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_p0",IADCX14_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_p1",IADCX14_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_p2",IADCX14_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx14_0_p3",IADCX14_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_p0",IADCX14_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_p1",IADCX14_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_p2",IADCX14_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx14_1_p3",IADCX14_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_p0",IADCX14_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_p1",IADCX14_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_p2",IADCX14_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx14_2_p3",IADCX14_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_p0",IADCX14_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_p1",IADCX14_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_p2",IADCX14_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx14_3_p3",IADCX14_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_p0",IADCX15_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_p1",IADCX15_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_p2",IADCX15_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx15_0_p3",IADCX15_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_p0",IADCX15_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_p1",IADCX15_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_p2",IADCX15_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx15_1_p3",IADCX15_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_p0",IADCX15_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_p1",IADCX15_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_p2",IADCX15_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx15_2_p3",IADCX15_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_p0",IADCX15_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_p1",IADCX15_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_p2",IADCX15_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx15_3_p3",IADCX15_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_p0",IADCX16_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_p1",IADCX16_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_p2",IADCX16_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx16_0_p3",IADCX16_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_p0",IADCX16_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_p1",IADCX16_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_p2",IADCX16_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx16_1_p3",IADCX16_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_p0",IADCX16_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_p1",IADCX16_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_p2",IADCX16_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx16_2_p3",IADCX16_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_p0",IADCX16_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_p1",IADCX16_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_p2",IADCX16_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx16_3_p3",IADCX16_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_p0",IADCX17_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_p1",IADCX17_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_p2",IADCX17_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx17_0_p3",IADCX17_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_p0",IADCX17_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_p1",IADCX17_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_p2",IADCX17_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx17_1_p3",IADCX17_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_p0",IADCX17_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_p1",IADCX17_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_p2",IADCX17_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx17_2_p3",IADCX17_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_p0",IADCX17_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_p1",IADCX17_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_p2",IADCX17_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx17_3_p3",IADCX17_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_p0",IADCX18_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_p1",IADCX18_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_p2",IADCX18_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx18_0_p3",IADCX18_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_p0",IADCX18_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_p1",IADCX18_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_p2",IADCX18_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx18_1_p3",IADCX18_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_p0",IADCX18_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_p1",IADCX18_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_p2",IADCX18_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx18_2_p3",IADCX18_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_p0",IADCX18_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_p1",IADCX18_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_p2",IADCX18_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx18_3_p3",IADCX18_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_p0",IADCX19_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_p1",IADCX19_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_p2",IADCX19_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx19_0_p3",IADCX19_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_p0",IADCX19_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_p1",IADCX19_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_p2",IADCX19_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx19_1_p3",IADCX19_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_p0",IADCX19_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_p1",IADCX19_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_p2",IADCX19_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx19_2_p3",IADCX19_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_p0",IADCX19_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_p1",IADCX19_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_p2",IADCX19_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx19_3_p3",IADCX19_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_p0",IADCX20_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_p1",IADCX20_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_p2",IADCX20_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx20_0_p3",IADCX20_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_p0",IADCX20_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_p1",IADCX20_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_p2",IADCX20_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx20_1_p3",IADCX20_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_p0",IADCX20_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_p1",IADCX20_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_p2",IADCX20_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx20_2_p3",IADCX20_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_p0",IADCX20_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_p1",IADCX20_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_p2",IADCX20_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx20_3_p3",IADCX20_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_p0",IADCX21_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_p1",IADCX21_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_p2",IADCX21_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx21_0_p3",IADCX21_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_p0",IADCX21_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_p1",IADCX21_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_p2",IADCX21_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx21_1_p3",IADCX21_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_p0",IADCX21_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_p1",IADCX21_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_p2",IADCX21_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx21_2_p3",IADCX21_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_p0",IADCX21_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_p1",IADCX21_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_p2",IADCX21_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx21_3_p3",IADCX21_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_p0",IADCX22_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_p1",IADCX22_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_p2",IADCX22_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx22_0_p3",IADCX22_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_p0",IADCX22_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_p1",IADCX22_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_p2",IADCX22_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx22_1_p3",IADCX22_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_p0",IADCX22_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_p1",IADCX22_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_p2",IADCX22_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx22_2_p3",IADCX22_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_p0",IADCX22_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_p1",IADCX22_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_p2",IADCX22_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx22_3_p3",IADCX22_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_p0",IADCX23_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_p1",IADCX23_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_p2",IADCX23_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx23_0_p3",IADCX23_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_p0",IADCX23_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_p1",IADCX23_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_p2",IADCX23_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx23_1_p3",IADCX23_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_p0",IADCX23_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_p1",IADCX23_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_p2",IADCX23_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx23_2_p3",IADCX23_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_p0",IADCX23_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_p1",IADCX23_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_p2",IADCX23_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx23_3_p3",IADCX23_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_p0",IADCX24_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_p1",IADCX24_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_p2",IADCX24_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx24_0_p3",IADCX24_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_p0",IADCX24_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_p1",IADCX24_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_p2",IADCX24_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx24_1_p3",IADCX24_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_p0",IADCX24_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_p1",IADCX24_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_p2",IADCX24_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx24_2_p3",IADCX24_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_p0",IADCX24_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_p1",IADCX24_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_p2",IADCX24_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx24_3_p3",IADCX24_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_p0",IADCX25_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_p1",IADCX25_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_p2",IADCX25_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx25_0_p3",IADCX25_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_p0",IADCX25_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_p1",IADCX25_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_p2",IADCX25_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx25_1_p3",IADCX25_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_p0",IADCX25_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_p1",IADCX25_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_p2",IADCX25_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx25_2_p3",IADCX25_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_p0",IADCX25_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_p1",IADCX25_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_p2",IADCX25_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx25_3_p3",IADCX25_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_p0",IADCX26_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_p1",IADCX26_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_p2",IADCX26_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx26_0_p3",IADCX26_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_p0",IADCX26_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_p1",IADCX26_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_p2",IADCX26_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx26_1_p3",IADCX26_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_p0",IADCX26_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_p1",IADCX26_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_p2",IADCX26_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx26_2_p3",IADCX26_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_p0",IADCX26_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_p1",IADCX26_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_p2",IADCX26_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx26_3_p3",IADCX26_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_p0",IADCX27_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_p1",IADCX27_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_p2",IADCX27_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx27_0_p3",IADCX27_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_p0",IADCX27_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_p1",IADCX27_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_p2",IADCX27_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx27_1_p3",IADCX27_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_p0",IADCX27_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_p1",IADCX27_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_p2",IADCX27_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx27_2_p3",IADCX27_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_p0",IADCX27_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_p1",IADCX27_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_p2",IADCX27_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx27_3_p3",IADCX27_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_p0",IADCX28_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_p1",IADCX28_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_p2",IADCX28_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx28_0_p3",IADCX28_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_p0",IADCX28_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_p1",IADCX28_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_p2",IADCX28_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx28_1_p3",IADCX28_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_p0",IADCX28_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_p1",IADCX28_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_p2",IADCX28_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx28_2_p3",IADCX28_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_p0",IADCX28_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_p1",IADCX28_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_p2",IADCX28_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx28_3_p3",IADCX28_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_p0",IADCX29_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_p1",IADCX29_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_p2",IADCX29_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx29_0_p3",IADCX29_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_p0",IADCX29_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_p1",IADCX29_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_p2",IADCX29_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx29_1_p3",IADCX29_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_p0",IADCX29_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_p1",IADCX29_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_p2",IADCX29_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx29_2_p3",IADCX29_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_p0",IADCX29_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_p1",IADCX29_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_p2",IADCX29_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx29_3_p3",IADCX29_3_P3));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_p0",IADCX30_0_P0));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_p1",IADCX30_0_P1));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_p2",IADCX30_0_P2));
  fKeyToIdx.insert(make_pair((string)"adcx30_0_p3",IADCX30_0_P3));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_p0",IADCX30_1_P0));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_p1",IADCX30_1_P1));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_p2",IADCX30_1_P2));
  fKeyToIdx.insert(make_pair((string)"adcx30_1_p3",IADCX30_1_P3));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_p0",IADCX30_2_P0));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_p1",IADCX30_2_P1));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_p2",IADCX30_2_P2));
  fKeyToIdx.insert(make_pair((string)"adcx30_2_p3",IADCX30_2_P3));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_p0",IADCX30_3_P0));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_p1",IADCX30_3_P1));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_p2",IADCX30_3_P2));
  fKeyToIdx.insert(make_pair((string)"adcx30_3_p3",IADCX30_3_P3));

// DAC noise
// Actually there is no DAC noise to subtract for ADCX
// so eventually we may delete this stuff.
  fKeyToIdx.insert(make_pair((string)"dacx0",IDACX0));
  fKeyToIdx.insert(make_pair((string)"dacx1",IDACX1));
  fKeyToIdx.insert(make_pair((string)"dacx2",IDACX2));
  fKeyToIdx.insert(make_pair((string)"dacx3",IDACX3));
  fKeyToIdx.insert(make_pair((string)"dacx4",IDACX4));
  fKeyToIdx.insert(make_pair((string)"dacx5",IDACX5));
  fKeyToIdx.insert(make_pair((string)"dacx6",IDACX6));
  fKeyToIdx.insert(make_pair((string)"dacx7",IDACX7));
  fKeyToIdx.insert(make_pair((string)"dacx8",IDACX8));
  fKeyToIdx.insert(make_pair((string)"dacx9",IDACX9));
  fKeyToIdx.insert(make_pair((string)"dacx10",IDACX10));
  fKeyToIdx.insert(make_pair((string)"dacx11",IDACX11));
  fKeyToIdx.insert(make_pair((string)"dacx12",IDACX12));
  fKeyToIdx.insert(make_pair((string)"dacx13",IDACX13));
  fKeyToIdx.insert(make_pair((string)"dacx14",IDACX14));
  fKeyToIdx.insert(make_pair((string)"dacx15",IDACX15));
  fKeyToIdx.insert(make_pair((string)"dacx16",IDACX16));
  fKeyToIdx.insert(make_pair((string)"dacx17",IDACX17));
  fKeyToIdx.insert(make_pair((string)"dacx18",IDACX18));
  fKeyToIdx.insert(make_pair((string)"dacx19",IDACX19));
  fKeyToIdx.insert(make_pair((string)"dacx20",IDACX20));
  fKeyToIdx.insert(make_pair((string)"dacx21",IDACX21));
  fKeyToIdx.insert(make_pair((string)"dacx22",IDACX22));
  fKeyToIdx.insert(make_pair((string)"dacx23",IDACX23));
  fKeyToIdx.insert(make_pair((string)"dacx24",IDACX24));
  fKeyToIdx.insert(make_pair((string)"dacx25",IDACX25));
  fKeyToIdx.insert(make_pair((string)"dacx26",IDACX26));
  fKeyToIdx.insert(make_pair((string)"dacx27",IDACX27));
  fKeyToIdx.insert(make_pair((string)"dacx28",IDACX28));
  fKeyToIdx.insert(make_pair((string)"dacx29",IDACX29));
  fKeyToIdx.insert(make_pair((string)"dacx30",IDACX30));

// CSR data
  fKeyToIdx.insert(make_pair((string)"csrx0",ICSRX0));
  fKeyToIdx.insert(make_pair((string)"csrx1",ICSRX1));
  fKeyToIdx.insert(make_pair((string)"csrx2",ICSRX2));
  fKeyToIdx.insert(make_pair((string)"csrx3",ICSRX3));
  fKeyToIdx.insert(make_pair((string)"csrx4",ICSRX4));
  fKeyToIdx.insert(make_pair((string)"csrx5",ICSRX5));
  fKeyToIdx.insert(make_pair((string)"csrx6",ICSRX6));
  fKeyToIdx.insert(make_pair((string)"csrx7",ICSRX7));
  fKeyToIdx.insert(make_pair((string)"csrx8",ICSRX8));
  fKeyToIdx.insert(make_pair((string)"csrx9",ICSRX9));
  fKeyToIdx.insert(make_pair((string)"csrx10",ICSRX10));
  fKeyToIdx.insert(make_pair((string)"csrx11",ICSRX11));
  fKeyToIdx.insert(make_pair((string)"csrx12",ICSRX12));
  fKeyToIdx.insert(make_pair((string)"csrx13",ICSRX13));
  fKeyToIdx.insert(make_pair((string)"csrx14",ICSRX14));
  fKeyToIdx.insert(make_pair((string)"csrx15",ICSRX15));
  fKeyToIdx.insert(make_pair((string)"csrx16",ICSRX16));
  fKeyToIdx.insert(make_pair((string)"csrx17",ICSRX17));
  fKeyToIdx.insert(make_pair((string)"csrx18",ICSRX18));
  fKeyToIdx.insert(make_pair((string)"csrx19",ICSRX19));
  fKeyToIdx.insert(make_pair((string)"csrx20",ICSRX20));
  fKeyToIdx.insert(make_pair((string)"csrx21",ICSRX21));
  fKeyToIdx.insert(make_pair((string)"csrx22",ICSRX22));
  fKeyToIdx.insert(make_pair((string)"csrx23",ICSRX23));
  fKeyToIdx.insert(make_pair((string)"csrx24",ICSRX24));
  fKeyToIdx.insert(make_pair((string)"csrx25",ICSRX25));
  fKeyToIdx.insert(make_pair((string)"csrx26",ICSRX26));
  fKeyToIdx.insert(make_pair((string)"csrx27",ICSRX27));
  fKeyToIdx.insert(make_pair((string)"csrx28",ICSRX28));
  fKeyToIdx.insert(make_pair((string)"csrx29",ICSRX29));
  fKeyToIdx.insert(make_pair((string)"csrx30",ICSRX30));

// VQWKs first index is VQWK board, second is channel
// b* = time blocks 1-4
// These are RAW data
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_nsamp",IVQWK0_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_num",  IVQWK0_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b1",   IVQWK0_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b2",   IVQWK0_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b3",   IVQWK0_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b4",   IVQWK0_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0",      IVQWK0_0));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_nsamp",IVQWK0_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_num",  IVQWK0_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b1",   IVQWK0_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b2",   IVQWK0_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b3",   IVQWK0_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b4",   IVQWK0_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1",      IVQWK0_1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_nsamp",IVQWK0_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_num",  IVQWK0_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b1",   IVQWK0_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b2",   IVQWK0_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b3",   IVQWK0_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b4",   IVQWK0_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2",      IVQWK0_2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_nsamp",IVQWK0_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_num",  IVQWK0_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b1",   IVQWK0_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b2",   IVQWK0_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b3",   IVQWK0_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b4",   IVQWK0_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3",      IVQWK0_3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_nsamp",IVQWK0_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_num",  IVQWK0_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b1",   IVQWK0_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b2",   IVQWK0_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b3",   IVQWK0_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b4",   IVQWK0_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4",      IVQWK0_4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_nsamp",IVQWK0_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_num",  IVQWK0_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b1",   IVQWK0_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b2",   IVQWK0_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b3",   IVQWK0_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b4",   IVQWK0_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5",      IVQWK0_5));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_nsamp",IVQWK0_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_num",  IVQWK0_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b1",   IVQWK0_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b2",   IVQWK0_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b3",   IVQWK0_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b4",   IVQWK0_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6",      IVQWK0_6));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_nsamp",IVQWK0_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_num",  IVQWK0_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b1",   IVQWK0_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b2",   IVQWK0_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b3",   IVQWK0_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b4",   IVQWK0_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7",      IVQWK0_7));

  fKeyToIdx.insert(make_pair((string)"vqwk1_0_nsamp",IVQWK1_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_num",  IVQWK1_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b1",   IVQWK1_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b2",   IVQWK1_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b3",   IVQWK1_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b4",   IVQWK1_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0",      IVQWK1_0));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_nsamp",IVQWK1_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_num",  IVQWK1_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b1",   IVQWK1_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b2",   IVQWK1_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b3",   IVQWK1_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b4",   IVQWK1_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1",      IVQWK1_1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_nsamp",IVQWK1_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_num",  IVQWK1_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b1",   IVQWK1_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b2",   IVQWK1_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b3",   IVQWK1_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b4",   IVQWK1_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2",      IVQWK1_2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_nsamp",IVQWK1_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_num",  IVQWK1_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b1",   IVQWK1_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b2",   IVQWK1_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b3",   IVQWK1_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b4",   IVQWK1_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3",      IVQWK1_3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_nsamp",IVQWK1_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_num",  IVQWK1_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b1",   IVQWK1_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b2",   IVQWK1_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b3",   IVQWK1_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b4",   IVQWK1_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4",      IVQWK1_4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_nsamp",IVQWK1_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_num",  IVQWK1_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b1",   IVQWK1_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b2",   IVQWK1_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b3",   IVQWK1_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b4",   IVQWK1_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5",      IVQWK1_5));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_nsamp",IVQWK1_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_num",  IVQWK1_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b1",   IVQWK1_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b2",   IVQWK1_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b3",   IVQWK1_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b4",   IVQWK1_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6",      IVQWK1_6));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_nsamp",IVQWK1_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_num",  IVQWK1_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b1",   IVQWK1_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b2",   IVQWK1_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b3",   IVQWK1_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b4",   IVQWK1_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7",      IVQWK1_7));

  fKeyToIdx.insert(make_pair((string)"vqwk2_0_nsamp",IVQWK2_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_num",  IVQWK2_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b1",   IVQWK2_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b2",   IVQWK2_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b3",   IVQWK2_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b4",   IVQWK2_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0",      IVQWK2_0));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_nsamp",IVQWK2_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_num",  IVQWK2_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b1",   IVQWK2_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b2",   IVQWK2_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b3",   IVQWK2_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b4",   IVQWK2_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1",      IVQWK2_1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_nsamp",IVQWK2_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_num",  IVQWK2_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b1",   IVQWK2_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b2",   IVQWK2_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b3",   IVQWK2_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b4",   IVQWK2_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2",      IVQWK2_2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_nsamp",IVQWK2_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_num",  IVQWK2_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b1",   IVQWK2_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b2",   IVQWK2_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b3",   IVQWK2_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b4",   IVQWK2_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3",      IVQWK2_3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_nsamp",IVQWK2_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_num",  IVQWK2_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b1",   IVQWK2_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b2",   IVQWK2_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b3",   IVQWK2_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b4",   IVQWK2_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4",      IVQWK2_4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_nsamp",IVQWK2_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_num",  IVQWK2_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b1",   IVQWK2_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b2",   IVQWK2_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b3",   IVQWK2_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b4",   IVQWK2_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5",      IVQWK2_5));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_nsamp",IVQWK2_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_num",  IVQWK2_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b1",   IVQWK2_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b2",   IVQWK2_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b3",   IVQWK2_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b4",   IVQWK2_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6",      IVQWK2_6));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_nsamp",IVQWK2_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_num",  IVQWK2_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b1",   IVQWK2_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b2",   IVQWK2_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b3",   IVQWK2_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b4",   IVQWK2_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7",      IVQWK2_7));

  fKeyToIdx.insert(make_pair((string)"vqwk3_0_nsamp",IVQWK3_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_num",  IVQWK3_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b1",   IVQWK3_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b2",   IVQWK3_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b3",   IVQWK3_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b4",   IVQWK3_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0",      IVQWK3_0));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_nsamp",IVQWK3_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_num",  IVQWK3_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b1",   IVQWK3_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b2",   IVQWK3_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b3",   IVQWK3_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b4",   IVQWK3_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1",      IVQWK3_1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_nsamp",IVQWK3_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_num",  IVQWK3_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b1",   IVQWK3_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b2",   IVQWK3_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b3",   IVQWK3_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b4",   IVQWK3_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2",      IVQWK3_2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_nsamp",IVQWK3_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_num",  IVQWK3_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b1",   IVQWK3_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b2",   IVQWK3_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b3",   IVQWK3_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b4",   IVQWK3_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3",      IVQWK3_3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_nsamp",IVQWK3_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_num",  IVQWK3_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b1",   IVQWK3_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b2",   IVQWK3_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b3",   IVQWK3_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b4",   IVQWK3_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4",      IVQWK3_4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_nsamp",IVQWK3_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_num",  IVQWK3_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b1",   IVQWK3_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b2",   IVQWK3_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b3",   IVQWK3_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b4",   IVQWK3_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5",      IVQWK3_5));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_nsamp",IVQWK3_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_num",  IVQWK3_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b1",   IVQWK3_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b2",   IVQWK3_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b3",   IVQWK3_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b4",   IVQWK3_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6",      IVQWK3_6));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_nsamp",IVQWK3_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_num",  IVQWK3_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b1",   IVQWK3_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b2",   IVQWK3_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b3",   IVQWK3_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b4",   IVQWK3_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7",      IVQWK3_7));

  fKeyToIdx.insert(make_pair((string)"vqwk4_0_nsamp",IVQWK4_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_num",  IVQWK4_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b1",   IVQWK4_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b2",   IVQWK4_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b3",   IVQWK4_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b4",   IVQWK4_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0",      IVQWK4_0));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_nsamp",IVQWK4_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_num",  IVQWK4_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b1",   IVQWK4_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b2",   IVQWK4_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b3",   IVQWK4_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b4",   IVQWK4_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1",      IVQWK4_1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_nsamp",IVQWK4_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_num",  IVQWK4_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b1",   IVQWK4_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b2",   IVQWK4_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b3",   IVQWK4_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b4",   IVQWK4_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2",      IVQWK4_2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_nsamp",IVQWK4_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_num",  IVQWK4_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b1",   IVQWK4_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b2",   IVQWK4_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b3",   IVQWK4_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b4",   IVQWK4_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3",      IVQWK4_3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_nsamp",IVQWK4_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_num",  IVQWK4_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b1",   IVQWK4_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b2",   IVQWK4_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b3",   IVQWK4_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b4",   IVQWK4_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4",      IVQWK4_4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_nsamp",IVQWK4_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_num",  IVQWK4_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b1",   IVQWK4_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b2",   IVQWK4_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b3",   IVQWK4_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b4",   IVQWK4_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5",      IVQWK4_5));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_nsamp",IVQWK4_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_num",  IVQWK4_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b1",   IVQWK4_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b2",   IVQWK4_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b3",   IVQWK4_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b4",   IVQWK4_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6",      IVQWK4_6));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_nsamp",IVQWK4_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_num",  IVQWK4_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b1",   IVQWK4_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b2",   IVQWK4_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b3",   IVQWK4_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b4",   IVQWK4_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7",      IVQWK4_7));

  fKeyToIdx.insert(make_pair((string)"vqwk5_0_nsamp",IVQWK5_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_num",  IVQWK5_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b1",   IVQWK5_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b2",   IVQWK5_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b3",   IVQWK5_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b4",   IVQWK5_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0",      IVQWK5_0));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_nsamp",IVQWK5_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_num",  IVQWK5_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b1",   IVQWK5_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b2",   IVQWK5_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b3",   IVQWK5_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b4",   IVQWK5_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1",      IVQWK5_1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_nsamp",IVQWK5_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_num",  IVQWK5_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b1",   IVQWK5_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b2",   IVQWK5_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b3",   IVQWK5_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b4",   IVQWK5_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2",      IVQWK5_2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_nsamp",IVQWK5_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_num",  IVQWK5_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b1",   IVQWK5_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b2",   IVQWK5_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b3",   IVQWK5_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b4",   IVQWK5_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3",      IVQWK5_3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_nsamp",IVQWK5_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_num",  IVQWK5_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b1",   IVQWK5_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b2",   IVQWK5_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b3",   IVQWK5_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b4",   IVQWK5_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4",      IVQWK5_4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_nsamp",IVQWK5_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_num",  IVQWK5_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b1",   IVQWK5_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b2",   IVQWK5_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b3",   IVQWK5_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b4",   IVQWK5_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5",      IVQWK5_5));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_nsamp",IVQWK5_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_num",  IVQWK5_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b1",   IVQWK5_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b2",   IVQWK5_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b3",   IVQWK5_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b4",   IVQWK5_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6",      IVQWK5_6));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_nsamp",IVQWK5_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_num",  IVQWK5_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b1",   IVQWK5_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b2",   IVQWK5_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b3",   IVQWK5_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b4",   IVQWK5_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7",      IVQWK5_7));

  fKeyToIdx.insert(make_pair((string)"vqwk6_0_nsamp",IVQWK6_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_num",  IVQWK6_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b1",   IVQWK6_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b2",   IVQWK6_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b3",   IVQWK6_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b4",   IVQWK6_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0",      IVQWK6_0));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_nsamp",IVQWK6_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_num",  IVQWK6_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b1",   IVQWK6_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b2",   IVQWK6_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b3",   IVQWK6_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b4",   IVQWK6_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1",      IVQWK6_1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_nsamp",IVQWK6_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_num",  IVQWK6_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b1",   IVQWK6_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b2",   IVQWK6_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b3",   IVQWK6_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b4",   IVQWK6_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2",      IVQWK6_2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_nsamp",IVQWK6_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_num",  IVQWK6_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b1",   IVQWK6_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b2",   IVQWK6_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b3",   IVQWK6_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b4",   IVQWK6_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3",      IVQWK6_3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_nsamp",IVQWK6_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_num",  IVQWK6_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b1",   IVQWK6_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b2",   IVQWK6_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b3",   IVQWK6_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b4",   IVQWK6_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4",      IVQWK6_4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_nsamp",IVQWK6_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_num",  IVQWK6_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b1",   IVQWK6_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b2",   IVQWK6_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b3",   IVQWK6_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b4",   IVQWK6_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5",      IVQWK6_5));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_nsamp",IVQWK6_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_num",  IVQWK6_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b1",   IVQWK6_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b2",   IVQWK6_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b3",   IVQWK6_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b4",   IVQWK6_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6",      IVQWK6_6));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_nsamp",IVQWK6_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_num",  IVQWK6_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b1",   IVQWK6_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b2",   IVQWK6_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b3",   IVQWK6_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b4",   IVQWK6_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7",      IVQWK6_7));

  fKeyToIdx.insert(make_pair((string)"vqwk7_0_nsamp",IVQWK7_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_num",  IVQWK7_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b1",   IVQWK7_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b2",   IVQWK7_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b3",   IVQWK7_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b4",   IVQWK7_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0",      IVQWK7_0));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_nsamp",IVQWK7_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_num",  IVQWK7_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b1",   IVQWK7_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b2",   IVQWK7_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b3",   IVQWK7_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b4",   IVQWK7_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1",      IVQWK7_1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_nsamp",IVQWK7_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_num",  IVQWK7_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b1",   IVQWK7_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b2",   IVQWK7_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b3",   IVQWK7_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b4",   IVQWK7_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2",      IVQWK7_2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_nsamp",IVQWK7_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_num",  IVQWK7_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b1",   IVQWK7_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b2",   IVQWK7_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b3",   IVQWK7_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b4",   IVQWK7_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3",      IVQWK7_3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_nsamp",IVQWK7_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_num",  IVQWK7_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b1",   IVQWK7_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b2",   IVQWK7_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b3",   IVQWK7_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b4",   IVQWK7_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4",      IVQWK7_4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_nsamp",IVQWK7_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_num",  IVQWK7_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b1",   IVQWK7_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b2",   IVQWK7_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b3",   IVQWK7_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b4",   IVQWK7_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5",      IVQWK7_5));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_nsamp",IVQWK7_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_num",  IVQWK7_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b1",   IVQWK7_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b2",   IVQWK7_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b3",   IVQWK7_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b4",   IVQWK7_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6",      IVQWK7_6));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_nsamp",IVQWK7_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_num",  IVQWK7_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b1",   IVQWK7_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b2",   IVQWK7_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b3",   IVQWK7_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b4",   IVQWK7_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7",      IVQWK7_7));

  fKeyToIdx.insert(make_pair((string)"vqwk8_0_nsamp",IVQWK8_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_num",  IVQWK8_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b1",   IVQWK8_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b2",   IVQWK8_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b3",   IVQWK8_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b4",   IVQWK8_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0",      IVQWK8_0));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_nsamp",IVQWK8_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_num",  IVQWK8_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b1",   IVQWK8_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b2",   IVQWK8_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b3",   IVQWK8_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b4",   IVQWK8_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1",      IVQWK8_1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_nsamp",IVQWK8_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_num",  IVQWK8_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b1",   IVQWK8_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b2",   IVQWK8_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b3",   IVQWK8_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b4",   IVQWK8_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2",      IVQWK8_2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_nsamp",IVQWK8_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_num",  IVQWK8_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b1",   IVQWK8_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b2",   IVQWK8_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b3",   IVQWK8_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b4",   IVQWK8_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3",      IVQWK8_3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_nsamp",IVQWK8_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_num",  IVQWK8_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b1",   IVQWK8_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b2",   IVQWK8_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b3",   IVQWK8_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b4",   IVQWK8_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4",      IVQWK8_4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_nsamp",IVQWK8_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_num",  IVQWK8_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b1",   IVQWK8_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b2",   IVQWK8_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b3",   IVQWK8_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b4",   IVQWK8_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5",      IVQWK8_5));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_nsamp",IVQWK8_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_num",  IVQWK8_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b1",   IVQWK8_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b2",   IVQWK8_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b3",   IVQWK8_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b4",   IVQWK8_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6",      IVQWK8_6));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_nsamp",IVQWK8_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_num",  IVQWK8_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b1",   IVQWK8_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b2",   IVQWK8_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b3",   IVQWK8_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b4",   IVQWK8_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7",      IVQWK8_7));

  fKeyToIdx.insert(make_pair((string)"vqwk9_0_nsamp",IVQWK9_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_num",  IVQWK9_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b1",   IVQWK9_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b2",   IVQWK9_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b3",   IVQWK9_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b4",   IVQWK9_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0",      IVQWK9_0));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_nsamp",IVQWK9_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_num",  IVQWK9_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b1",   IVQWK9_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b2",   IVQWK9_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b3",   IVQWK9_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b4",   IVQWK9_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1",      IVQWK9_1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_nsamp",IVQWK9_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_num",  IVQWK9_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b1",   IVQWK9_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b2",   IVQWK9_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b3",   IVQWK9_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b4",   IVQWK9_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2",      IVQWK9_2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_nsamp",IVQWK9_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_num",  IVQWK9_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b1",   IVQWK9_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b2",   IVQWK9_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b3",   IVQWK9_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b4",   IVQWK9_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3",      IVQWK9_3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_nsamp",IVQWK9_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_num",  IVQWK9_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b1",   IVQWK9_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b2",   IVQWK9_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b3",   IVQWK9_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b4",   IVQWK9_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4",      IVQWK9_4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_nsamp",IVQWK9_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_num",  IVQWK9_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b1",   IVQWK9_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b2",   IVQWK9_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b3",   IVQWK9_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b4",   IVQWK9_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5",      IVQWK9_5));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_nsamp",IVQWK9_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_num",  IVQWK9_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b1",   IVQWK9_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b2",   IVQWK9_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b3",   IVQWK9_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b4",   IVQWK9_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6",      IVQWK9_6));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_nsamp",IVQWK9_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_num",  IVQWK9_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b1",   IVQWK9_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b2",   IVQWK9_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b3",   IVQWK9_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b4",   IVQWK9_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7",      IVQWK9_7));

  fKeyToIdx.insert(make_pair((string)"vqwk10_0_nsamp",IVQWK10_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_num",  IVQWK10_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b1",   IVQWK10_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b2",   IVQWK10_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b3",   IVQWK10_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b4",   IVQWK10_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0",      IVQWK10_0));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_nsamp",IVQWK10_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_num",  IVQWK10_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b1",   IVQWK10_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b2",   IVQWK10_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b3",   IVQWK10_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b4",   IVQWK10_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1",      IVQWK10_1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_nsamp",IVQWK10_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_num",  IVQWK10_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b1",   IVQWK10_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b2",   IVQWK10_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b3",   IVQWK10_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b4",   IVQWK10_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2",      IVQWK10_2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_nsamp",IVQWK10_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_num",  IVQWK10_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b1",   IVQWK10_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b2",   IVQWK10_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b3",   IVQWK10_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b4",   IVQWK10_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3",      IVQWK10_3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_nsamp",IVQWK10_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_num",  IVQWK10_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b1",   IVQWK10_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b2",   IVQWK10_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b3",   IVQWK10_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b4",   IVQWK10_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4",      IVQWK10_4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_nsamp",IVQWK10_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_num",  IVQWK10_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b1",   IVQWK10_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b2",   IVQWK10_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b3",   IVQWK10_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b4",   IVQWK10_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5",      IVQWK10_5));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_nsamp",IVQWK10_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_num",  IVQWK10_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b1",   IVQWK10_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b2",   IVQWK10_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b3",   IVQWK10_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b4",   IVQWK10_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6",      IVQWK10_6));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_nsamp",IVQWK10_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_num",  IVQWK10_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b1",   IVQWK10_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b2",   IVQWK10_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b3",   IVQWK10_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b4",   IVQWK10_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7",      IVQWK10_7));

  fKeyToIdx.insert(make_pair((string)"vqwk11_0_nsamp",IVQWK11_0_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_num",  IVQWK11_0_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b1",   IVQWK11_0_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b2",   IVQWK11_0_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b3",   IVQWK11_0_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b4",   IVQWK11_0_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0",      IVQWK11_0));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_nsamp",IVQWK11_1_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_num",  IVQWK11_1_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b1",   IVQWK11_1_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b2",   IVQWK11_1_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b3",   IVQWK11_1_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b4",   IVQWK11_1_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1",      IVQWK11_1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_nsamp",IVQWK11_2_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_num",  IVQWK11_2_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b1",   IVQWK11_2_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b2",   IVQWK11_2_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b3",   IVQWK11_2_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b4",   IVQWK11_2_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2",      IVQWK11_2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_nsamp",IVQWK11_3_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_num",  IVQWK11_3_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b1",   IVQWK11_3_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b2",   IVQWK11_3_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b3",   IVQWK11_3_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b4",   IVQWK11_3_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3",      IVQWK11_3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_nsamp",IVQWK11_4_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_num",  IVQWK11_4_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b1",   IVQWK11_4_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b2",   IVQWK11_4_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b3",   IVQWK11_4_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b4",   IVQWK11_4_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4",      IVQWK11_4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_nsamp",IVQWK11_5_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_num",  IVQWK11_5_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b1",   IVQWK11_5_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b2",   IVQWK11_5_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b3",   IVQWK11_5_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b4",   IVQWK11_5_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5",      IVQWK11_5));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_nsamp",IVQWK11_6_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_num",  IVQWK11_6_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b1",   IVQWK11_6_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b2",   IVQWK11_6_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b3",   IVQWK11_6_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b4",   IVQWK11_6_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6",      IVQWK11_6));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_nsamp",IVQWK11_7_NSAMP));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_num",  IVQWK11_7_NUM));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b1",   IVQWK11_7_B1));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b2",   IVQWK11_7_B2));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b3",   IVQWK11_7_B3));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b4",   IVQWK11_7_B4));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7",      IVQWK11_7));

// VQWKs first index is VQWK board, second is channel
// b* = time blocks 1-4
// These are CALIBRATED data
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b1_cal",   IVQWK0_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b2_cal",   IVQWK0_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b3_cal",   IVQWK0_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_b4_cal",   IVQWK0_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_0_cal",      IVQWK0_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b1_cal",   IVQWK0_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b2_cal",   IVQWK0_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b3_cal",   IVQWK0_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_b4_cal",   IVQWK0_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_1_cal",      IVQWK0_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b1_cal",   IVQWK0_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b2_cal",   IVQWK0_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b3_cal",   IVQWK0_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_b4_cal",   IVQWK0_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_2_cal",      IVQWK0_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b1_cal",   IVQWK0_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b2_cal",   IVQWK0_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b3_cal",   IVQWK0_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_b4_cal",   IVQWK0_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_3_cal",      IVQWK0_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b1_cal",   IVQWK0_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b2_cal",   IVQWK0_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b3_cal",   IVQWK0_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_b4_cal",   IVQWK0_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_4_cal",      IVQWK0_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b1_cal",   IVQWK0_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b2_cal",   IVQWK0_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b3_cal",   IVQWK0_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_b4_cal",   IVQWK0_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_5_cal",      IVQWK0_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b1_cal",   IVQWK0_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b2_cal",   IVQWK0_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b3_cal",   IVQWK0_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_b4_cal",   IVQWK0_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_6_cal",      IVQWK0_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b1_cal",   IVQWK0_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b2_cal",   IVQWK0_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b3_cal",   IVQWK0_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_b4_cal",   IVQWK0_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk0_7_cal",      IVQWK0_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b1_cal",   IVQWK1_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b2_cal",   IVQWK1_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b3_cal",   IVQWK1_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_b4_cal",   IVQWK1_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_0_cal",      IVQWK1_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b1_cal",   IVQWK1_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b2_cal",   IVQWK1_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b3_cal",   IVQWK1_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_b4_cal",   IVQWK1_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_1_cal",      IVQWK1_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b1_cal",   IVQWK1_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b2_cal",   IVQWK1_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b3_cal",   IVQWK1_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_b4_cal",   IVQWK1_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_2_cal",      IVQWK1_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b1_cal",   IVQWK1_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b2_cal",   IVQWK1_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b3_cal",   IVQWK1_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_b4_cal",   IVQWK1_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_3_cal",      IVQWK1_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b1_cal",   IVQWK1_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b2_cal",   IVQWK1_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b3_cal",   IVQWK1_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_b4_cal",   IVQWK1_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_4_cal",      IVQWK1_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b1_cal",   IVQWK1_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b2_cal",   IVQWK1_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b3_cal",   IVQWK1_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_b4_cal",   IVQWK1_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_5_cal",      IVQWK1_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b1_cal",   IVQWK1_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b2_cal",   IVQWK1_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b3_cal",   IVQWK1_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_b4_cal",   IVQWK1_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_6_cal",      IVQWK1_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b1_cal",   IVQWK1_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b2_cal",   IVQWK1_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b3_cal",   IVQWK1_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_b4_cal",   IVQWK1_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk1_7_cal",      IVQWK1_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b1_cal",   IVQWK2_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b2_cal",   IVQWK2_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b3_cal",   IVQWK2_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_b4_cal",   IVQWK2_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_0_cal",      IVQWK2_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b1_cal",   IVQWK2_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b2_cal",   IVQWK2_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b3_cal",   IVQWK2_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_b4_cal",   IVQWK2_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_1_cal",      IVQWK2_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b1_cal",   IVQWK2_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b2_cal",   IVQWK2_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b3_cal",   IVQWK2_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_b4_cal",   IVQWK2_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_2_cal",      IVQWK2_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b1_cal",   IVQWK2_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b2_cal",   IVQWK2_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b3_cal",   IVQWK2_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_b4_cal",   IVQWK2_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_3_cal",      IVQWK2_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b1_cal",   IVQWK2_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b2_cal",   IVQWK2_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b3_cal",   IVQWK2_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_b4_cal",   IVQWK2_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_4_cal",      IVQWK2_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b1_cal",   IVQWK2_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b2_cal",   IVQWK2_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b3_cal",   IVQWK2_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_b4_cal",   IVQWK2_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_5_cal",      IVQWK2_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b1_cal",   IVQWK2_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b2_cal",   IVQWK2_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b3_cal",   IVQWK2_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_b4_cal",   IVQWK2_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_6_cal",      IVQWK2_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b1_cal",   IVQWK2_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b2_cal",   IVQWK2_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b3_cal",   IVQWK2_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_b4_cal",   IVQWK2_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk2_7_cal",      IVQWK2_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b1_cal",   IVQWK3_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b2_cal",   IVQWK3_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b3_cal",   IVQWK3_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_b4_cal",   IVQWK3_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_0_cal",      IVQWK3_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b1_cal",   IVQWK3_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b2_cal",   IVQWK3_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b3_cal",   IVQWK3_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_b4_cal",   IVQWK3_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_1_cal",      IVQWK3_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b1_cal",   IVQWK3_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b2_cal",   IVQWK3_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b3_cal",   IVQWK3_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_b4_cal",   IVQWK3_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_2_cal",      IVQWK3_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b1_cal",   IVQWK3_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b2_cal",   IVQWK3_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b3_cal",   IVQWK3_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_b4_cal",   IVQWK3_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_3_cal",      IVQWK3_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b1_cal",   IVQWK3_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b2_cal",   IVQWK3_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b3_cal",   IVQWK3_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_b4_cal",   IVQWK3_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_4_cal",      IVQWK3_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b1_cal",   IVQWK3_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b2_cal",   IVQWK3_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b3_cal",   IVQWK3_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_b4_cal",   IVQWK3_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_5_cal",      IVQWK3_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b1_cal",   IVQWK3_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b2_cal",   IVQWK3_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b3_cal",   IVQWK3_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_b4_cal",   IVQWK3_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_6_cal",      IVQWK3_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b1_cal",   IVQWK3_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b2_cal",   IVQWK3_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b3_cal",   IVQWK3_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_b4_cal",   IVQWK3_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk3_7_cal",      IVQWK3_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b1_cal",   IVQWK4_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b2_cal",   IVQWK4_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b3_cal",   IVQWK4_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_b4_cal",   IVQWK4_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_0_cal",      IVQWK4_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b1_cal",   IVQWK4_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b2_cal",   IVQWK4_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b3_cal",   IVQWK4_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_b4_cal",   IVQWK4_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_1_cal",      IVQWK4_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b1_cal",   IVQWK4_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b2_cal",   IVQWK4_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b3_cal",   IVQWK4_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_b4_cal",   IVQWK4_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_2_cal",      IVQWK4_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b1_cal",   IVQWK4_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b2_cal",   IVQWK4_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b3_cal",   IVQWK4_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_b4_cal",   IVQWK4_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_3_cal",      IVQWK4_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b1_cal",   IVQWK4_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b2_cal",   IVQWK4_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b3_cal",   IVQWK4_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_b4_cal",   IVQWK4_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_4_cal",      IVQWK4_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b1_cal",   IVQWK4_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b2_cal",   IVQWK4_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b3_cal",   IVQWK4_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_b4_cal",   IVQWK4_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_5_cal",      IVQWK4_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b1_cal",   IVQWK4_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b2_cal",   IVQWK4_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b3_cal",   IVQWK4_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_b4_cal",   IVQWK4_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_6_cal",      IVQWK4_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b1_cal",   IVQWK4_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b2_cal",   IVQWK4_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b3_cal",   IVQWK4_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_b4_cal",   IVQWK4_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk4_7_cal",      IVQWK4_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b1_cal",   IVQWK5_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b2_cal",   IVQWK5_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b3_cal",   IVQWK5_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_b4_cal",   IVQWK5_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_0_cal",      IVQWK5_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b1_cal",   IVQWK5_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b2_cal",   IVQWK5_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b3_cal",   IVQWK5_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_b4_cal",   IVQWK5_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_1_cal",      IVQWK5_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b1_cal",   IVQWK5_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b2_cal",   IVQWK5_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b3_cal",   IVQWK5_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_b4_cal",   IVQWK5_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_2_cal",      IVQWK5_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b1_cal",   IVQWK5_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b2_cal",   IVQWK5_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b3_cal",   IVQWK5_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_b4_cal",   IVQWK5_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_3_cal",      IVQWK5_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b1_cal",   IVQWK5_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b2_cal",   IVQWK5_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b3_cal",   IVQWK5_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_b4_cal",   IVQWK5_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_4_cal",      IVQWK5_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b1_cal",   IVQWK5_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b2_cal",   IVQWK5_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b3_cal",   IVQWK5_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_b4_cal",   IVQWK5_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_5_cal",      IVQWK5_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b1_cal",   IVQWK5_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b2_cal",   IVQWK5_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b3_cal",   IVQWK5_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_b4_cal",   IVQWK5_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_6_cal",      IVQWK5_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b1_cal",   IVQWK5_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b2_cal",   IVQWK5_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b3_cal",   IVQWK5_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_b4_cal",   IVQWK5_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk5_7_cal",      IVQWK5_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b1_cal",   IVQWK6_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b2_cal",   IVQWK6_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b3_cal",   IVQWK6_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_b4_cal",   IVQWK6_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_0_cal",      IVQWK6_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b1_cal",   IVQWK6_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b2_cal",   IVQWK6_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b3_cal",   IVQWK6_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_b4_cal",   IVQWK6_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_1_cal",      IVQWK6_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b1_cal",   IVQWK6_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b2_cal",   IVQWK6_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b3_cal",   IVQWK6_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_b4_cal",   IVQWK6_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_2_cal",      IVQWK6_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b1_cal",   IVQWK6_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b2_cal",   IVQWK6_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b3_cal",   IVQWK6_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_b4_cal",   IVQWK6_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_3_cal",      IVQWK6_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b1_cal",   IVQWK6_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b2_cal",   IVQWK6_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b3_cal",   IVQWK6_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_b4_cal",   IVQWK6_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_4_cal",      IVQWK6_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b1_cal",   IVQWK6_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b2_cal",   IVQWK6_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b3_cal",   IVQWK6_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_b4_cal",   IVQWK6_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_5_cal",      IVQWK6_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b1_cal",   IVQWK6_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b2_cal",   IVQWK6_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b3_cal",   IVQWK6_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_b4_cal",   IVQWK6_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_6_cal",      IVQWK6_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b1_cal",   IVQWK6_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b2_cal",   IVQWK6_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b3_cal",   IVQWK6_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_b4_cal",   IVQWK6_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk6_7_cal",      IVQWK6_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b1_cal",   IVQWK7_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b2_cal",   IVQWK7_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b3_cal",   IVQWK7_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_b4_cal",   IVQWK7_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_0_cal",      IVQWK7_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b1_cal",   IVQWK7_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b2_cal",   IVQWK7_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b3_cal",   IVQWK7_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_b4_cal",   IVQWK7_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_1_cal",      IVQWK7_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b1_cal",   IVQWK7_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b2_cal",   IVQWK7_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b3_cal",   IVQWK7_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_b4_cal",   IVQWK7_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_2_cal",      IVQWK7_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b1_cal",   IVQWK7_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b2_cal",   IVQWK7_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b3_cal",   IVQWK7_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_b4_cal",   IVQWK7_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_3_cal",      IVQWK7_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b1_cal",   IVQWK7_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b2_cal",   IVQWK7_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b3_cal",   IVQWK7_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_b4_cal",   IVQWK7_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_4_cal",      IVQWK7_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b1_cal",   IVQWK7_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b2_cal",   IVQWK7_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b3_cal",   IVQWK7_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_b4_cal",   IVQWK7_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_5_cal",      IVQWK7_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b1_cal",   IVQWK7_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b2_cal",   IVQWK7_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b3_cal",   IVQWK7_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_b4_cal",   IVQWK7_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_6_cal",      IVQWK7_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b1_cal",   IVQWK7_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b2_cal",   IVQWK7_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b3_cal",   IVQWK7_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_b4_cal",   IVQWK7_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk7_7_cal",      IVQWK7_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b1_cal",   IVQWK8_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b2_cal",   IVQWK8_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b3_cal",   IVQWK8_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_b4_cal",   IVQWK8_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_0_cal",      IVQWK8_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b1_cal",   IVQWK8_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b2_cal",   IVQWK8_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b3_cal",   IVQWK8_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_b4_cal",   IVQWK8_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_1_cal",      IVQWK8_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b1_cal",   IVQWK8_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b2_cal",   IVQWK8_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b3_cal",   IVQWK8_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_b4_cal",   IVQWK8_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_2_cal",      IVQWK8_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b1_cal",   IVQWK8_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b2_cal",   IVQWK8_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b3_cal",   IVQWK8_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_b4_cal",   IVQWK8_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_3_cal",      IVQWK8_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b1_cal",   IVQWK8_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b2_cal",   IVQWK8_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b3_cal",   IVQWK8_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_b4_cal",   IVQWK8_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_4_cal",      IVQWK8_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b1_cal",   IVQWK8_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b2_cal",   IVQWK8_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b3_cal",   IVQWK8_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_b4_cal",   IVQWK8_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_5_cal",      IVQWK8_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b1_cal",   IVQWK8_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b2_cal",   IVQWK8_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b3_cal",   IVQWK8_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_b4_cal",   IVQWK8_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_6_cal",      IVQWK8_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b1_cal",   IVQWK8_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b2_cal",   IVQWK8_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b3_cal",   IVQWK8_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_b4_cal",   IVQWK8_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk8_7_cal",      IVQWK8_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b1_cal",   IVQWK9_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b2_cal",   IVQWK9_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b3_cal",   IVQWK9_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_b4_cal",   IVQWK9_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_0_cal",      IVQWK9_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b1_cal",   IVQWK9_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b2_cal",   IVQWK9_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b3_cal",   IVQWK9_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_b4_cal",   IVQWK9_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_1_cal",      IVQWK9_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b1_cal",   IVQWK9_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b2_cal",   IVQWK9_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b3_cal",   IVQWK9_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_b4_cal",   IVQWK9_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_2_cal",      IVQWK9_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b1_cal",   IVQWK9_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b2_cal",   IVQWK9_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b3_cal",   IVQWK9_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_b4_cal",   IVQWK9_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_3_cal",      IVQWK9_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b1_cal",   IVQWK9_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b2_cal",   IVQWK9_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b3_cal",   IVQWK9_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_b4_cal",   IVQWK9_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_4_cal",      IVQWK9_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b1_cal",   IVQWK9_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b2_cal",   IVQWK9_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b3_cal",   IVQWK9_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_b4_cal",   IVQWK9_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_5_cal",      IVQWK9_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b1_cal",   IVQWK9_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b2_cal",   IVQWK9_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b3_cal",   IVQWK9_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_b4_cal",   IVQWK9_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_6_cal",      IVQWK9_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b1_cal",   IVQWK9_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b2_cal",   IVQWK9_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b3_cal",   IVQWK9_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_b4_cal",   IVQWK9_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk9_7_cal",      IVQWK9_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b1_cal",   IVQWK10_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b2_cal",   IVQWK10_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b3_cal",   IVQWK10_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_b4_cal",   IVQWK10_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_0_cal",      IVQWK10_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b1_cal",   IVQWK10_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b2_cal",   IVQWK10_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b3_cal",   IVQWK10_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_b4_cal",   IVQWK10_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_1_cal",      IVQWK10_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b1_cal",   IVQWK10_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b2_cal",   IVQWK10_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b3_cal",   IVQWK10_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_b4_cal",   IVQWK10_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_2_cal",      IVQWK10_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b1_cal",   IVQWK10_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b2_cal",   IVQWK10_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b3_cal",   IVQWK10_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_b4_cal",   IVQWK10_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_3_cal",      IVQWK10_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b1_cal",   IVQWK10_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b2_cal",   IVQWK10_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b3_cal",   IVQWK10_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_b4_cal",   IVQWK10_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_4_cal",      IVQWK10_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b1_cal",   IVQWK10_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b2_cal",   IVQWK10_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b3_cal",   IVQWK10_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_b4_cal",   IVQWK10_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_5_cal",      IVQWK10_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b1_cal",   IVQWK10_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b2_cal",   IVQWK10_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b3_cal",   IVQWK10_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_b4_cal",   IVQWK10_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_6_cal",      IVQWK10_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b1_cal",   IVQWK10_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b2_cal",   IVQWK10_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b3_cal",   IVQWK10_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_b4_cal",   IVQWK10_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk10_7_cal",      IVQWK10_7_CAL));

  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b1_cal",   IVQWK11_0_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b2_cal",   IVQWK11_0_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b3_cal",   IVQWK11_0_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_b4_cal",   IVQWK11_0_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_0_cal",      IVQWK11_0_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b1_cal",   IVQWK11_1_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b2_cal",   IVQWK11_1_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b3_cal",   IVQWK11_1_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_b4_cal",   IVQWK11_1_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_1_cal",      IVQWK11_1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b1_cal",   IVQWK11_2_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b2_cal",   IVQWK11_2_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b3_cal",   IVQWK11_2_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_b4_cal",   IVQWK11_2_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_2_cal",      IVQWK11_2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b1_cal",   IVQWK11_3_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b2_cal",   IVQWK11_3_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b3_cal",   IVQWK11_3_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_b4_cal",   IVQWK11_3_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_3_cal",      IVQWK11_3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b1_cal",   IVQWK11_4_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b2_cal",   IVQWK11_4_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b3_cal",   IVQWK11_4_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_b4_cal",   IVQWK11_4_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_4_cal",      IVQWK11_4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b1_cal",   IVQWK11_5_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b2_cal",   IVQWK11_5_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b3_cal",   IVQWK11_5_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_b4_cal",   IVQWK11_5_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_5_cal",      IVQWK11_5_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b1_cal",   IVQWK11_6_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b2_cal",   IVQWK11_6_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b3_cal",   IVQWK11_6_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_b4_cal",   IVQWK11_6_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_6_cal",      IVQWK11_6_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b1_cal",   IVQWK11_7_B1_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b2_cal",   IVQWK11_7_B2_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b3_cal",   IVQWK11_7_B3_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_b4_cal",   IVQWK11_7_B4_CAL));
  fKeyToIdx.insert(make_pair((string)"vqwk11_7_cal",      IVQWK11_7_CAL));

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

  fKeyToIdx.insert(make_pair((string)"scaler7_0",ISCALER7_0));
  fKeyToIdx.insert(make_pair((string)"scaler7_1",ISCALER7_1));
  fKeyToIdx.insert(make_pair((string)"scaler7_2",ISCALER7_2));
  fKeyToIdx.insert(make_pair((string)"scaler7_3",ISCALER7_3));
  fKeyToIdx.insert(make_pair((string)"scaler7_4",ISCALER7_4));
  fKeyToIdx.insert(make_pair((string)"scaler7_5",ISCALER7_5));
  fKeyToIdx.insert(make_pair((string)"scaler7_6",ISCALER7_6));
  fKeyToIdx.insert(make_pair((string)"scaler7_7",ISCALER7_7));
  fKeyToIdx.insert(make_pair((string)"scaler7_8",ISCALER7_8));
  fKeyToIdx.insert(make_pair((string)"scaler7_9",ISCALER7_9));
  fKeyToIdx.insert(make_pair((string)"scaler7_10",ISCALER7_10));
  fKeyToIdx.insert(make_pair((string)"scaler7_11",ISCALER7_11));
  fKeyToIdx.insert(make_pair((string)"scaler7_12",ISCALER7_12));
  fKeyToIdx.insert(make_pair((string)"scaler7_13",ISCALER7_13));
  fKeyToIdx.insert(make_pair((string)"scaler7_14",ISCALER7_14));
  fKeyToIdx.insert(make_pair((string)"scaler7_15",ISCALER7_15));
  fKeyToIdx.insert(make_pair((string)"scaler7_16",ISCALER7_16));
  fKeyToIdx.insert(make_pair((string)"scaler7_17",ISCALER7_17));
  fKeyToIdx.insert(make_pair((string)"scaler7_18",ISCALER7_18));
  fKeyToIdx.insert(make_pair((string)"scaler7_19",ISCALER7_19));
  fKeyToIdx.insert(make_pair((string)"scaler7_20",ISCALER7_20));
  fKeyToIdx.insert(make_pair((string)"scaler7_21",ISCALER7_21));
  fKeyToIdx.insert(make_pair((string)"scaler7_22",ISCALER7_22));
  fKeyToIdx.insert(make_pair((string)"scaler7_23",ISCALER7_23));
  fKeyToIdx.insert(make_pair((string)"scaler7_24",ISCALER7_24));
  fKeyToIdx.insert(make_pair((string)"scaler7_25",ISCALER7_25));
  fKeyToIdx.insert(make_pair((string)"scaler7_26",ISCALER7_26));
  fKeyToIdx.insert(make_pair((string)"scaler7_27",ISCALER7_27));
  fKeyToIdx.insert(make_pair((string)"scaler7_28",ISCALER7_28));
  fKeyToIdx.insert(make_pair((string)"scaler7_29",ISCALER7_29));
  fKeyToIdx.insert(make_pair((string)"scaler7_30",ISCALER7_30));
  fKeyToIdx.insert(make_pair((string)"scaler7_31",ISCALER7_31));

  fKeyToIdx.insert(make_pair((string)"scaler8_0",ISCALER8_0));
  fKeyToIdx.insert(make_pair((string)"scaler8_1",ISCALER8_1));
  fKeyToIdx.insert(make_pair((string)"scaler8_2",ISCALER8_2));
  fKeyToIdx.insert(make_pair((string)"scaler8_3",ISCALER8_3));
  fKeyToIdx.insert(make_pair((string)"scaler8_4",ISCALER8_4));
  fKeyToIdx.insert(make_pair((string)"scaler8_5",ISCALER8_5));
  fKeyToIdx.insert(make_pair((string)"scaler8_6",ISCALER8_6));
  fKeyToIdx.insert(make_pair((string)"scaler8_7",ISCALER8_7));
  fKeyToIdx.insert(make_pair((string)"scaler8_8",ISCALER8_8));
  fKeyToIdx.insert(make_pair((string)"scaler8_9",ISCALER8_9));
  fKeyToIdx.insert(make_pair((string)"scaler8_10",ISCALER8_10));
  fKeyToIdx.insert(make_pair((string)"scaler8_11",ISCALER8_11));
  fKeyToIdx.insert(make_pair((string)"scaler8_12",ISCALER8_12));
  fKeyToIdx.insert(make_pair((string)"scaler8_13",ISCALER8_13));
  fKeyToIdx.insert(make_pair((string)"scaler8_14",ISCALER8_14));
  fKeyToIdx.insert(make_pair((string)"scaler8_15",ISCALER8_15));
  fKeyToIdx.insert(make_pair((string)"scaler8_16",ISCALER8_16));
  fKeyToIdx.insert(make_pair((string)"scaler8_17",ISCALER8_17));
  fKeyToIdx.insert(make_pair((string)"scaler8_18",ISCALER8_18));
  fKeyToIdx.insert(make_pair((string)"scaler8_19",ISCALER8_19));
  fKeyToIdx.insert(make_pair((string)"scaler8_20",ISCALER8_20));
  fKeyToIdx.insert(make_pair((string)"scaler8_21",ISCALER8_21));
  fKeyToIdx.insert(make_pair((string)"scaler8_22",ISCALER8_22));
  fKeyToIdx.insert(make_pair((string)"scaler8_23",ISCALER8_23));
  fKeyToIdx.insert(make_pair((string)"scaler8_24",ISCALER8_24));
  fKeyToIdx.insert(make_pair((string)"scaler8_25",ISCALER8_25));
  fKeyToIdx.insert(make_pair((string)"scaler8_26",ISCALER8_26));
  fKeyToIdx.insert(make_pair((string)"scaler8_27",ISCALER8_27));
  fKeyToIdx.insert(make_pair((string)"scaler8_28",ISCALER8_28));
  fKeyToIdx.insert(make_pair((string)"scaler8_29",ISCALER8_29));
  fKeyToIdx.insert(make_pair((string)"scaler8_30",ISCALER8_30));
  fKeyToIdx.insert(make_pair((string)"scaler8_31",ISCALER8_31));

  fKeyToIdx.insert(make_pair((string)"scaler9_0",ISCALER9_0));
  fKeyToIdx.insert(make_pair((string)"scaler9_1",ISCALER9_1));
  fKeyToIdx.insert(make_pair((string)"scaler9_2",ISCALER9_2));
  fKeyToIdx.insert(make_pair((string)"scaler9_3",ISCALER9_3));
  fKeyToIdx.insert(make_pair((string)"scaler9_4",ISCALER9_4));
  fKeyToIdx.insert(make_pair((string)"scaler9_5",ISCALER9_5));
  fKeyToIdx.insert(make_pair((string)"scaler9_6",ISCALER9_6));
  fKeyToIdx.insert(make_pair((string)"scaler9_7",ISCALER9_7));
  fKeyToIdx.insert(make_pair((string)"scaler9_8",ISCALER9_8));
  fKeyToIdx.insert(make_pair((string)"scaler9_9",ISCALER9_9));
  fKeyToIdx.insert(make_pair((string)"scaler9_10",ISCALER9_10));
  fKeyToIdx.insert(make_pair((string)"scaler9_11",ISCALER9_11));
  fKeyToIdx.insert(make_pair((string)"scaler9_12",ISCALER9_12));
  fKeyToIdx.insert(make_pair((string)"scaler9_13",ISCALER9_13));
  fKeyToIdx.insert(make_pair((string)"scaler9_14",ISCALER9_14));
  fKeyToIdx.insert(make_pair((string)"scaler9_15",ISCALER9_15));
  fKeyToIdx.insert(make_pair((string)"scaler9_16",ISCALER9_16));
  fKeyToIdx.insert(make_pair((string)"scaler9_17",ISCALER9_17));
  fKeyToIdx.insert(make_pair((string)"scaler9_18",ISCALER9_18));
  fKeyToIdx.insert(make_pair((string)"scaler9_19",ISCALER9_19));
  fKeyToIdx.insert(make_pair((string)"scaler9_20",ISCALER9_20));
  fKeyToIdx.insert(make_pair((string)"scaler9_21",ISCALER9_21));
  fKeyToIdx.insert(make_pair((string)"scaler9_22",ISCALER9_22));
  fKeyToIdx.insert(make_pair((string)"scaler9_23",ISCALER9_23));
  fKeyToIdx.insert(make_pair((string)"scaler9_24",ISCALER9_24));
  fKeyToIdx.insert(make_pair((string)"scaler9_25",ISCALER9_25));
  fKeyToIdx.insert(make_pair((string)"scaler9_26",ISCALER9_26));
  fKeyToIdx.insert(make_pair((string)"scaler9_27",ISCALER9_27));
  fKeyToIdx.insert(make_pair((string)"scaler9_28",ISCALER9_28));
  fKeyToIdx.insert(make_pair((string)"scaler9_29",ISCALER9_29));
  fKeyToIdx.insert(make_pair((string)"scaler9_30",ISCALER9_30));
  fKeyToIdx.insert(make_pair((string)"scaler9_31",ISCALER9_31));

  fKeyToIdx.insert(make_pair((string)"scaler10_0",ISCALER10_0));
  fKeyToIdx.insert(make_pair((string)"scaler10_1",ISCALER10_1));
  fKeyToIdx.insert(make_pair((string)"scaler10_2",ISCALER10_2));
  fKeyToIdx.insert(make_pair((string)"scaler10_3",ISCALER10_3));
  fKeyToIdx.insert(make_pair((string)"scaler10_4",ISCALER10_4));
  fKeyToIdx.insert(make_pair((string)"scaler10_5",ISCALER10_5));
  fKeyToIdx.insert(make_pair((string)"scaler10_6",ISCALER10_6));
  fKeyToIdx.insert(make_pair((string)"scaler10_7",ISCALER10_7));
  fKeyToIdx.insert(make_pair((string)"scaler10_8",ISCALER10_8));
  fKeyToIdx.insert(make_pair((string)"scaler10_9",ISCALER10_9));
  fKeyToIdx.insert(make_pair((string)"scaler10_10",ISCALER10_10));
  fKeyToIdx.insert(make_pair((string)"scaler10_11",ISCALER10_11));
  fKeyToIdx.insert(make_pair((string)"scaler10_12",ISCALER10_12));
  fKeyToIdx.insert(make_pair((string)"scaler10_13",ISCALER10_13));
  fKeyToIdx.insert(make_pair((string)"scaler10_14",ISCALER10_14));
  fKeyToIdx.insert(make_pair((string)"scaler10_15",ISCALER10_15));
  fKeyToIdx.insert(make_pair((string)"scaler10_16",ISCALER10_16));
  fKeyToIdx.insert(make_pair((string)"scaler10_17",ISCALER10_17));
  fKeyToIdx.insert(make_pair((string)"scaler10_18",ISCALER10_18));
  fKeyToIdx.insert(make_pair((string)"scaler10_19",ISCALER10_19));
  fKeyToIdx.insert(make_pair((string)"scaler10_20",ISCALER10_20));
  fKeyToIdx.insert(make_pair((string)"scaler10_21",ISCALER10_21));
  fKeyToIdx.insert(make_pair((string)"scaler10_22",ISCALER10_22));
  fKeyToIdx.insert(make_pair((string)"scaler10_23",ISCALER10_23));
  fKeyToIdx.insert(make_pair((string)"scaler10_24",ISCALER10_24));
  fKeyToIdx.insert(make_pair((string)"scaler10_25",ISCALER10_25));
  fKeyToIdx.insert(make_pair((string)"scaler10_26",ISCALER10_26));
  fKeyToIdx.insert(make_pair((string)"scaler10_27",ISCALER10_27));
  fKeyToIdx.insert(make_pair((string)"scaler10_28",ISCALER10_28));
  fKeyToIdx.insert(make_pair((string)"scaler10_29",ISCALER10_29));
  fKeyToIdx.insert(make_pair((string)"scaler10_30",ISCALER10_30));
  fKeyToIdx.insert(make_pair((string)"scaler10_31",ISCALER10_31));

  fKeyToIdx.insert(make_pair((string)"scaler11_0",ISCALER11_0));
  fKeyToIdx.insert(make_pair((string)"scaler11_1",ISCALER11_1));
  fKeyToIdx.insert(make_pair((string)"scaler11_2",ISCALER11_2));
  fKeyToIdx.insert(make_pair((string)"scaler11_3",ISCALER11_3));
  fKeyToIdx.insert(make_pair((string)"scaler11_4",ISCALER11_4));
  fKeyToIdx.insert(make_pair((string)"scaler11_5",ISCALER11_5));
  fKeyToIdx.insert(make_pair((string)"scaler11_6",ISCALER11_6));
  fKeyToIdx.insert(make_pair((string)"scaler11_7",ISCALER11_7));
  fKeyToIdx.insert(make_pair((string)"scaler11_8",ISCALER11_8));
  fKeyToIdx.insert(make_pair((string)"scaler11_9",ISCALER11_9));
  fKeyToIdx.insert(make_pair((string)"scaler11_10",ISCALER11_10));
  fKeyToIdx.insert(make_pair((string)"scaler11_11",ISCALER11_11));
  fKeyToIdx.insert(make_pair((string)"scaler11_12",ISCALER11_12));
  fKeyToIdx.insert(make_pair((string)"scaler11_13",ISCALER11_13));
  fKeyToIdx.insert(make_pair((string)"scaler11_14",ISCALER11_14));
  fKeyToIdx.insert(make_pair((string)"scaler11_15",ISCALER11_15));
  fKeyToIdx.insert(make_pair((string)"scaler11_16",ISCALER11_16));
  fKeyToIdx.insert(make_pair((string)"scaler11_17",ISCALER11_17));
  fKeyToIdx.insert(make_pair((string)"scaler11_18",ISCALER11_18));
  fKeyToIdx.insert(make_pair((string)"scaler11_19",ISCALER11_19));
  fKeyToIdx.insert(make_pair((string)"scaler11_20",ISCALER11_20));
  fKeyToIdx.insert(make_pair((string)"scaler11_21",ISCALER11_21));
  fKeyToIdx.insert(make_pair((string)"scaler11_22",ISCALER11_22));
  fKeyToIdx.insert(make_pair((string)"scaler11_23",ISCALER11_23));
  fKeyToIdx.insert(make_pair((string)"scaler11_24",ISCALER11_24));
  fKeyToIdx.insert(make_pair((string)"scaler11_25",ISCALER11_25));
  fKeyToIdx.insert(make_pair((string)"scaler11_26",ISCALER11_26));
  fKeyToIdx.insert(make_pair((string)"scaler11_27",ISCALER11_27));
  fKeyToIdx.insert(make_pair((string)"scaler11_28",ISCALER11_28));
  fKeyToIdx.insert(make_pair((string)"scaler11_29",ISCALER11_29));
  fKeyToIdx.insert(make_pair((string)"scaler11_30",ISCALER11_30));
  fKeyToIdx.insert(make_pair((string)"scaler11_31",ISCALER11_31));

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

  fKeyToIdx.insert(make_pair((string)"scaler7_0_clkdiv",ISCALER7_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_1_clkdiv",ISCALER7_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_2_clkdiv",ISCALER7_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_3_clkdiv",ISCALER7_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_4_clkdiv",ISCALER7_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_5_clkdiv",ISCALER7_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_6_clkdiv",ISCALER7_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_7_clkdiv",ISCALER7_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_8_clkdiv",ISCALER7_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_9_clkdiv",ISCALER7_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_10_clkdiv",ISCALER7_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_11_clkdiv",ISCALER7_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_12_clkdiv",ISCALER7_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_13_clkdiv",ISCALER7_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_14_clkdiv",ISCALER7_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_15_clkdiv",ISCALER7_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_16_clkdiv",ISCALER7_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_17_clkdiv",ISCALER7_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_18_clkdiv",ISCALER7_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_19_clkdiv",ISCALER7_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_20_clkdiv",ISCALER7_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_21_clkdiv",ISCALER7_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_22_clkdiv",ISCALER7_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_23_clkdiv",ISCALER7_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_24_clkdiv",ISCALER7_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_25_clkdiv",ISCALER7_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_26_clkdiv",ISCALER7_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_27_clkdiv",ISCALER7_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_28_clkdiv",ISCALER7_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_29_clkdiv",ISCALER7_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_30_clkdiv",ISCALER7_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler7_31_clkdiv",ISCALER7_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler8_0_clkdiv",ISCALER8_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_1_clkdiv",ISCALER8_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_2_clkdiv",ISCALER8_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_3_clkdiv",ISCALER8_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_4_clkdiv",ISCALER8_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_5_clkdiv",ISCALER8_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_6_clkdiv",ISCALER8_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_7_clkdiv",ISCALER8_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_8_clkdiv",ISCALER8_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_9_clkdiv",ISCALER8_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_10_clkdiv",ISCALER8_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_11_clkdiv",ISCALER8_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_12_clkdiv",ISCALER8_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_13_clkdiv",ISCALER8_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_14_clkdiv",ISCALER8_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_15_clkdiv",ISCALER8_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_16_clkdiv",ISCALER8_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_17_clkdiv",ISCALER8_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_18_clkdiv",ISCALER8_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_19_clkdiv",ISCALER8_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_20_clkdiv",ISCALER8_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_21_clkdiv",ISCALER8_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_22_clkdiv",ISCALER8_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_23_clkdiv",ISCALER8_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_24_clkdiv",ISCALER8_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_25_clkdiv",ISCALER8_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_26_clkdiv",ISCALER8_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_27_clkdiv",ISCALER8_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_28_clkdiv",ISCALER8_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_29_clkdiv",ISCALER8_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_30_clkdiv",ISCALER8_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler8_31_clkdiv",ISCALER8_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler9_0_clkdiv",ISCALER9_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_1_clkdiv",ISCALER9_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_2_clkdiv",ISCALER9_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_3_clkdiv",ISCALER9_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_4_clkdiv",ISCALER9_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_5_clkdiv",ISCALER9_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_6_clkdiv",ISCALER9_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_7_clkdiv",ISCALER9_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_8_clkdiv",ISCALER9_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_9_clkdiv",ISCALER9_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_10_clkdiv",ISCALER9_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_11_clkdiv",ISCALER9_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_12_clkdiv",ISCALER9_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_13_clkdiv",ISCALER9_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_14_clkdiv",ISCALER9_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_15_clkdiv",ISCALER9_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_16_clkdiv",ISCALER9_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_17_clkdiv",ISCALER9_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_18_clkdiv",ISCALER9_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_19_clkdiv",ISCALER9_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_20_clkdiv",ISCALER9_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_21_clkdiv",ISCALER9_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_22_clkdiv",ISCALER9_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_23_clkdiv",ISCALER9_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_24_clkdiv",ISCALER9_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_25_clkdiv",ISCALER9_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_26_clkdiv",ISCALER9_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_27_clkdiv",ISCALER9_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_28_clkdiv",ISCALER9_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_29_clkdiv",ISCALER9_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_30_clkdiv",ISCALER9_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler9_31_clkdiv",ISCALER9_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler10_0_clkdiv",ISCALER10_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_1_clkdiv",ISCALER10_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_2_clkdiv",ISCALER10_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_3_clkdiv",ISCALER10_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_4_clkdiv",ISCALER10_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_5_clkdiv",ISCALER10_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_6_clkdiv",ISCALER10_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_7_clkdiv",ISCALER10_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_8_clkdiv",ISCALER10_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_9_clkdiv",ISCALER10_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_10_clkdiv",ISCALER10_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_11_clkdiv",ISCALER10_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_12_clkdiv",ISCALER10_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_13_clkdiv",ISCALER10_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_14_clkdiv",ISCALER10_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_15_clkdiv",ISCALER10_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_16_clkdiv",ISCALER10_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_17_clkdiv",ISCALER10_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_18_clkdiv",ISCALER10_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_19_clkdiv",ISCALER10_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_20_clkdiv",ISCALER10_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_21_clkdiv",ISCALER10_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_22_clkdiv",ISCALER10_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_23_clkdiv",ISCALER10_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_24_clkdiv",ISCALER10_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_25_clkdiv",ISCALER10_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_26_clkdiv",ISCALER10_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_27_clkdiv",ISCALER10_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_28_clkdiv",ISCALER10_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_29_clkdiv",ISCALER10_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_30_clkdiv",ISCALER10_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler10_31_clkdiv",ISCALER10_31_CLKDIV));

  fKeyToIdx.insert(make_pair((string)"scaler11_0_clkdiv",ISCALER11_0_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_1_clkdiv",ISCALER11_1_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_2_clkdiv",ISCALER11_2_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_3_clkdiv",ISCALER11_3_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_4_clkdiv",ISCALER11_4_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_5_clkdiv",ISCALER11_5_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_6_clkdiv",ISCALER11_6_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_7_clkdiv",ISCALER11_7_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_8_clkdiv",ISCALER11_8_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_9_clkdiv",ISCALER11_9_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_10_clkdiv",ISCALER11_10_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_11_clkdiv",ISCALER11_11_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_12_clkdiv",ISCALER11_12_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_13_clkdiv",ISCALER11_13_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_14_clkdiv",ISCALER11_14_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_15_clkdiv",ISCALER11_15_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_16_clkdiv",ISCALER11_16_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_17_clkdiv",ISCALER11_17_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_18_clkdiv",ISCALER11_18_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_19_clkdiv",ISCALER11_19_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_20_clkdiv",ISCALER11_20_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_21_clkdiv",ISCALER11_21_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_22_clkdiv",ISCALER11_22_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_23_clkdiv",ISCALER11_23_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_24_clkdiv",ISCALER11_24_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_25_clkdiv",ISCALER11_25_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_26_clkdiv",ISCALER11_26_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_27_clkdiv",ISCALER11_27_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_28_clkdiv",ISCALER11_28_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_29_clkdiv",ISCALER11_29_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_30_clkdiv",ISCALER11_30_CLKDIV));
  fKeyToIdx.insert(make_pair((string)"scaler11_31_clkdiv",ISCALER11_31_CLKDIV));


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

  fKeyToIdx.insert(make_pair((string)"scaler7_0_cal",ISCALER7_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_1_cal",ISCALER7_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_2_cal",ISCALER7_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_3_cal",ISCALER7_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_4_cal",ISCALER7_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_5_cal",ISCALER7_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_6_cal",ISCALER7_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_7_cal",ISCALER7_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_8_cal",ISCALER7_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_9_cal",ISCALER7_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_10_cal",ISCALER7_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_11_cal",ISCALER7_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_12_cal",ISCALER7_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_13_cal",ISCALER7_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_14_cal",ISCALER7_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_15_cal",ISCALER7_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_16_cal",ISCALER7_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_17_cal",ISCALER7_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_18_cal",ISCALER7_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_19_cal",ISCALER7_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_20_cal",ISCALER7_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_21_cal",ISCALER7_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_22_cal",ISCALER7_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_23_cal",ISCALER7_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_24_cal",ISCALER7_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_25_cal",ISCALER7_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_26_cal",ISCALER7_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_27_cal",ISCALER7_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_28_cal",ISCALER7_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_29_cal",ISCALER7_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_30_cal",ISCALER7_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler7_31_cal",ISCALER7_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler8_0_cal",ISCALER8_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_1_cal",ISCALER8_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_2_cal",ISCALER8_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_3_cal",ISCALER8_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_4_cal",ISCALER8_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_5_cal",ISCALER8_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_6_cal",ISCALER8_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_7_cal",ISCALER8_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_8_cal",ISCALER8_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_9_cal",ISCALER8_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_10_cal",ISCALER8_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_11_cal",ISCALER8_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_12_cal",ISCALER8_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_13_cal",ISCALER8_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_14_cal",ISCALER8_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_15_cal",ISCALER8_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_16_cal",ISCALER8_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_17_cal",ISCALER8_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_18_cal",ISCALER8_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_19_cal",ISCALER8_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_20_cal",ISCALER8_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_21_cal",ISCALER8_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_22_cal",ISCALER8_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_23_cal",ISCALER8_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_24_cal",ISCALER8_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_25_cal",ISCALER8_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_26_cal",ISCALER8_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_27_cal",ISCALER8_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_28_cal",ISCALER8_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_29_cal",ISCALER8_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_30_cal",ISCALER8_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler8_31_cal",ISCALER8_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler9_0_cal",ISCALER9_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_1_cal",ISCALER9_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_2_cal",ISCALER9_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_3_cal",ISCALER9_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_4_cal",ISCALER9_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_5_cal",ISCALER9_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_6_cal",ISCALER9_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_7_cal",ISCALER9_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_8_cal",ISCALER9_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_9_cal",ISCALER9_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_10_cal",ISCALER9_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_11_cal",ISCALER9_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_12_cal",ISCALER9_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_13_cal",ISCALER9_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_14_cal",ISCALER9_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_15_cal",ISCALER9_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_16_cal",ISCALER9_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_17_cal",ISCALER9_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_18_cal",ISCALER9_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_19_cal",ISCALER9_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_20_cal",ISCALER9_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_21_cal",ISCALER9_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_22_cal",ISCALER9_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_23_cal",ISCALER9_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_24_cal",ISCALER9_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_25_cal",ISCALER9_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_26_cal",ISCALER9_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_27_cal",ISCALER9_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_28_cal",ISCALER9_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_29_cal",ISCALER9_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_30_cal",ISCALER9_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler9_31_cal",ISCALER9_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler10_0_cal",ISCALER10_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_1_cal",ISCALER10_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_2_cal",ISCALER10_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_3_cal",ISCALER10_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_4_cal",ISCALER10_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_5_cal",ISCALER10_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_6_cal",ISCALER10_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_7_cal",ISCALER10_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_8_cal",ISCALER10_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_9_cal",ISCALER10_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_10_cal",ISCALER10_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_11_cal",ISCALER10_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_12_cal",ISCALER10_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_13_cal",ISCALER10_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_14_cal",ISCALER10_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_15_cal",ISCALER10_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_16_cal",ISCALER10_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_17_cal",ISCALER10_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_18_cal",ISCALER10_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_19_cal",ISCALER10_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_20_cal",ISCALER10_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_21_cal",ISCALER10_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_22_cal",ISCALER10_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_23_cal",ISCALER10_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_24_cal",ISCALER10_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_25_cal",ISCALER10_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_26_cal",ISCALER10_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_27_cal",ISCALER10_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_28_cal",ISCALER10_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_29_cal",ISCALER10_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_30_cal",ISCALER10_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler10_31_cal",ISCALER10_31_CAL));

  fKeyToIdx.insert(make_pair((string)"scaler11_0_cal",ISCALER11_0_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_1_cal",ISCALER11_1_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_2_cal",ISCALER11_2_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_3_cal",ISCALER11_3_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_4_cal",ISCALER11_4_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_5_cal",ISCALER11_5_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_6_cal",ISCALER11_6_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_7_cal",ISCALER11_7_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_8_cal",ISCALER11_8_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_9_cal",ISCALER11_9_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_10_cal",ISCALER11_10_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_11_cal",ISCALER11_11_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_12_cal",ISCALER11_12_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_13_cal",ISCALER11_13_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_14_cal",ISCALER11_14_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_15_cal",ISCALER11_15_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_16_cal",ISCALER11_16_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_17_cal",ISCALER11_17_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_18_cal",ISCALER11_18_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_19_cal",ISCALER11_19_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_20_cal",ISCALER11_20_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_21_cal",ISCALER11_21_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_22_cal",ISCALER11_22_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_23_cal",ISCALER11_23_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_24_cal",ISCALER11_24_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_25_cal",ISCALER11_25_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_26_cal",ISCALER11_26_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_27_cal",ISCALER11_27_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_28_cal",ISCALER11_28_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_29_cal",ISCALER11_29_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_30_cal",ISCALER11_30_CAL));
  fKeyToIdx.insert(make_pair((string)"scaler11_31_cal",ISCALER11_31_CAL));


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
  fKeyToIdx.insert(make_pair((string)"v2f_clk7",IV2F_CLK7));
  fKeyToIdx.insert(make_pair((string)"v2f_clk8",IV2F_CLK8));
  fKeyToIdx.insert(make_pair((string)"v2f_clk9",IV2F_CLK9));
  fKeyToIdx.insert(make_pair((string)"v2f_clk10",IV2F_CLK10));
  fKeyToIdx.insert(make_pair((string)"v2f_clk11",IV2F_CLK11));

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

  // Linear Array
  // 1-8 diode pads.  
  // Sum = 0th moment
  //   x = 1st moment 
  // rms = 2nd moment
  fKeyToIdx.insert(make_pair((string)"lina1_1r",ILINA1_1R));
  fKeyToIdx.insert(make_pair((string)"lina1_2r",ILINA1_2R));
  fKeyToIdx.insert(make_pair((string)"lina1_3r",ILINA1_3R));
  fKeyToIdx.insert(make_pair((string)"lina1_4r",ILINA1_4R));
  fKeyToIdx.insert(make_pair((string)"lina1_5r",ILINA1_5R));
  fKeyToIdx.insert(make_pair((string)"lina1_6r",ILINA1_6R));
  fKeyToIdx.insert(make_pair((string)"lina1_7r",ILINA1_7R));
  fKeyToIdx.insert(make_pair((string)"lina1_8r",ILINA1_8R));
  fKeyToIdx.insert(make_pair((string)"lina1_1" ,ILINA1_1));
  fKeyToIdx.insert(make_pair((string)"lina1_2" ,ILINA1_2));
  fKeyToIdx.insert(make_pair((string)"lina1_3" ,ILINA1_3));
  fKeyToIdx.insert(make_pair((string)"lina1_4" ,ILINA1_4));
  fKeyToIdx.insert(make_pair((string)"lina1_5" ,ILINA1_5));
  fKeyToIdx.insert(make_pair((string)"lina1_6" ,ILINA1_6));
  fKeyToIdx.insert(make_pair((string)"lina1_7" ,ILINA1_7));
  fKeyToIdx.insert(make_pair((string)"lina1_8" ,ILINA1_8));
  fKeyToIdx.insert(make_pair((string)"lina1sum",ILINA1SUM));
  fKeyToIdx.insert(make_pair((string)"lina1x"  ,ILINA1X));
  fKeyToIdx.insert(make_pair((string)"lina1rms",ILINA1RMS));
  fKeyToIdx.insert(make_pair((string)"lina1xg"  ,ILINA1XG));
  fKeyToIdx.insert(make_pair((string)"lina1rmsg",ILINA1RMSG));

  fKeyToIdx.insert(make_pair((string)"isync0",IISYNC0));
  fKeyToIdx.insert(make_pair((string)"chsync0",ICHSYNC0));
  fKeyToIdx.insert(make_pair((string)"chsync1",ICHSYNC1));
  fKeyToIdx.insert(make_pair((string)"chsync2",ICHSYNC2));
  fKeyToIdx.insert(make_pair((string)"rsync1",IRSYNC1));
  fKeyToIdx.insert(make_pair((string)"rsync2",IRSYNC2));
  fKeyToIdx.insert(make_pair((string)"lsync1",ILSYNC1));
  fKeyToIdx.insert(make_pair((string)"lsync2",ILSYNC2));

};

void TaDevice::InitPvdisList() {
 
// Build the list of PVDIS-specific keys

  PvdisDmax = -1*fgPvdKchk;
  PvdisDmin = fgPvdKchk;
  PvdisPmax = -1*fgPvdKchk;
  PvdisPmin = fgPvdKchk;

  Int_t ldebug=1;
  static string stofind = "dis";
  if (ldebug) cout << "Check InitPvdis for string "<<stofind<<endl;

  for (map<string, Int_t>::const_iterator si = fKeyToIdx.begin(); 
       si != fKeyToIdx.end();  si++) {
       string skey = si->first;
       if (ldebug==2) cout << "find key ? "<<skey<<endl;    
       if (skey.find(stofind) < skey.size()) {
         Int_t key = si->second;
	 pvdisKeys.push_back(key);
	 //    cout << ".... FOUND IT "<<si->second<<"  "<<pvdisKeys.size()<<endl;
         if (key > PvdisDmax) PvdisDmax = key;
         if (key < PvdisDmin) PvdisDmin = key;
       }
  }

// List of raw devices (like scaler6_0) that correspond to 
// derived PVDIS devices (like 'dis*')

  if (PvdisDmin != fgPvdKchk && PvdisDmax != -1*fgPvdKchk) { 
    for (Int_t dkey = PvdisDmin; dkey <= PvdisDmax; dkey++) {
      map<Int_t, Int_t>::const_iterator ki = fRevKeyMap.find(dkey);
      if (ki != fRevKeyMap.end()) {
        Int_t key = fRevKeyMap[dkey];
        if (key > PvdisPmax) PvdisPmax = key;
        if (key < PvdisPmin) PvdisPmin = key;
        if (DECODE_DEBUG) {
	  cout << "Check Pvdis derived key "<< dkey;
          cout << "  raw key  "<<key<<endl;

	}
      }
    }
  }

  if (DECODE_DEBUG || ldebug) {
     cout << "Pvdis Primary key range "<<PvdisPmin<<"  to "<<PvdisPmax<<endl;
     cout << "Pvdis Derived key range "<<PvdisDmin<<"  to "<<PvdisDmax<<endl;
  }

}

Int_t TaDevice::GetMinPvdisPKey() {
  if (PvdisPmin == fgPvdKchk) return -1;
  return PvdisPmin;
}

Int_t TaDevice::GetMaxPvdisPKey() {
  if (PvdisPmax == -1*fgPvdKchk) return -1;
  return PvdisPmax;
}

Int_t TaDevice::GetMinPvdisDKey() {
  if (PvdisDmin == fgPvdKchk) return -1;
  return PvdisDmin;
}

Int_t TaDevice::GetMaxPvdisDKey() {
  if (PvdisDmax == -1*fgPvdKchk) return -1;
  return PvdisDmax;
}



Int_t TaDevice::GetNumPvdisDev() {

  if ((Int_t)pvdisKeys.size() > 0) return pvdisKeys.size();

  return 0;

}

Int_t TaDevice::GetPvdisDev(Int_t i) {

  if (i >= 0 && i < (Int_t)pvdisKeys.size()) return pvdisKeys[i];
  return 0;

}


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
   fAdcxPed = new Double_t[4*ADCXNUM];
   memcpy(fAdcxPed, rhs.fAdcxPed, 4*ADCXNUM*sizeof(Double_t));
   fDacxSlope = new Double_t[4*ADCXNUM];
   memcpy(fDacxSlope, rhs.fDacxSlope, 4*ADCXNUM*sizeof(Double_t));
   fVqwkPed = new Double_t[8*VQWKNUM];
   memcpy(fVqwkPed, rhs.fVqwkPed, 8*VQWKNUM*sizeof(Double_t));
   fDevNum = new Int_t[MAXKEYS];
   memcpy(fDevNum, rhs.fDevNum, MAXKEYS*sizeof(Int_t));
   fChanNum = new Int_t[MAXKEYS];
   memcpy(fChanNum, rhs.fChanNum, MAXKEYS*sizeof(Int_t));
   fAdcptr = new Int_t[MAXROC];
   memcpy(fAdcptr, rhs.fAdcptr, MAXROC*sizeof(Int_t));
   fAdcxptr = new Int_t[MAXROC];
   memcpy(fAdcxptr, rhs.fAdcxptr, MAXROC*sizeof(Int_t));
   fVqwkptr = new Int_t[MAXROC];
   memcpy(fVqwkptr, rhs.fVqwkptr, MAXROC*sizeof(Int_t));
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
   delete [] fVqwkPed;
   delete [] fDacSlope;
   delete [] fAdcxPed;
   delete [] fDacxSlope;
   delete [] fDevNum;
   delete [] fChanNum;
   delete [] fAdcptr;
   delete [] fAdcxptr;
   delete [] fVqwkptr;
   delete [] fScalptr;
   delete [] fTbdptr;
   delete [] fTirptr;
   delete [] fDaqptr;
};


