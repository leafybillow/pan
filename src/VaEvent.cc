//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           VaEvent.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    An event of data. Includes methods to get data using keys.  For
//    ADCs and scalers can also get the data by slot number and
//    channel.  Events are loaded from a data source and may have
//    analysis results added to them; they may be checked for cut
//    conditions.
//
////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define DEBUG
#define SANE_DECODE 1

#include "VaEvent.hh"
#include "TaCutList.hh"
#include "TaLabelledQuantity.hh"
#include "TaRun.hh"
#include "TaString.hh"
#include "TaDevice.hh"
#include "TaDataBase.hh"
#include <iostream>
#include <iomanip>
#include <utility>
#include <cmath>

#ifndef NODICT
ClassImp(VaEvent)
#endif

// Static members
const Int_t ADC_MinDAC = 3000;
const ErrCode_t VaEvent::fgTAEVT_OK = 0;
const ErrCode_t VaEvent::fgTAEVT_ERROR = -1;
VaEvent VaEvent::fgLastEv;
Bool_t VaEvent::fgFirstDecode = true;
Double_t VaEvent::fgLoBeam;
Double_t VaEvent::fgBurpCut;
Cut_t VaEvent::fgLoBeamNo;
Cut_t VaEvent::fgBurpNo;  
Cut_t VaEvent::fgEvtSeqNo;
Cut_t VaEvent::fgStartupNo;
UInt_t VaEvent::fgOversample;
UInt_t VaEvent::fgCurMon;
UInt_t VaEvent::fgSizeConst;
UInt_t VaEvent::fgNCuts;


