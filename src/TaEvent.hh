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

class TaDevice;
class TTree;
class TaCutList;
class TaLabelledQuantity;
class TaRun;
class VaDataBase;

class TaEvent {

public:

  // Constructors/destructors/operators
  TaEvent();
  virtual ~TaEvent();
  TaEvent(const TaEvent &ev);
  TaEvent& operator=(const TaEvent &ev);

  // Major functions
  static void RunInit(const TaRun& run);    // initialization at start of run
  void Load ( const Int_t *buffer );
  void Decode( const TaDevice& devices );             // decode the event 
  void CheckEvent(TaRun& run);
  void AddCut (const ECutType, const Int_t); // store cut conditions
  void AddResult( const TaLabelledQuantity& result);

 // Data access functions
  static Int_t BuffSize() { return fgMaxEvLen; };
  static Int_t GetMaxEvNumber() { return fgMaxEvNum; }; // Maximum event number
  Int_t GetRawData(Int_t index) const; // raw data, index = location in buffer 
  Double_t GetData(Int_t key) const;   // get data by unique key
  Double_t GetRawADCData(Int_t slot, Int_t chan) const;  // get raw ADC data in slot and chan.
  Double_t GetCalADCData(Int_t slot, Int_t chan) const;  // get calib. ADC data in slot and chan.
  Double_t GetScalerData(Int_t slot, Int_t chan) const;  // get scaler data in slot and chan.

  Bool_t CutStatus() const;          // Return true iff event failed one or more cut conditions 
  Bool_t BeamCut() const;            // Return true iff event failed low beam cut
  Bool_t IsPrestartEvent() const;    // run number available in 'prestart' events.
  Bool_t IsPhysicsEvent() const;
  EventNumber_t GetEvNumber() const; // event number
  UInt_t GetEvLength() const;        // event length
  UInt_t GetEvType() const;          // event type
  SlotNumber_t GetTimeSlot() const;  // time slot
  void SetDelHelicity (EHelicity);   // set delayed helicity
  EHelicity GetHelicity() const;     // (in time) helicity
  EHelicity GetDelHelicity() const;  // delayed helicity
  EPairSynch GetPairSynch() const;   // pair synch
  const vector < TaLabelledQuantity > & GetResults() const; // results for event
  void RawDump() const;      // dump raw data for debugging.
  void DeviceDump() const;   // dump device data for debugging.

  void AddToTree (const TaDevice& dev, 
		  const TaCutList& cutlist, 
		  TTree &tree);    // Add data to root Tree

private:

  // Private methods
  void Create(const TaEvent&);
  void Uncreate();
  Int_t Idx(const Int_t& key) const;
  Double_t Rotate(Double_t x, Double_t y, Int_t xy);

  // Constants
  static const UInt_t fgMaxEvLen = 2000;    // Maximum length for event buffer
  static const EventNumber_t fgMaxEvNum = 10000000;  // Maximum event number
  static const Double_t fgKappa = 18.76;   // stripline BPM calibration

  // Static members
  static TaEvent fgLastEv;     // copy of previous event
  static Bool_t fgFirstDecode; // true until first event decoded
  static Double_t fgLoBeam;    // cut threshold from database
  static Double_t fgBurpCut;   // cut threshold from database
  static UInt_t fgSizeConst;   // size of first physics event should be size of all

  // Data members
  Int_t *fEvBuffer;            // Raw event data
  UInt_t fEvType;              // Event type: 17 = prestart, 1-11 = physics
  EventNumber_t fEvNum;        // Event number from data stream
  UInt_t fEvLen;               // Length of event data
  Int_t* fCutArray;            // Array of cut values
  Bool_t fFailedACut;          // True iff a cut failed
  vector<TaLabelledQuantity> fResults;     // Results of event analysis
  EHelicity fDelHel;           // Delayed helicity filled from later event
  Double_t *fData;             // Decoded/corrected data

#ifdef DICT
ClassDef(TaEvent,0)  // An event
#endif

};

#endif
