//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaEvent.cc  (implementation)
//           ^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    An event of data.
//    Includes methods to get data using keys.  For ADCs one
//    can also get the data by ADC number and channel.
//    The user (presumaby a TaRun) must LoadDevice() at
//    least once in the life of TaEvent, presumably once
//    per run.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define DEBUG

#include "TaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "TaRun.hh"
#include "VaDevice.hh"
#include "VaDataBase.hh"
#include "TaAsciiDB.hh"
#include <iostream>
#include <utility>
#include <cmath>

#ifdef DICT
ClassImp(TaEvent)
#endif

TaEvent
TaEvent::fgLastEv;

Bool_t 
TaEvent::fgDidInit = false;

Bool_t 
TaEvent::fFirstDecode = true;

TaEvent::TaEvent(): 
  fEvType(0),  fEvNum(0),  fEvLen(0), fSizeConst(0), fDelHel(UnkHeli)
{
  fEvBuffer = new Int_t[fgMaxEvLen];
  memset(fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
  fCutFail.clear();
  fCutPass.clear();
  fResults.clear();
}

TaEvent::~TaEvent() {
  Uncreate();
}

TaEvent::TaEvent(const TaEvent& ev) 
{
  Create (ev);
} 


TaEvent &TaEvent::operator=(const TaEvent &ev)
{
  if ( &ev != this )
    {
      Uncreate();
      Create (ev);
    }
  return *this;
}

// Major functions

void TaEvent::InitDevices( map<string, VaDevice* >& dev) {
// Initialize the map of devices and the map of keys.
// The device map is a non-const copy obtained from TaRun 
// since TaRun initializes this map.  This method MUST be called 
// at least once, and presumably only once per run.
  fDevices = dev;
  fgDidInit = kTRUE;
  pair<string, string> ssp;
  pair<string, int> sip;
  pair<int, int> iip;
  for (map<string, VaDevice* >::iterator dmap = dev.begin(); 
   dmap != dev.end();  dmap++) {
    VaDevice *device = dmap->second;
    vector<string> keys = device->GetKeys();
    for (vector<string>::iterator ikey = keys.begin(); 
      ikey != keys.end();  ikey++) {
        string key = *ikey;
  	ssp.first = key;  ssp.second = dmap->first;
        pair<map<string, string>::iterator,bool> p = fKeyDev.insert(ssp);
        sip.first = key;  sip.second = 1;          
        if ( !(p.second) ) sip.second = 0;  // device key not unique in event.
        fKeyUni.insert(sip);  
        if (device->IsADC() && device->IsRaw(key)) {
          if (device->GetChannel(key) >= 0) {
   	    iip.first = device->GetADCNumber(); iip.second = device->GetChannel(key);
            ssp.first = device->GetName();  ssp.second = key;
            adcs.insert( make_pair (iip, ssp) );                      
	  }
	}
    }
  }
#ifdef DEBUG
   for (map<string, VaDevice* >::iterator dmap = dev.begin(); 
   dmap != dev.end();  dmap++) {
    VaDevice *device = dmap->second;
    vector<string> keys = device->GetKeys();
    cout << "------ Device : "<<device->GetName()<<endl;
    for (vector<string>::iterator ikey = keys.begin(); 
      ikey != keys.end();  ikey++) {
        string key = *ikey;
        cout << "       -- Key   "<<key<<endl;
        if (device->IsADC() && device->IsRaw(key)) {
          cout << "             Is a raw ADC "<<endl;
          cout << "             adc num "<<device->GetADCNumber();
          cout << "   channel "<<device->GetChannel(key)<<endl;
          iip = make_pair(device->GetADCNumber(),device->GetChannel(key));
          ssp = adcs[iip];
          cout << "             corresp. device "<<ssp.first;
          cout << "   and key "<<ssp.second<<endl;
	}
    }
   }
#endif
};

void TaEvent::Load (const Int_t* buff) {
  // Put a raw event into the buffer, and pull out event type and number.
  fCutFail.clear();
  fCutPass.clear();
  fEvLen = buff[0] + 1;
  if (fEvLen >= fgMaxEvLen) {
      cout << "TaEvent::Load Warning, event is anomalously large"<<endl;
      fEvLen = fgMaxEvLen;
  }
  memset(fEvBuffer,0,fgMaxEvLen*sizeof(Int_t));
  memcpy(fEvBuffer,buff,fEvLen*sizeof(Int_t));
  fEvType = fEvBuffer[1]>>16;
  fEvNum = 0;
  if ( IsPhysicsEvent() ) fEvNum = fEvBuffer[4];
};

void TaEvent::Decode() {
// Decode all raw data, which are VaDevices.
// Note: This assumes the event structure remains constant for
// the given run (a rather safe assumption for parity DAQ).  One
// somewhat weak check on this is to verify a constant event length.
    if ( IsPhysicsEvent() )  {
      if (fFirstDecode) {
         fSizeConst = GetEvLength();
         fFirstDecode = false;
      }
      if ( fSizeConst != (long)GetEvLength() ) {
         cout << "VaDevice:: WARNING: Event structure is changing !!"<<endl;
         cout << "As a result, decoding may fail."<<endl;
      }
    }
    for (map < string, VaDevice* >::iterator devmap = fDevices.begin();
       devmap != fDevices.end(); devmap++) (devmap->second)->Decode(*this);
};

const vector<pair<ECutType,Int_t> >& TaEvent::CheckEvent(const TaRun& run)
{
  // Analysis-independent checks of event quality; adds failed cuts
  // to event's list and returns that list.

  Double_t current = GetData("bcm1");

  Int_t val = 0;
  if ( current < run.GetDataBase()->GetCutValue("lobeam") )
    {
#ifdef NOISY
      clog << "Event " << fEvNum << " failed lobeam cut, "
	   << current << " < " << run.GetDataBase()->GetCutValue("lobeam") << endl;
#endif
      val = 1;
    }
  AddCut (LowBeamCut, val);

  if ( fgLastEv.GetEvNumber() != 0 )
    {
      // Not the first event, so check event-to-event differences
      
      // Beam burp -- current change greater than limit?
      val = 0;
      if (abs (current-fgLastEv.GetData("bcm1")) > 
	  run.GetDataBase()->GetCutValue("burpcut") )
	{
#ifdef NOISY
	  clog << "Event " << fEvNum << " failed beam burp cut, "
	       << abs (current-fgLastEv.GetData("bcm1")) << " > " << run.GetDataBase()->GetCutValue("burpcut") << endl;
#endif
	  val = 1;
	}
      AddCut (BeamBurpCut, val);

      // Check time slot sequence
      val = 0;
      if ( run.GetOversample() > 0 )
	{ 
	  if ( GetTimeSlot() != 
	       fgLastEv.GetTimeSlot() % run.GetOversample() + 1 ) 
	    {
	      cout << "TaEvent::CheckEvent ERROR Event " 
		   << GetEvNumber() 
		   << " unexpected oversample value, went from " 
		   << fgLastEv.GetTimeSlot()
		   << " to " 
		   << GetTimeSlot() 
		   << endl;
	      val = 1;
	    } 
	}
      AddCut (OversampleCut, val);
    }
  
  fgLastEv = *this;

  return fCutFail;
};


void TaEvent::AddCut (const ECutType cut, const Int_t val)
{
  // Store information about cut conditions passed or failed by this event.
  if (val == 0)
    fCutPass.push_back (make_pair (cut, val));
  else
    fCutFail.push_back (make_pair (cut, val));
};


void TaEvent::AddResult( const TaLabelledQuantity& result)
{
  // Store a result from analysis of this event.
  fResults.push_back (result);
};


// Data access functions

Int_t TaEvent::GetRawData(Int_t index) const {
  if (index >= 0 && (UInt_t)index < fgMaxEvLen) 
    return fEvBuffer[index];
  else
    {
      cout << "TaEvent::GetRawData ERROR: index " << index 
	   << "out of range 0 to " << fgMaxEvLen;
      return 0;
    }
};

Double_t TaEvent::GetData(const string& devicename, const string& key) const {
// Get data from device "devicename" with key "key"
// Restriction:  Keys must be unique for a given devicename.
  static map<string, VaDevice* >::const_iterator d;
  if ( !fgDidInit ) {
    cout << "TaEvent:: ERROR: Did not initialize TaEvent"<<endl;
    return 0;
  }
  d = fDevices.find(devicename);
  if (d != fDevices.end()) 
    return (d->second)->GetData(key);
  else
    {
      cout << "TaEvent::GetData ERROR: Device " << devicename 
	   << " not found" << endl;
      return 0;
    }
}; 

Double_t TaEvent::GetData(const string& key) const {  
// Get data for a key (e.g. key = 'bcm1', 'bpm8xp', 'bpm8x')
// Restriction for using this method: key must be unique in the event.
// If not, then use GetData(device,key) instead.
  static map<string, string>::const_iterator sd;
  static map <string, int>::const_iterator si;
  if ( !fgDidInit ) {
    cout << "TaEvent:: ERROR: Did not initialize TaEvent"<<endl;
    return 0;
  }
  sd = fKeyDev.find(key);

  if (sd == fKeyDev.end()) 
    {
      cout << "TaEvent::GetData ERROR: Key " << key << " not found. "<<endl;
      if (TAEVENT_VERBOSE) {
 	 cout << "Reasons why a key may not be found :"<<endl;
         cout << "   > 1. A TaEvent you are using is out of scope ?"<<endl;
         cout << "     2. For raw data, key omitted from database ?"<<endl;
         cout << "     3. For processed data, the key not added by any class ?"<<endl;
         cout << "     4. Typo, etc"<<endl;
      }
      return 0;
    }
  si = fKeyUni.find(key);

  if ( si == fKeyUni.end() ) {
    cout << "TaEvent:: GetData ERROR:  Key " << key 
         << " is not unique within the event, so use the"
         << " GetData(device, key) method instead." << endl;
    return 0;
  }
  string device = sd->second;
  return GetData(device, key);
};

Double_t TaEvent::GetData(const string& devicename, const Int_t& channel) const {
// Get data from device "devicename" at a channel offset, if defined.
// This works for device channels with name devicename-K in database, where
// K = channel, e.g. "scaler1-24" is devicename="scaler1" and channel 24.
  static map<string, VaDevice* >::const_iterator d;
  if ( !fgDidInit ) {
    cout << "TaEvent:: ERROR: Did not initialize TaEvent"<<endl;
    return 0;
  }
  d = fDevices.find(devicename);
  if (d != fDevices.end()) return (d->second)->GetData(channel);
  return 0;
};

Double_t TaEvent::GetADCData(const Int_t& slot, const Int_t& chan) const {
// Return the Princeton-ADC data in slot=0,1,2...etc, and channel=0,1,2,3
  static pair<string, string> ssp;
  //  cout<<"GETADCData::slot and pair debug, INPUT :"<<slot<<" , "<<chan<<endl;
  static map<pair<int,int>, pair<string,string> >::const_iterator iiss;
  iiss = adcs.find(make_pair(slot,chan));
  ssp = iiss->second;
  //  ssp  = adcs[make_pair(slot,chan)];  // get device and key
  if (fDevices.find(ssp.first) == fDevices.end()) return 0;
  //  cout<< "GETADCData::slot and pair debug, OUTPUT :"<<ssp.first<<" , "<<ssp.second<<endl;
  return GetData(ssp.first, ssp.second);
};


Bool_t TaEvent::CutStatus() const {
  return (fCutFail.size() > 0);    
};

Bool_t TaEvent::IsPrestartEvent() const {
  return (fEvType == 17);
};

Bool_t TaEvent::IsPhysicsEvent() const {
  return (fEvType >= 1 && fEvType < 12);
};

EventNumber_t TaEvent::GetEvNumber() const {
  return fEvNum;
};

UInt_t TaEvent::GetEvLength() const {
  return fEvLen;
};

UInt_t TaEvent::GetEvType() const {
  return fEvType;
};

SlotNumber_t TaEvent::GetTimeSlot() const {
  // In all cases, it appears that this function has been used
  // with the idea that it returns the value of the
  // current oversampling period.
  // Remember: the TIR timeslot has nothing to do with oversampling
  // We should probably rename this function to avoid future confusion

  //  return (SlotNumber_t)GetData("timeslot");  
  return (SlotNumber_t)GetData("oversample_bin");
};

void TaEvent::SetDelHelicity(EHelicity h)
{
  fDelHel = h;
}

EHelicity TaEvent::GetHelicity() const 
{
  Double_t val = GetData("helicity");
  if (val == 1)
    return RightHeli;
  else
    return LeftHeli;
}

EHelicity TaEvent::GetDelHelicity() const {
  return fDelHel;
}

EPairSynch TaEvent::GetPairSynch() const {
  Double_t val = GetData("pairsynch");
  if (val == 1)
    return FirstPS;
  else
    return SecondPS;
};

const vector < TaLabelledQuantity > & TaEvent::GetResults() const 
{ 
  return fResults; 
};

const vector <pair<ECutType,Int_t> > & TaEvent::GetCuts() const
{ 
  return fCutFail; 
};

const vector <pair<ECutType,Int_t> > & 
TaEvent::GetCutsPassed() const
{ 
  return fCutPass; 
};

void TaEvent::RawDump() const {
// Diagnostic dump of raw data for debugging purposes
   cout << "\n\n==========  Raw Data Dump  ==========" << hex << endl;
   cout << "\n Event number  " << dec << GetEvNumber();
   cout << "  length " << GetEvLength() << "  type " << GetEvType() << endl;
   cout << "\n Hex (0x) | Dec (d) format for each data" << endl;
   UInt_t ipt = 0;
   for (UInt_t j = 0; j < GetEvLength()/5; j++) {
       cout << dec << "\n fEvBuffer[" << ipt << "] = ";
       for (UInt_t k = j; k < j+5; k++) {
	 cout << hex << " 0x"<< GetRawData(ipt);
	 cout << " = (d)"<< dec << GetRawData(ipt) << " |";
         ipt++;
       }
   }
   if (ipt < GetEvLength()) {
      cout << dec << "\n fEvBuffer[" << ipt << "] = ";
      for (UInt_t k = ipt; k < GetEvLength(); k++) {
	cout << hex << " 0x"<< GetRawData(ipt);
	cout << " = (d)"<< dec << GetRawData(ipt) << " |";
        ipt++;
      }
      cout << endl;
   }
   cout << "--------------------------------\n\n";
};

void TaEvent::DeviceDump() const {
// Diagnostic dump of device data for debugging purposes.
// Of course this presumes prior knowledge of the device keys, which
// violates our philosophy, but it is only for debugging & illustration.
  static string bpmraw[] = { "bpm8xp", "bpm8xm", "bpm8yp", "bpm8ym", 
                             "bpm10xp", "bpm10xm", "bpm10yp", "bpm10ym",
                             "bpm12xp", "bpm12xm", "bpm12yp", "bpm12ym",
                             "bpm4axp", "bpm4axm", "bpm4ayp", "bpm4aym",
                             "bpm4bxp", "bpm4bxm", "bpm4byp", "bpm4bym" };
  static string bpmcook[] = { "bpm8x", "bpm8y", "bpm10x", "bpm10y",
               "bpm12x", "bpm12y", "bpm4ax", "bpm4ay", "bpm4ba", "bpm4by" };
  static string scalers[] = { "scaler1_1", "scaler1_2", "scaler1_3", "scaler1_4" };
  
  cout << "\n============   Device Data for Event "<<dec<<GetEvNumber()<<"  ======== "<<endl;
  cout << "Raw BCM1 = 0x"<<hex<<(Int_t)GetData("bcm1r");
  cout <<"    BCM2 = 0x"<<(Int_t)GetData("bcm2r")<<endl;
  cout << "Calibrated BCM1 = "<<GetData("bcm1")<<"  BCM2 = "<<GetData("bcm2")<<endl; 
  cout << "Raw BPM antenna:   (hex Int_t)"<<endl;
  for (int i = 0; i < 20; i++) {
    cout << bpmraw[i] << "  = 0x"<<hex<<(Int_t)GetData(bpmraw[i])<<"   ";
    if (i > 0 && (((i+1) % 4) == 0)) cout << endl;
  }
  cout << "Calibrated BPM data:  (Double_t) "<<endl;
  for (int i = 0; i < 10; i++) {
    cout << bpmcook[i] << "   "<<GetData(bpmraw[i])<<"   ";
    if (i > 0 && (((i+1) % 2) == 0)) cout << endl;
  }
  cout << "Scalers: "<<endl;
  for (int i = 0; i < 4; i++) {
    cout << scalers[i] << "  = 0x"<<hex<<(Int_t)GetData(scalers[i])<<"   ";
  }
  cout <<endl;
  DumpBits (false);
  cout << "Data by ADC  (hex Int_t)"<<endl;
  for (int adc = 0; adc < 10; adc++) {
    cout << "ADC "<<dec<<adc<<"  = "<<hex;
    for (int chan = 0; chan < 4; chan++) cout << "  "<<(Int_t)GetADCData(adc,chan);
    cout << endl;
  }
}


void
TaEvent::DumpBits (const Bool_t showEvNum = true) const
{
  // Diagnostic dump of trigger, helicity, timeslot, and pairsynch for
  // debugging purposes

  Int_t timeslot = (Int_t) GetData ("timeslot");
  Int_t OSslot = (Int_t) GetData ("oversample_bin");
  if (showEvNum)
    cout << "Event " << GetEvNumber() << "   ";
  cout << "tirdata = 0x" << hex << (Int_t) GetData ("tirdata");  
  cout << "   helicity = " << dec << (Int_t) GetData ("helicity");
  cout << "   timeslot = " << timeslot;
  cout << "   pairsynch = " << (Int_t) GetData ("pairsynch");
  cout << "   oversample_slot = " << OSslot;
  if (OSslot == 1)
    cout << " *** ";
  cout <<endl;
}


// Private methods

void TaEvent::Create(const TaEvent& rhs)
{
 fEvType = rhs.fEvType;
 fEvNum = rhs.fEvNum;
 fEvLen = rhs.fEvLen;
 fSizeConst = rhs.fSizeConst;
 fCutFail = rhs.fCutFail;
 fCutPass = rhs.fCutPass;
 fResults = rhs.fResults;
 fKeyDev = rhs.fKeyDev;
 fKeyUni = rhs.fKeyUni; 
 fDelHel = rhs.fDelHel;
 fEvBuffer = new Int_t[fgMaxEvLen];
 memset (fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
 memcpy(fEvBuffer, rhs.fEvBuffer, fEvLen*sizeof(Int_t));
 map < string, VaDevice* > devs = rhs.fDevices;
 for (map<string, VaDevice*>::iterator imap = devs.begin();
     imap != devs.end(); imap++) {
       string name = imap->first;
       VaDevice *dev = new VaDevice(*(imap->second));
       fDevices.insert ( make_pair(name,dev) );
 }
};

void TaEvent::Uncreate()
{
  delete [] fEvBuffer;
  for (map<string, VaDevice*>::iterator imap = fDevices.begin();
      imap != fDevices.end(); imap++) {
         delete imap->second;
  }
  fDevices.clear();
};
