////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaEvent.cc  (implementation)
//           ^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    An event of data.
//    Includes methods to get data using keys.  For ADCs and 
//    scalers can also get the data by slot number and channel.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define DEBUG

#include "TaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "TaRun.hh"
#include "TaDevice.hh"
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
TaEvent::fFirstDecode = true;

TaEvent::TaEvent(): 
  fEvType(0),  fEvNum(0),  fEvLen(0), fSizeConst(0), fDelHel(UnkHeli)
{
  fFirstCheck = kTRUE;
  fEvBuffer = new Int_t[fgMaxEvLen];
  memset(fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
  fData = new Double_t[MAXKEYS];
  memset(fData, 0, MAXKEYS*sizeof(Double_t));
  fCutFail.clear();
  fCutPass.clear();
  fResults.clear();
  fLoBeam = 0;
  fBurpCut = 0;
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

void TaEvent::Load (const Int_t* buff) {
// Put a raw event into the buffer, and pull out event type and number.
  fCutFail.clear();
  fCutPass.clear();
  fEvLen = buff[0] + 1;
  if (fEvLen >= fgMaxEvLen) {
      cout << "TaEvent::Load ERROR  fEvLen = "<<fEvLen; 
      cout << "  is greater than fgMaxEvLen = "<<fgMaxEvLen<<endl;
      cout << "(perhaps compile with larger fgMaxEvLen parameter)"<<endl;
      fEvLen = fgMaxEvLen;
  }
  memset(fEvBuffer,0,fgMaxEvLen*sizeof(Int_t));
  memcpy(fEvBuffer,buff,fEvLen*sizeof(Int_t));
  fEvType = fEvBuffer[1]>>16;
  fEvNum = 0;
  if ( IsPhysicsEvent() ) fEvNum = fEvBuffer[4];
};

void TaEvent::Decode(const TaDevice& devices) {
// Decodes all the raw data and applies all the calibrations.
// Note: This assumes the event structure remains constant.  
// We check this by verifying a constant event length.
  Int_t i,j,key,ok,ixp,ixm,iyp,iym,ix,iy;
  Double_t sum,xrot,yrot;
  memset(fData, 0, MAXKEYS*sizeof(Double_t));
  if ( IsPhysicsEvent() )  {
    if (fFirstDecode) {
        fSizeConst = GetEvLength();
        fFirstDecode = false;
    }
    if ( fSizeConst != (UInt_t)GetEvLength() ) {
        cout << "TaEvent:: WARNING: Event structure is changing !!"<<endl;
        cout << "As a result, decoding may fail."<<endl;
    }
  }
  for (i = 0; i < devices.GetNumRaw(); i++) {
     key = devices.GetRawKey(i);
     fData[key] =  GetRawData(devices.GetEvPointer(key));
  }
// Calibrate the ADCs first
  for (i = 0;  i < ADCNUM; i++) {
    for (j = 0; j < 4; j++) {
      key = i*4 + j;
      fData[ACCOFF + key] = fData[ADCOFF + key] - devices.GetPedestal(key)
       - (fData[DACOFF + i] * devices.GetDacSlope(key) - devices.GetDacInt(key));
    }
  }
// Stripline BPMs
  for (i = 0; i < STRNUM; i++) {
    ok = 1;
    for (j = 0; j < 4; j++) {
      key = STROFF + 6*i + j;
      if (devices.GetAdcNum(key) < 0 || devices.GetChanNum(key) < 0) {
         ok = 0;   // An adc or channel not found.
         continue;
      }
      fData[key] = fData[ADCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];    
    }
    if ( !ok ) continue;
    key = STROFF + 6*i;
    ixp = ACCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key); 
    ixm = ACCOFF + 4*devices.GetAdcNum(key+1) + devices.GetChanNum(key+1);
    iyp = ACCOFF + 4*devices.GetAdcNum(key+2) + devices.GetChanNum(key+2);
    iym = ACCOFF + 4*devices.GetAdcNum(key+3) + devices.GetChanNum(key+3);
    ix  = key + 4;
    iy  = key + 5;
    sum = fData[ixp] + fData[ixm];
    xrot = 0;
    if ( sum > 0 ) xrot = 
          fgKappa * (fData[ixp] - fData[ixm])/ sum;
    sum = fData[iyp] + fData[iym]; 
    yrot = 0;
    if ( sum > 0 ) yrot = 
          fgKappa * (fData[iyp] - fData[iym])/ sum;
    fData[ix] = Rotate (xrot, yrot, 1);
    fData[iy] = Rotate (xrot, yrot, 2);
  }
// Cavity BPM monitors (when they exist)
  for (i = 0; i < CAVNUM; i++) {
    for (j = 0; j < 2; j++) {
       key = CAVOFF + 4*i + j;
       if (devices.GetAdcNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
       fData[key+2] = 
         fData[ACCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];
// This needs to be divided by current... when they exist.
    }
  }
// Happex-1 era BCMs
  for (i = 0; i < BCMNUM; i++) {
    key = BCMOFF + 2*i;
    if (devices.GetAdcNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    // raw and corrected BCM data
    fData[key] = fData[ADCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];
    fData[key+1] = fData[ACCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];  
  }
// Detectors
  for (i = 0; i < DETNUM; i++) {
    key = DETOFF + 2*i;
    if (devices.GetAdcNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    // raw and corrected detector data
    fData[key] = fData[ADCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];
    fData[key+1] = fData[ACCOFF + 4*devices.GetAdcNum(key) + devices.GetChanNum(key)];  
  }
  fData[IHELICITY] = (Double_t)(((int)GetData(ITIRDATA) & 0x40) >> 6);
  fData[IPAIRSYNCH] = (Double_t)(((int)GetData(ITIRDATA) & 0x80) >> 7);
  fData[ITIMESLOT] = (Double_t)(((int)GetData(IOVERSAMPLE) & 0xff00) >> 8);

};

const vector<pair<ECutType,Int_t> >& 
TaEvent::CheckEvent(TaRun& run)
{
  // Analysis-independent checks of event quality; adds failed cuts
  // to event's list and returns that list.

  Double_t current = GetData(IBCM1);

  Int_t val = 0;
  if (fFirstCheck) {   // this works only if the run doesn't change.
    fLoBeam = run.GetDataBase()->GetCutValue("lobeam");
    fBurpCut = run.GetDataBase()->GetCutValue("burpcut");
    fFirstCheck = kFALSE;
  }

  if ( current < fLoBeam )
    {
#ifdef NOISY
      clog << "Event " << fEvNum << " failed lobeam cut, "
	   << current << " < " << fLoBeam;
#endif
      val = 1;
    }
  AddCut (LowBeamCut, val);
  run.UpdateCutList (LowBeamCut, val, fEvNum);

  if ( fgLastEv.GetEvNumber() != 0 )
    {
      // Not the first event, so check event-to-event differences
      
      // Beam burp -- current change greater than limit?
      val = 0;
      if (abs (current-fgLastEv.GetData(IBCM1)) > fBurpCut)
	{
#ifdef NOISY
	  clog << "Event " << fEvNum << " failed beam burp cut, "
	       << abs (current-fgLastEv.GetData(IBCM1)) << " > " << fBurpCut << endl;
#endif
	  val = 1;
	}
      AddCut (BeamBurpCut, val);
      run.UpdateCutList (BeamBurpCut, val, fEvNum);

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
      run.UpdateCutList (OversampleCut, val, fEvNum);
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
  return (SlotNumber_t)GetData(ITIMESLOT);
};

void TaEvent::SetDelHelicity(EHelicity h)
{
  fDelHel = h;
}

EHelicity TaEvent::GetHelicity() const 
{
  Double_t val = GetData(IHELICITY);
  if (val == 1)
    return RightHeli;
  else
    return LeftHeli;
}

EHelicity TaEvent::GetDelHelicity() const {
  return fDelHel;
}

EPairSynch TaEvent::GetPairSynch() const {
  Double_t val = GetData(IPAIRSYNCH);
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
  static int bpmraw[] = {  IBPM8XP, IBPM8XM, IBPM8YP, IBPM8YM, 
                           IBPM10XP, IBPM10XM, IBPM10YP, IBPM10YM, 
                           IBPM12XP, IBPM12XM, IBPM12YP, IBPM12YM, 
                           IBPM4AXP, IBPM4AXM, IBPM4AYP, IBPM4AYM, 
                           IBPM4BXP, IBPM4BXM, IBPM4BYP, IBPM4BYM };
  static int bpmcalib[] = { IBPM8X, IBPM8Y, IBPM10X, IBPM10Y, 
                           IBPM4AX, IBPM4AY, IBPM4BX, IBPM4BY };
  static int scalers[] = { ISCALER0_0, ISCALER0_1, ISCALER0_2, ISCALER0_3 };
  cout << "\n============   Device Data for Event "<<dec<<GetEvNumber()<<"  ======== "<<endl;
  cout << "Raw BCM1 = 0x"<<hex<<(Int_t)GetData(IBCM1R);
  cout <<"    BCM2 = 0x"<<(Int_t)GetData(IBCM2R)<<endl;
  cout << "Calibrated BCM1 = "<<GetData(IBCM1)<<"  BCM2 = "<<GetData(IBCM2)<<endl; 
  cout << "Raw BPM antenna:   (hex Int_t)"<<endl;
  for (int i = 0; i < 20; i++) {
    cout << dec << bpmraw[i] << "  = 0x"<<hex<<(Int_t)GetData(bpmraw[i])<<"   ";
    if (i > 0 && (((i+1) % 4) == 0)) cout << endl;
  }
  cout << "Calibrated BPM positions:  (Double_t) "<<endl;
  for (int i = 0; i < 8; i++) {
    cout << bpmcalib[i] << " = "<<GetData(bpmcalib[i])<<"    ";
    if (i > 0 && (((i+1) % 2) == 0)) cout << endl;
  }
  cout << "Scalers: "<<endl;
  for (int i = 0; i < 4; i++) {
    cout << dec << scalers[i] << "  = 0x"<<hex<<(Int_t)GetData(scalers[i])<<"   ";
  }
  cout <<endl;
  cout << "tirdata = 0x"<<hex<<(Int_t)GetData(ITIRDATA);  
  cout << "   helicity = "<<dec<<(Int_t)GetData(IHELICITY);
  cout << "   timeslot = "<<(Int_t)GetData(ITIMESLOT);
  cout << "   pairsynch = "<<(Int_t)GetData(IPAIRSYNCH)<<endl;
  cout << "Data by ADC "<<endl;
  for (int adc = 0; adc < 10; adc++) {
    cout << " -> ADC "<<dec<<adc<<endl;
    for (int chan = 0; chan < 4; chan++) {
      cout << "  chan "<<  chan;
      cout << " raw= 0x" << hex << (Int_t)GetRawADCData(adc,chan)<<dec;
      cout << "  cal= " << GetCalADCData(adc,chan);
      if (chan == 0 || chan == 2) cout << "  |   ";
      if (chan == 1) cout << endl;
    }
    cout << endl;
  }
}

Double_t TaEvent::GetData( Int_t key ) const { 
// To find a value corresponding to a data key 
  return fData[Idx(key)];
}; 

Int_t TaEvent::Idx(const Int_t& index) const {
  if (index >= 0 && index < MAXKEYS) return index;
  return 0;
};

Double_t TaEvent::GetRawADCData( Int_t adc, Int_t chan ) const {
// Data raw data for adc # 0, 1, 2...  and channel # 0,1,2,3
  return GetData(ADCOFF + 4*adc + chan);
}; 

Double_t TaEvent::GetCalADCData( Int_t adc, Int_t chan ) const {
// Data calibrated data for adc # 0, 1, 2...and channel # 0,1,2,3
  return GetData(ACCOFF + 4*adc + chan);
}; 

Double_t TaEvent::GetScalerData( Int_t scaler, Int_t chan ) const {
// Data from scaler # 1,2,3..  and channel # 1,2,3...
  return GetData(SCAOFF + 32*scaler + chan);
}; 

void TaEvent::AddToTree(const TaDevice& devices, TTree& rawtree ) {
// Add the data of this device to the raw data tree (root output)
// Called by TaRun::InitDevices()
    Int_t bufsize = 5000;
    char tinfo[20];
    Int_t key;
    map<string, Int_t> fKeyToIdx = devices.GetKeyList();
    for (map<string, Int_t>::iterator ikey = fKeyToIdx.begin(); 
      ikey != fKeyToIdx.end(); ikey++) {
        string keystr = ikey->first;
        key = devices.GetKey(keystr);
        if (key <= 0) continue;
        if (key >= MAXKEYS) {
          cout << "TaEvent::AddToTree::ERROR:  Attempt to add a key = "<<key;
          cout << " larger than array size MAXKEYS = "<<MAXKEYS<<endl;
          cout << "Compile with a bigger MAXKEYS value."<<endl;
          continue;
        }
        strcpy(tinfo,keystr.c_str());  strcat(tinfo,"/D");
  	rawtree.Branch(keystr.c_str(), &fData[key], tinfo, bufsize);
    }
};

// Private methods

Double_t TaEvent::Rotate(Double_t x, Double_t y, Int_t xy) {
// Rotation to get X or Y depending on xy flag
   Double_t result = 0;
   Double_t root2 = (Double_t)sqrt(2);
   if (xy == 2) {
       result = ( x + y ) / root2;
   } else {
       result = ( x - y ) / root2;
   }
   return result;
};

void TaEvent::Create(const TaEvent& rhs)
{
 fEvType = rhs.fEvType;
 fEvNum = rhs.fEvNum;
 fEvLen = rhs.fEvLen;
 fSizeConst = rhs.fSizeConst;
 fCutFail = rhs.fCutFail;
 fCutPass = rhs.fCutPass;
 fResults = rhs.fResults;
 fDelHel = rhs.fDelHel;
 fFirstCheck = rhs.fFirstCheck;
 fLoBeam = rhs.fLoBeam;
 fBurpCut = rhs.fBurpCut;
 fEvBuffer = new Int_t[fgMaxEvLen];
 memset (fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
 memcpy(fEvBuffer, rhs.fEvBuffer, fEvLen*sizeof(Int_t));
 fData = new Double_t[MAXKEYS];
 memcpy(fData, rhs.fData, MAXKEYS*sizeof(Double_t));
};

void TaEvent::Uncreate()
{
  delete [] fEvBuffer;
  delete [] fData;
};






