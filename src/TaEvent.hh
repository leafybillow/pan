#ifndef PAN_TaEvent
#define PAN_TaEvent

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaEvent.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
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

#define TAEVENT_VERBOSE 1

#include "Rtypes.h"
#include "PanTypes.hh"
#include "DevTypes.hh"
#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <utility>
//#define FAKEHEL
#ifdef FAKEHEL
#include <iostream>
#include <fstream>
#endif

//#define FAKEDET
#ifdef FAKEDET
class TRandom;
#endif
class TaDevice;
class TTree;
class TaCutList;
class TaLabelledQuantity;
class TaRun;
class TaDataBase;

class TaEvent {

public:

  // Constructors/destructors/operators
  TaEvent();
  virtual ~TaEvent();
  TaEvent(const TaEvent &ev);
  TaEvent& operator=(const TaEvent &ev);
  TaEvent& CopyInPlace (const TaEvent& rhs);

  // Major functions
  static ErrCode_t RunInit(const TaRun& run);    // initialization at start of run
  void Load ( const Int_t *buffer );
  void Decode( TaDevice& devices );             // decode the event 
  void CheckEvent(TaRun& run);
  void AddCut (const Cut_t, const Int_t); // store cut conditions
  void AddResult( const TaLabelledQuantity& result);

 // Data access functions
  static Int_t BuffSize() { return fgMaxEvLen; };
  static Int_t GetMaxEvNumber() { return fgMaxEvNum; }; // Maximum event number
  Int_t GetRawData(Int_t index) const; // raw data, index = location in buffer 
  Double_t GetData(Int_t key) const;   // get data by unique key
  Double_t GetDataSum (vector<Int_t>, vector<Double_t> = vector<Double_t>(0)) const; // get weighted sum of data
  Double_t GetRawADCData(Int_t slot, Int_t chan) const;  // get raw ADC data in slot and chan.
  Double_t GetCalADCData(Int_t slot, Int_t chan) const;  // get calib. ADC data in slot and chan.
  Double_t GetScalerData(Int_t slot, Int_t chan) const;  // get scaler data in slot and chan.

  Bool_t CutStatus() const;          // Return true iff event failed one or more cut conditions 
  Bool_t BeamCut() const;            // Return true iff event failed low beam cut
  UInt_t GetNCuts () const;          // Return size of cut array
  Int_t CutCond (const Cut_t c) const; // Return value of cut condition c
  Bool_t IsPrestartEvent() const;    // run number available in 'prestart' events.
  Bool_t IsPhysicsEvent() const;
  EventNumber_t GetEvNumber() const; // event number
  UInt_t GetEvLength() const;        // event length
  UInt_t GetEvType() const;          // event type
  SlotNumber_t GetTimeSlot() const;  // time slot
  void SetHelicity (EHelicity);      // set true helicity
  EHelicity GetROHelicity() const;   // readout helicity
  EHelicity GetHelicity() const;     // true helicity
  void SetPrevROHelicity(EHelicity h);     // set readout helicity of prev evt
  void SetPrevHelicity(EHelicity h);  // set true helicity of prev evt
  EHelicity GetPrevROHelicity() const;     // get readout helicity of prev evt
  EHelicity GetPrevHelicity() const;  // get true helicity of prev evt

  EPairSynch GetPairSynch() const;   // pair synch
  EQuadSynch GetQuadSynch() const;   // quad synch
  const vector < TaLabelledQuantity > & GetResults() const; // results for event
  void RawDump() const;      // dump raw data for debugging.
  void DeviceDump() const;   // dump device data for debugging.
  void MiniDump() const;     // dump of selected data on one line

  void AddToTree (TaDevice& dev, 
		  const TaCutList& cutlist, 
		  TTree &tree);    // Add data to root Tree

private:

  // Private methods
  void Create(const TaEvent&);
  void Uncreate();
  Int_t Idx(const Int_t& key) const;
  Double_t Rotate(Int_t key, Double_t x, Double_t y, Int_t xy);
  void DecodeCook( TaDevice& devices );   // Called by AddToTree

  // Constants
  static const UInt_t fgMaxEvLen = 2000;    // Maximum length for event buffer
  static const EventNumber_t fgMaxEvNum = 10000000;  // Maximum event number
  static const Double_t fgKappa = 18.76;   // stripline BPM calibration
  static const Cut_t fgMaxCuts = 10;    // Length of cut values array
  static const ErrCode_t fgTAEVT_ERROR;  // returned on error
  static const ErrCode_t fgTAEVT_OK;      // returned on success

  // Static members
  static TaEvent fgLastEv;     // copy of previous event
  static Bool_t fgFirstDecode; // true until first event decoded
  static Double_t fgLoBeam;    // cut threshold from database
  static Double_t fgBurpCut;   // cut threshold from database
  static Cut_t fgLoBeamNo;  // cut number for low beam
  static Cut_t fgBurpNo;    // cut number for beam burp
  static Cut_t fgOversampleNo; // cut number for oversample
  static UInt_t fgOversample;  // oversample factor
  static UInt_t fgSizeConst;   // size of first physics event should be size of all
  static UInt_t fgNCuts;       // Length of cut array
#ifdef FAKEHEL
  static ifstream fgHelfile;   // fake helicity data file
#endif
#ifdef FAKEDET
  static Double_t fgK[4];  // Proportionality constants
  static Double_t fgD[4];  // Noise amplitude
  static TRandom fgR;      // Random number object
  // Fake data = BCM1 * (fgK[i] + 
  //                     fgD[i] * normal noise[i])
#endif

  // Data members
  Int_t *fEvBuffer;            // Raw event data
  UInt_t fEvType;              // Event type: 17 = prestart, 1-11 = physics
  EventNumber_t fEvNum;        // Event number from data stream
  UInt_t fEvLen;               // Length of event data
  Int_t* fCutArray;            // Array of cut values
  Bool_t fFailedACut;          // True iff a cut failed
  vector<TaLabelledQuantity> fResults;     // Results of event analysis
  EHelicity fHel;              // True helicity filled from later event
  EHelicity fPrevROHel;        // Readout helicity for previous event
  EHelicity fPrevHel;          // True helicity for previous event
  Double_t *fData;             // Decoded/corrected data

#ifndef NODICT
ClassDef(TaEvent,0)  // An event
#endif

};

#endif