VaEvent::VaEvent(): 
  fEvType(0),  
  fEvNum(0),  
  fEvLen(0), 
  fFailedACut(false), 
  fHel(UnkHeli),
  fPrevROHel(UnkHeli),
  fPrevHel(UnkHeli)
{
  fEvBuffer = new Int_t[fgMaxEvLen];
  memset(fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
  fData = new Double_t[MAXKEYS];
  memset(fData, 0, MAXKEYS*sizeof(Double_t));
  fN1roc = new Int_t[MAXROC];
  memset(fN1roc, 0, MAXROC*sizeof(Int_t));
  fLenroc = new Int_t[MAXROC];
  memset(fLenroc, 0, MAXROC*sizeof(Int_t));
  fIrn = new Int_t[MAXROC];
  memset(fIrn, 0, MAXROC*sizeof(Int_t));
  if (fgNCuts > 0)
    {
      fCutArray = new Int_t[fgNCuts];
      memset(fCutArray, 0, fgNCuts*sizeof(Int_t));
    }
  else
    fCutArray = 0;
  fResults.clear();
}

VaEvent::~VaEvent() {
  Uncreate();
}

VaEvent::VaEvent(const VaEvent& ev) 
{
  Create (ev);
} 

VaEvent &VaEvent::operator=(const VaEvent &ev)
{
  if ( &ev != this )
    {
      Uncreate();
      Create (ev);
    }
  return *this;
}

VaEvent& 
VaEvent::CopyInPlace (const VaEvent& rhs)
{
  // Like operator=, but copy an event into existing memory, not
  // deleting and reallocating.  This should always be safe, but just
  // to minimize possible problems use this instead of operator= only
  // when pointers must be preserved.

  if ( &rhs != this )
    {
      fEvType = rhs.fEvType;
      fEvNum = rhs.fEvNum;
      fEvLen = rhs.fEvLen;
      fFailedACut = rhs.fFailedACut;
      fResults = rhs.fResults;
      fHel = rhs.fHel;
      fPrevROHel = rhs.fPrevROHel;
      fPrevHel = rhs.fPrevHel;
      memcpy(fEvBuffer, rhs.fEvBuffer, fEvLen*sizeof(Int_t));
      memcpy(fData, rhs.fData, MAXKEYS*sizeof(Double_t));
      if (rhs.fCutArray != 0 && fgNCuts > 0)
	memcpy(fCutArray, rhs.fCutArray, fgNCuts*sizeof(Int_t));
      else
	fCutArray = 0;
    }
  return *this;
};


// Major functions

ErrCode_t
VaEvent::RunInit(const TaRun& run)
{
  // Initialization at start of run.  Get quantities from database
  // which will be needed in event loop, and set other static variables
  // to initial values.
  cout << "VaEvent:: RunInit() " << endl;

  fgFirstDecode = true;

  fgLoBeam = run.GetDataBase().GetCutValue("lobeam");
  fgBurpCut = run.GetDataBase().GetCutValue("burpcut");

  fgNCuts = (UInt_t) run.GetDataBase().GetNumCuts();

  fgLoBeamNo = run.GetDataBase().GetCutNumber ("Low_beam");
  fgBurpNo = run.GetDataBase().GetCutNumber ("Beam_burp");
  fgEvtSeqNo = run.GetDataBase().GetCutNumber ("Evt_seq");
  fgStartupNo = run.GetDataBase().GetCutNumber ("Startup");
  if (fgEvtSeqNo == fgNCuts)
    fgEvtSeqNo = run.GetDataBase().GetCutNumber ("Oversample"); // backward compat
  if (fgLoBeamNo == fgNCuts ||
      fgEvtSeqNo == fgNCuts)
    {
      cerr << "VaEvent::RunInit ERROR: Following cut(s) are not defined "
	   << "in database and are required:";
      if (fgLoBeamNo == fgNCuts) cerr << " Low_beam";
      if (fgEvtSeqNo == fgNCuts) cerr << " Evt_seq";
      cerr << endl;
      return fgTAEVT_ERROR;
    }
  if (fgBurpNo == fgNCuts ||
      fgStartupNo == fgNCuts)
    {
      cerr << "VaEvent::RunInit WARNING: Following cut(s) are not defined "
	   << "in database and will not be imposed:";
      if (fgBurpNo == fgNCuts) cerr << " Beam_burp";
      if (fgStartupNo == fgNCuts) cerr << " Startup";
      cerr << endl;
    }

  string scurmon = run.GetDataBase().GetCurMon();
  if (scurmon == "none") scurmon = "bcm1";
  fgCurMon = run.GetKey (scurmon);

  fgOversample = run.GetOversample();
  fgLastEv = VaEvent();

  vector<Double_t> qpd1const = run.GetDataBase().GetQpd1Const();
  Int_t ip=0;
  for(vector<Double_t>::iterator iconst = qpd1const.begin();
      iconst != qpd1const.end(); iconst++)  fQPD1Pars[ip++] = *iconst;

  return fgTAEVT_OK;

}
  
void 
VaEvent::Load (const Int_t* buff) 
{
  // Put a raw event into the buffer, and pull out event type and number.

  fFailedACut = false;
  fEvLen = buff[0] + 1;
  if (fEvLen >= fgMaxEvLen) {
      cerr << "VaEvent::Load ERROR  fEvLen = "<<fEvLen; 
      cerr << "  is greater than fgMaxEvLen = "<<fgMaxEvLen<<endl;
      cerr << "(perhaps compile with larger fgMaxEvLen parameter)"<<endl;
      fEvLen = fgMaxEvLen;
  }
  memset(fEvBuffer,0,fgMaxEvLen*sizeof(Int_t));
  memcpy(fEvBuffer,buff,fEvLen*sizeof(Int_t));
  fEvType = fEvBuffer[1]>>16;
  fEvNum = 0;
  if ( IsPhysicsEvent() ) fEvNum = fEvBuffer[4];
};

void VaEvent::DecodeCook(TaDevice& devices) {
// Determine at start-up which cooked data to add to tree.
// This uses a feature of Decode() but ensures that users of
// this class don't think it is a physics event.

  fEvType = 999;   // Large event type to "trick" Decode().
  Decode(devices);
};

void VaEvent::Decode(TaDevice& devices) {
// Decodes all the raw data and applies all the calibrations and BPM
// rotations.  Note: This assumes the event structure remains
// constant.  We check this by verifying a constant event length.
// Also note that in order for cooked data to appear in the output,
// everywhere we have fData[cook_key] = function of fData[raw_key], 
// we MUST have a line devices.SetUsed(cook_key) if we want it.

  Int_t i,j,key,idx,ixp,ixm,iyp,iym,ix,iy,icra;
  Int_t ixpyp,ixpym,ixmyp,ixmym;

  Double_t sum,xval,yval;
  memset(fData, 0, MAXKEYS*sizeof(Double_t));
  if ( IsPhysicsEvent() )  {
    if (fgFirstDecode) {
        fgSizeConst = GetEvLength();
        fgFirstDecode = false;
    }
    if ( fgSizeConst != (UInt_t)GetEvLength() ) {
        cerr << "VaEvent:: FATAL ERROR: Event structure is changing !!"<<endl;
	cerr << "Size was " << fgSizeConst << " now is " << (Int_t)GetEvLength() << endl;
        cerr << "As a result, decoding will fail."<<endl;
        exit(0);  // This should never happen, but if it does Bob needs to fix some stuff !
    }
  }

  if (DecodeCrates(devices) == 1) {
    for (i = 0; i < devices.GetNumRaw(); i++) {
      key = devices.GetRawKey(i);
      icra = devices.GetCrate(key);     
// EvPointer Convention:  If the crate number is <= 0, then the 
// event pointer is an absolute offset.  If crate > 0, then the 
// pointer is relative to the header for that device and crate.
      if (icra <= 0) {
#ifdef SANE_DECODE
// This is almost certainly an error, but ... if you know
// better you can turn off the ifdef at top of this code.
        if (devices.GetEvPointer(key) < 2) {
	  cout << "VaEvent::WARNING: Probably wrong datamap entry ";
          cout << "for key = "<<devices.GetKey(key)<<endl;
	}
#endif
        fData[key] =  GetRawData(devices.GetEvPointer(key));
      } else {
        fData[key] =  GetRawData(
          devices.GetOffset(key)+devices.GetEvPointer(key) );
      }
      if (DECODE_DEBUG == 1) {
        cout << endl << "------------------" <<endl;
        cout << "Raw Data for key "<< devices.GetKey(key)<<dec<<endl;
        if (icra <= 0) {
          cout << " abs. offset " << devices.GetEvPointer(key);
        } else {
	  cout << " tied to crate " << icra;
          cout << "   at offset " << devices.GetOffset(key);
          cout << "   rel. to offset " << devices.GetEvPointer(key);
        }
        cout << endl;
        cout << " Data word = (dec) "<<dec<<(Int_t)fData[key];
        cout << "   = (hex) "<<hex<<(Int_t)fData[key]<<endl; 
      }
    }
  }

// Calibrate the ADCs first
  for (i = 0;  i < ADCNUM; i++) {
    for (j = 0; j < 4; j++) {
      key = i*4 + j;
      fData[ACCOFF + key] = fData[ADCOFF + key] - devices.GetPedestal(ADCOFF + key)
       - (fData[DACOFF + i]-ADC_MinDAC) * devices.GetDacSlope(key);
      if (devices.IsUsed(ADCOFF+key)) devices.SetUsed(ACCOFF+key);
    }
  }
  
  // Calibrate Scalers for use with v2fs
  // Only one V2F is allowed per scaler.  Sorry.
  Int_t clockkey;
  for (i = 0; i < V2FCLKNUM; i++) {
    clockkey = i + V2FCLKOFF;
    // If there's no clock... then there's no calibration.
    if (fData[clockkey] == 0) {
      for (j = 0; j < 32; j++) {
	key = j + i*32;
	fData[SCCOFF + key] = 0;
        if (devices.IsUsed(SCAOFF+key)) devices.SetUsed(SCCOFF+key);
      }
    } else {
      // HA! There IS a clock!
      Double_t clock = fData[clockkey];
      for (j = 0; j < 32; j++) {
	key = j + i*32;
	fData[SCCOFF + key] = 
	  (fData[SCAOFF + key] 
	   - devices.GetPedestal(SCAOFF + key))/clock;
        if (devices.IsUsed(SCAOFF+key)) devices.SetUsed(SCCOFF+key);
      }
    }
  }  

// Batteries
  for (i = 0; i < BATNUM; i++) {
    key = BATOFF + i;
    if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    idx = devices.GetCalIndex(key);
    if (idx < 0) continue;
    fData[key] = fData[idx];
    if (devices.IsUsed(key)) devices.SetUsed(key);
  }

// Quad photodiode
  for (i = 0; i < QPDNUM; i++) {
    key = QPDOFF + 9*i;
    ixpyp = devices.GetCalIndex(key);
    ixpym = devices.GetCalIndex(key+1);
    ixmyp = devices.GetCalIndex(key+2);
    ixmym = devices.GetCalIndex(key+3);
    if (ixpyp < 0 || ixpym < 0 || ixmyp < 0 || ixmym < 0) continue;
    if (i==0) {  // kludgey way to use relative gains for qpd1
       fData[ixpyp] *= fQPD1Pars[0];
       fData[ixpym] *= fQPD1Pars[1];
       fData[ixmyp] *= fQPD1Pars[2];
       fData[ixmym] *= fQPD1Pars[3];
     }
    ix  = key + 4;
    iy  = key + 5;
    sum = fData[ixpyp] + fData[ixpym] + fData[ixmyp] + fData[ixmym];
    fData[key + 6] = sum;
    if (devices.IsUsed(key)) devices.SetUsed(key+6);
    xval = 0;
    yval = 0;
    if ( sum != 0 ) { 
      if (i==0) {
	xval = fQPD1Pars[4]*
	  (fData[ixpyp] + fData[ixpym] - fData[ixmyp]- fData[ixmym])/ sum;
	yval = fQPD1Pars[5]*
	  (fData[ixpyp] - fData[ixpym] + fData[ixmyp]- fData[ixmym])/ sum;
      } else {
	xval = 
	  (fData[ixpyp] + fData[ixpym] - fData[ixmyp]- fData[ixmym])/ sum;
	yval = 
	  (fData[ixpyp] - fData[ixpym] + fData[ixmyp]- fData[ixmym])/ sum;
      }
    }
    if (devices.IsRotated(ix)) {
       fData[ix] = Rotate (xval, yval, 1);
    } else {
       fData[ix] = xval;
    }
    if (devices.IsUsed(key)) devices.SetUsed(ix);
    if (devices.IsRotated(iy)) {
       fData[iy] = Rotate (xval, yval, 2);
    } else {
       fData[iy] = yval;
    }
    if (devices.IsUsed(key)) devices.SetUsed(iy);
  }

// Stripline BPMs
  for (i = 0; i < STRNUM; i++) {
    key = STROFF + 9*i;
    ixp = devices.GetCalIndex(key);
    ixm = devices.GetCalIndex(key+1);
    iyp = devices.GetCalIndex(key+2);
    iym = devices.GetCalIndex(key+3);
    if (ixp < 0 || ixm < 0 || iyp < 0 || iym < 0) continue;
    ix  = key + 4;
    iy  = key + 5;
    sum = fData[ixp] + fData[ixm];
    fData[key + 6] = sum;
    if (devices.IsUsed(key)) devices.SetUsed(key+6);
    xval = 0;
    if ( sum > 0 ) xval = 
          fgKappa * (fData[ixp] - fData[ixm])/ sum;
    sum = fData[iyp] + fData[iym]; 
    fData[key + 7] = sum;
    if (devices.IsUsed(key)) devices.SetUsed(key+7);
    yval = 0;
    if ( sum > 0 ) yval = 
          fgKappa * (fData[iyp] - fData[iym])/ sum;
    if (devices.IsRotated(ix)) {
       fData[ix] = Rotate (xval, yval, 1);
    } else {
       fData[ix] = xval;
    }
    if (devices.IsUsed(key)) devices.SetUsed(ix);
    if (devices.IsRotated(iy)) {
       fData[iy] = Rotate (xval, yval, 2);
    } else {
       fData[iy] = yval;
    }
    if (devices.IsUsed(key)) devices.SetUsed(iy);
    fData[key + 8] = fData[key + 6] + fData[key + 7];
    if (devices.IsUsed(key)) devices.SetUsed(key+8);
  }

// Cavity BPM monitors (when they exist)
  for (i = 0; i < CAVNUM; i++) {
    for (j = 0; j < 2; j++) {
       key = CAVOFF + 4*i + j;
       if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
       idx = devices.GetCalIndex(key);
       if (idx < 0) continue;
       fData[key+2] = fData[idx];
       if (devices.IsUsed(key)) devices.SetUsed(key+2);
// This needs to be divided by current... when they exist.
    }
  }
// Happex-1 era BCMs
  for (i = 0; i < BCMNUM; i++) {
    key = BCMOFF + 2*i;
    if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    idx = devices.GetCalIndex(key);
    if (idx < 0) continue;
    fData[key+1] = fData[idx];
    if (devices.IsUsed(key)) devices.SetUsed(key+1);
  }

// Lumi monitors
  for (i = 0; i < LMINUM; i++) {
    key = LMIOFF + 2*i;
    if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
// corrected LUMI data
    idx = devices.GetCalIndex(key);
    if (idx < 0) continue;
    fData[key+1] = fData[idx];
    if (devices.IsUsed(key)) devices.SetUsed(key+1);
  }

// Detectors
  for (i = 0; i < DETNUM; i++) {
    key = DETOFF + 2*i;
    if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    idx = devices.GetCalIndex(key);
    if (idx < 0) continue;
    fData[key+1] = fData[idx];
    if (devices.IsUsed(key)) devices.SetUsed(key+1);
  }

  // Helicity signals
  for (i = 0; i < TIRNUM; i++) {
    key = TIROFF + i;
    if (devices.GetDevNum(key) < 0 || devices.GetChanNum(key) < 0) continue;
    // Quadsynch
    fData[QUDOFF + i] = (Double_t)(((int)GetData(key) & 0x20) >> 5);
    // Helicity
    fData[HELOFF + i] = (Double_t)(((int)GetData(key) & 0x40) >> 6);
    // Pairsynch
    fData[PAROFF + i] = (Double_t)(((int)GetData(key) & 0x80) >> 7);
    if (devices.IsUsed(key)) {
      devices.SetUsed(QUDOFF + i);
      devices.SetUsed(HELOFF + i);
      devices.SetUsed(PAROFF + i);
    }
  }

  // Old scheme for getting helicity.  Save it just in case I messed up.
  //   fData[IQUADSYNCH] = (Double_t)(((int)GetData(ITIRDATA) & 0x20) >> 5);
  //   fData[IHELICITY] = (Double_t)(((int)GetData(ITIRDATA) & 0x40) >> 6);
  //   fData[IPAIRSYNCH] = (Double_t)(((int)GetData(ITIRDATA) & 0x80) >> 7);

//  // Correction for inverted Pairsynch polarity
//    Int_t fakeps = (((int)GetData(ITIRDATA) & 0x80) >> 7);
//    if(fakeps==1) {
//      fakeps = 0; 
//    }else if (fakeps==0) {
//      fakeps = 1;
//    }
//    fData[IPAIRSYNCH] = (Double_t) fakeps;

  fData[ITIMESLOT] = (Double_t)(((int)GetData(IOVERSAMPLE) & 0xff00) >> 8);
  // kluge to correct new timing boards, with os counting 1,2...(n-1),0
  if (fData[ITIMESLOT]==0) fData[ITIMESLOT] = fgOversample;


//    clog << "VaEvent::Load hel/ps/qs/ts: " 
//         << " " << fData[IHELICITY]
//         << " " << fData[IPAIRSYNCH]
//         << " " << fData[IQUADSYNCH]
//         << " " << fData[ITIMESLOT] << endl;

  // Old scheme for getting helicity.  Save it just in case I messed up.
  // Remember to set the "used" flag (to use for root output).
  //   if (devices.IsUsed(ITIRDATA)) {  
  //      devices.SetUsed(IHELICITY);
  //      devices.SetUsed(IPAIRSYNCH); 
  //      devices.SetUsed(IQUADSYNCH); 
  //   }
  if (devices.IsUsed(IOVERSAMPLE)) devices.SetUsed(ITIMESLOT);

};

void
VaEvent::CheckEvent(TaRun& run)
{
  // Analysis-independent checks of event quality; updates event's cut array.

  const Int_t WRONGEVNO  = 0x1;
  const Int_t WRONGTMSL  = 0x2;

  Double_t current = GetData(fgCurMon);

  Int_t val = 0;

  fFailedACut = false;
  if ( fgLoBeamNo < fgNCuts && current < fgLoBeam )
    {
#ifdef NOISY
      clog << "Event " << fEvNum << " failed lobeam cut, "
	   << current << " < " << fgLoBeam << endl;
#endif
      val = 1;
    }
  AddCut (fgLoBeamNo, val);
  run.UpdateCutList (fgLoBeamNo, val, fEvNum);

  if ( fgLastEv.GetEvNumber() == 0 )
    {
      // First event, cut it (startup cut)

      val = 1;
    }
  else
    {
      // Not the first event, so check event-to-event differences
      
      // Beam burp -- current change greater than limit?
      val = 0;
      if (fgBurpNo < fgNCuts &&
	  abs (current-fgLastEv.GetData(fgCurMon)) > fgBurpCut)
	{
#ifdef NOISY
	  clog << "Event " << fEvNum << " failed beam burp cut, "
	       << abs (current-fgLastEv.GetData(fgCurMon)) << " > " << fgBurpCut << endl;
#endif
	  val = 1;
	}
      AddCut (fgBurpNo, val);
      run.UpdateCutList (fgBurpNo, val, fEvNum);

      // Check event number sequence
      val = 0;
      if (fgEvtSeqNo < fgNCuts &&
	  GetEvNumber() != fgLastEv.GetEvNumber() + 1) 
	{
	  cerr << "VaEvent::CheckEvent ERROR Event " 
	       << GetEvNumber() 
	       << " unexpected event number, last event was " 
	       << fgLastEv.GetEvNumber()
	       << endl;
	  val = WRONGEVNO;
	} 
      AddCut (fgEvtSeqNo, val);
      run.UpdateCutList (fgEvtSeqNo, val, fEvNum);

      // Check time slot sequence
      val = 0;
      if ( fgEvtSeqNo < fgNCuts &&
	  fgOversample > 0 )
	{ 
	  if ( GetTimeSlot() != 
	       fgLastEv.GetTimeSlot() % fgOversample + 1 ) 
	    {
	      cerr << "VaEvent::CheckEvent ERROR Event " 
		   << GetEvNumber() 
		   << " unexpected timeslot value, went from " 
		   << fgLastEv.GetTimeSlot()
		   << " to " 
		   << GetTimeSlot() 
		   << endl;
	      val = WRONGTMSL;
	    } 
	}
      AddCut (fgEvtSeqNo, val);
      run.UpdateCutList (fgEvtSeqNo, val, fEvNum);
      
      // Clear the startup cut
      val = 0;
    }
  
  AddCut (fgStartupNo, val);
  run.UpdateCutList (fgStartupNo, val, fEvNum);

  fgLastEv = *this;
};


Int_t VaEvent::DecodeCrates(TaDevice& devices) {
// Decoding of crate structure.  
// A crate is also called a "ROC" in CODA parlance.
// return code:
//    0 = no decoding done (error, or wrong event type)
//    1 = fine.
  int iroc, lentot, n1, numroc, ipt, istart, istop;
// Cannot decode non-physics triggers  
  if(fEvType < 0 || fEvType > 12) return 0; 
  if(DECODE_DEBUG) RawDump();
  memset (fN1roc, 0, MAXROC*sizeof(Int_t));
  memset (fLenroc, 0, MAXROC*sizeof(Int_t));
  memset (fIrn, 0, MAXROC*sizeof(Int_t));
  numroc = 0;
  lentot = 0;
// Split up the ROCs 
  for (iroc=0; iroc < MAXROC; iroc++) {
// n1 = pointer to first word of ROC
    if(iroc==0) {
       n1 = fEvBuffer[2]+3;
    } else {
       n1 = fN1roc[fIrn[iroc-1]]+fLenroc[fIrn[iroc-1]]+1;
    }
    if( (unsigned int)(n1+1) >= fEvLen ) break;
// fIrn = ROC number 
    fIrn[iroc]=(fEvBuffer[n1+1]&0xff0000)>>16;
// This error might mean you need a bigger MAXROC parameter,
// or it might mean corruption of the raw data.
    if(fIrn[iroc]<0 || fIrn[iroc]>=MAXROC) {
       cout << "ERROR in VaEvent::DecodeCrates():";
       cout << "  illegal ROC number " <<dec<<fIrn[iroc]<<endl;
       return 0;
    }
    if (iroc == 0) {
       fN1roc[fIrn[iroc]] = n1;
       fLenroc[fIrn[iroc]] = fEvBuffer[n1];
       lentot = n1 + fEvBuffer[n1];
    } else {
       fN1roc[fIrn[iroc]]=
          fN1roc[fIrn[iroc-1]]+fLenroc[fIrn[iroc-1]]+1;
       fLenroc[fIrn[iroc]] = fEvBuffer[fN1roc[fIrn[iroc]]];
       lentot  = lentot + fLenroc[fIrn[iroc]] + 1;
    } 
    numroc++;
    if(DECODE_DEBUG) {
        cout << "Roc ptr " <<dec<<numroc<<"  "<<iroc+1;
        cout << "  "<<fIrn[iroc]<<"  "<<fN1roc[fIrn[iroc]];
        cout << "  "<<fLenroc[fIrn[iroc]];
        cout << "  "<<lentot<<"  "<<fEvLen<<endl;
    }
    if ((unsigned int)lentot >= fEvLen) break;
  }
// Find device headers in each ROC
  for (iroc=0; iroc < numroc; iroc++) {
   istart = fN1roc[fIrn[iroc]]+1;
   istop = fN1roc[fIrn[iroc]]+fLenroc[fIrn[iroc]];
   ipt = istart; 
   while (ipt++ < istop) {
       devices.FindHeaders(fIrn[iroc], ipt, fEvBuffer[ipt]);
   }
  }
  if (DECODE_DEBUG) devices.PrintHeaders();
  return 1;
}

void VaEvent::AddCut (const Cut_t cut, const Int_t val)
{
  // Store information about cut conditions passed or failed by this event.

  if (fCutArray != 0)
    fCutArray[(unsigned int) cut] = val;
  if (val != 0)
    fFailedACut = true;
};


void VaEvent::AddResult( const TaLabelledQuantity& result)
{
  // Store a result from analysis of this event.

  fResults.push_back (result);
};


// Data access functions

Int_t VaEvent::GetRawData(Int_t index) const {
  // Return an item from the event buffer.

  if (index >= 0 && (UInt_t)index < fgMaxEvLen) 
    return fEvBuffer[index];
  else
    {
      cerr << "VaEvent::GetRawData ERROR: index " << index 
	   << "out of range 0 to " << fgMaxEvLen;
      return 0;
    }
};

Bool_t VaEvent::CutStatus() const {
  // Return true iff event failed one or more cut conditions 

  return (fFailedACut);
};

Bool_t 
VaEvent::BeamCut() const
{
  // Return true iff event failed low beam cut

  return (CutCond(fgLoBeamNo) != 0);
}

UInt_t 
VaEvent::GetNCuts () const
{
  // Return size of cut array

  return fgNCuts;
}

Int_t 
VaEvent::CutCond (const Cut_t c) const
{
  // Return value of cut condition c

  return (c < fgNCuts ? fCutArray[(unsigned int) c] : 0);
}

Bool_t VaEvent::IsPrestartEvent() const {
  return (fEvType == 17);
};

Bool_t VaEvent::IsPhysicsEvent() const {
  return (fEvType >= 1 && fEvType < 12);
};

EventNumber_t VaEvent::GetEvNumber() const {
  return fEvNum;
};

UInt_t VaEvent::GetEvLength() const {
  return fEvLen;
};

UInt_t VaEvent::GetEvType() const {
  return fEvType;
};

SlotNumber_t VaEvent::GetTimeSlot() const {
  // Return oversampling timeslot for this event.

  return (SlotNumber_t)GetData(ITIMESLOT);
};

void VaEvent::SetHelicity(EHelicity h)
{
  // Fill in the true helicity value for an event.  We use this to
  // associated a delayed helicity with the earlier event it applies to.

  fHel = h;
}

EHelicity VaEvent::GetROHelicity() const 
{
  // Return readout helicity as RightHeli or LeftHeli.  (WARNING: This
  // is the helicity stored in the data stream for this event, which
  // in general is *not* the helicity to use in analysis of this
  // event!  See GetHelicity().  Note also that this is the helicity
  // bit from the source and does not reflect half wave plate state,
  // g-2 precession, etc.)

  Double_t val = GetData(IHELICITY);
  if (val == 0)
    return RightHeli;
  else
    return LeftHeli;
}

EHelicity VaEvent::GetHelicity() const 
{
  // Return true helicity as RightHeli or LeftHeli.  (WARNING: This is
  // the helicity to use in analysis of this event, which in general
  // is *not* the helicity stored in the data stream for this event!
  // See GetROHelicity().  Note also that this is the helicity bit
  // from the source and does not reflect half wave plate state, g-2
  // precession, etc.)

  return fHel;
}

void VaEvent::SetPrevROHelicity(EHelicity h)
{
  // Fill in the readout helicity value for the previous event.

  fPrevROHel = h;
}

void VaEvent::SetPrevHelicity(EHelicity h)
{
  // Fill in the true helicity value for the previous event.

  fPrevHel = h;
}

EHelicity VaEvent::GetPrevROHelicity() const 
{
  // Return readout helicity of previous event as RightHeli or LeftHeli.

  return fPrevROHel;
}

EHelicity VaEvent::GetPrevHelicity() const {
  // Return true helicity of previous event as RightHeli or LeftHeli.

  return fPrevHel;
}

EPairSynch VaEvent::GetPairSynch() const {
  // Return pairsynch (aka realtime) for this event as FirstPS or
  // SecondPS, tagging this as an event from the first or second
  // window, repectively, of a helicity window pair.

  Double_t val = GetData(IPAIRSYNCH);
  if (val == 1) 
    return FirstPS;
  else 
    return SecondPS;
};

EQuadSynch VaEvent::GetQuadSynch() const {
  // Return quadsynch for this event as FirstQS or
  // OtherQS, tagging this as an event from the first or later
  // window, repectively, of a helicity window quad.

  Double_t val = GetData(IQUADSYNCH);
  if (val == 1) 
    return FirstQS;
  else 
    return OtherQS;
};

const vector < TaLabelledQuantity > & VaEvent::GetResults() const 
{ 
  // Return event analysis results stored in this event.

  return fResults; 
};

void VaEvent::RawDump() const {
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

void VaEvent::DeviceDump() const {
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
  cout << "   readout helicity = "<<dec<<(Int_t)GetData(IHELICITY);
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

void 
VaEvent::MiniDump() const 
{
// Diagnostic dump of selected data on one line for debugging purposes.

  cout << "Event " << dec << setw(7) << GetEvNumber()
       << " h(ro)/h/t/p/q =" << dec 
       << " " << setw(2) << (Int_t) GetData (IHELICITY)
       << "/" << setw(2) << (Int_t) GetHelicity()
       << "/" << setw(2) << (Int_t) GetData (ITIMESLOT)
       << "/" << setw(2) << (Int_t) GetData (IPAIRSYNCH)
       << "/" << setw(2) << (Int_t) GetData (IQUADSYNCH)
       << " Cuts = " << setw(2) << CutCond(0)
       << setw(2) << CutCond(1)
       << setw(2) << CutCond(2)
       << setw(2) << CutCond(3)
       << setw(2) << CutCond(4)
       << " BCM = " << GetData (fgCurMon)
       << endl; 
}

Double_t VaEvent::GetData( Int_t key ) const { 
// To find a value corresponding to a data key 
  return fData[Idx(key)];
}; 

Double_t 
VaEvent::GetDataSum (vector<Int_t> keys, vector<Double_t> wts) const
{
  // Get weighted sum of quantities corresponding to set of keys

  Double_t sum = 0;

  if (wts.size() == 0)
    for (vector<Int_t>::const_iterator p = keys.begin();
	 p != keys.end();
	 ++p)
      sum += fData[Idx(*p)];
  else if (wts.size() != keys.size())
    cerr << "VaEvent::GetDataSum ERROR: Weight and key vector sizes differ" << endl;
  else
    for (size_t i = 0; i < keys.size(); ++i)
      sum += wts[i] * fData[Idx(keys[i])];

  return sum;
}

Int_t VaEvent::Idx(const Int_t& index) const {
  if (index >= 0 && index < MAXKEYS) return index;
  return 0;
};

Double_t VaEvent::GetRawADCData( Int_t adc, Int_t chan ) const {
// Data raw data for adc # 0, 1, 2...  and channel # 0,1,2,3
  return GetData(ADCOFF + 4*adc + chan);
}; 

Double_t VaEvent::GetCalADCData( Int_t adc, Int_t chan ) const {
// Data calibrated data for adc # 0, 1, 2...and channel # 0,1,2,3
  return GetData(ACCOFF + 4*adc + chan);
}; 

Double_t VaEvent::GetScalerData( Int_t scaler, Int_t chan ) const {
// Data from scaler # 1,2,3..  and channel # 1,2,3...
  return GetData(SCAOFF + 32*scaler + chan);
}; 

void 
VaEvent::AddToTree (TaDevice& devices, 
		    const TaCutList& cutlist,
		    TTree& rawtree ) 
{
  // Add the data of this device to the raw data tree (root output)
  // Called by TaRun::InitDevices()
    DecodeCook(devices);
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
          cerr << "VaEvent::AddToTree::ERROR:  Attempt to add a key = "<<key;
          cerr << " larger than array size MAXKEYS = "<<MAXKEYS<<endl;
          cerr << "Compile with a bigger MAXKEYS value."<<endl;
          continue;
        }
// Dont add to tree unless in datamap
        if ( !devices.IsUsed(key) ) continue;  
        strcpy(tinfo,keystr.c_str());  strcat(tinfo,"/D");
  	rawtree.Branch(keystr.c_str(), &fData[key], tinfo, bufsize);
    }


    for (Cut_t icut = Cut_t (0); icut < cutlist.GetNumCuts(); ++icut)
      {
	TaString cutstr = "cond_" + cutlist.GetName(icut);
	cutstr = cutstr.ToLower();
        strcpy (tinfo, cutstr.c_str());  strcat(tinfo,"/I");
  	rawtree.Branch(cutstr.c_str(), 
		       &fCutArray[(unsigned int) icut], 
		       tinfo, 
		       bufsize);
      }	
};

// Private methods

Double_t VaEvent::Rotate(Double_t x, Double_t y, Int_t xy) {
// Rotation to get X or Y depending on xy flag
   Double_t result = 0;
   Double_t root2 = sqrt(2.0);
   if (xy == 2) {
      result = ( x + y ) / root2;
   } else {
      result = ( x - y ) / root2;
   } 
   return result;
};

void VaEvent::Create(const VaEvent& rhs)
{
  // Utility routine used by copy constructor and assignment.

 fEvType = rhs.fEvType;
 fEvNum = rhs.fEvNum;
 fEvLen = rhs.fEvLen;
 fFailedACut = rhs.fFailedACut;
 fResults = rhs.fResults;
 fHel = rhs.fHel;
 fPrevROHel = rhs.fPrevROHel;
 fPrevHel = rhs.fPrevHel;
 fEvBuffer = new Int_t[fgMaxEvLen];
 memset (fEvBuffer, 0, fgMaxEvLen*sizeof(Int_t));
 memcpy(fEvBuffer, rhs.fEvBuffer, fEvLen*sizeof(Int_t));
 fData = new Double_t[MAXKEYS];
 memcpy(fData, rhs.fData, MAXKEYS*sizeof(Double_t));
 // We don't need to copy these pointers because they get
 // filled and used only in decoding.
 fN1roc = new Int_t[MAXROC];
 fIrn = new Int_t[MAXROC];
 fLenroc = new Int_t[MAXROC];
 if (rhs.fCutArray != 0 && fgNCuts > 0)
   {
     fCutArray = new Int_t[fgNCuts];
     memcpy(fCutArray, rhs.fCutArray, fgNCuts*sizeof(Int_t));
   }
 else
   fCutArray = 0;
};

void VaEvent::Uncreate()
{
  // Utility routine used by destructor and assignment.

  delete [] fEvBuffer;
  delete [] fData;
  delete [] fN1roc;
  delete [] fLenroc;
  delete [] fIrn;
  if (fCutArray != 0)
    delete [] fCutArray;
};
