#ifndef PAN_TaEvent
#define PAN_TaEvent

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaEvent.hh  (header file)
//           ^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    An event of data.
//    Includes methods to get data using keys.  For ADCs and 
//    scalers can also get the data by slot number and channel.
//
//////////////////////////////////////////////////////////////////////////

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
  void Load ( const Int_t *buffer );
  void Decode( const TaDevice& devices );             // decode the event 
  const vector<pair<ECutType,Int_t> >& CheckEvent(const TaRun& run);
  void AddCut (const ECutType cut, const Int_t val);
  void AddResult( const TaLabelledQuantity& result);

 // Data access functions
  static Int_t BuffSize() { return fgMaxEvLen; };
  static Int_t GetMaxEvNumber() { return fgMaxEvNum; }; // Maximum event number
  Int_t GetRawData(Int_t index) const; // raw data, index = location in buffer 
  Double_t GetData(Int_t key) const;   // get data by unique key
  Double_t GetRawADCData(Int_t slot, Int_t chan) const;  // get raw ADC data in slot and chan.
  Double_t GetCalADCData(Int_t slot, Int_t chan) const;  // get calib. ADC data in slot and chan.
  Double_t GetScalerData(Int_t slot, Int_t chan) const;  // get scaler data in slot and chan.

  Bool_t CutStatus() const;
  Bool_t IsPrestartEvent() const;   // run number available in 'prestart' events.
  Bool_t IsPhysicsEvent() const;
  EventNumber_t GetEvNumber() const;  // event number
  UInt_t GetEvLength() const;        // event length
  UInt_t GetEvType() const;          // event type
  SlotNumber_t GetTimeSlot() const;  // time slot
  void SetDelHelicity (EHelicity);   // set delayed helicity
  EHelicity GetHelicity() const;     // (in time) helicity
  EHelicity GetDelHelicity() const;  // delayed helicity
  EPairSynch GetPairSynch() const;   // pair synch
  const vector < TaLabelledQuantity > & GetResults() const; // results for event
  const vector <pair<ECutType,Int_t> > & GetCuts() const; // cut conditions failed by event
  const vector <pair<ECutType,Int_t> > & GetCutsPassed() const; // cut conditions passed by event
  void RawDump() const;      // dump raw data for debugging.
  void DeviceDump() const;   // dump device data for debugging.

  void AddToTree(const TaDevice& dev, TTree &tree);    // Add data to root Tree

private:

  Bool_t fInited;
  Int_t fNumRaw;
  Double_t *fData;

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
  static TaEvent fgLastEv;  // last ev
  static Bool_t fFirstDecode;

  // Data members
  Int_t *fEvBuffer;
  UInt_t fEvType;
  EventNumber_t fEvNum;
  UInt_t fEvLen,fSizeConst;
  Bool_t fFirstCheck;
  Double_t fLoBeam, fBurpCut;
  vector<pair<ECutType,Int_t> > fCutFail;
  vector<pair<ECutType,Int_t> > fCutPass;
  vector<TaLabelledQuantity> fResults;
  EHelicity fDelHel; // Delayed helicity

#ifdef DICT
ClassDef(TaEvent,0)  // An event
#endif

};

#endif
