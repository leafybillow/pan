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
//    Includes methods to get data using keys.  For ADCs one
//    can also get the data by ADC number and channel.
//    The user (presumaby a TaRun) must LoadDevice() at
//    least once in the life of TaEvent, presumably once
//    per run.
//
//////////////////////////////////////////////////////////////////////////

#define TAEVENT_VERBOSE 1
#include "Rtypes.h"
#include "PanTypes.hh"
#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <utility>

class VaDevice;
class TaLabelledQuantity;
class TaRun;

class TaEvent {

public:

  // Constructors/destructors/operators
  TaEvent();
  virtual ~TaEvent();
  TaEvent(const TaEvent &ev);
  TaEvent& operator=(const TaEvent &ev);

  // Major functions
  void InitDevices( map < string, VaDevice* >& devices );
  void Load ( const Int_t *buffer );
  void Decode();             // decode the event 
  const vector<pair<ECutType,Int_t> >& CheckEvent(const TaRun& run);
  void AddCut (const ECutType cut, const Int_t val);
  void AddResult( const TaLabelledQuantity& result);

 // Data access functions
  static Int_t BuffSize() { return fgMaxEvLen; };
  static Int_t GetMaxEvNumber() { return fgMaxEvNum; }; // Maximum event number
  Int_t GetRawData(Int_t index) const; // raw data, index = location in buffer 
  Double_t GetData(const string& devicename, const Int_t& channel) const;
  Double_t GetData(const string& devicename, const string& key) const;
  Double_t GetData(const string& key) const;
  Double_t GetADCData(const Int_t& slot, const Int_t& chan) const;  // ADC 0,1.. and chan 0,1..

  Bool_t CutStatus() const;
  Bool_t IsPrestartEvent() const;   // run number available in 'prestart' events.
  Bool_t IsPhysicsEvent() const;
  EventNumber_t GetEvNumber() const;        // event number
  UInt_t GetEvLength() const;        // event length
  UInt_t GetEvType() const;          // event type
  SlotNumber_t GetTimeSlot() const;        // time slot
  void SetDelHelicity (EHelicity);   // set delayed helicity
  EHelicity GetHelicity() const;     // (in time) helicity
  EHelicity GetDelHelicity() const;  // delayed helicity
  EPairSynch GetPairSynch() const;   // pair synch
  const vector < TaLabelledQuantity > & GetResults() const; // results for event
  const vector <pair<ECutType,Int_t> > & GetCuts() const; // cut conditions failed by event
  const vector <pair<ECutType,Int_t> > & GetCutsPassed() const; // cut conditions passed by event
  void RawDump() const;      // dump raw data for debugging.
  void DeviceDump() const;   // dump device data for debugging.

private:

  // Private methods
  void Create(const TaEvent&);
  void Uncreate();

  // Constants
  static const UInt_t fgMaxEvLen = 20000;    // Maximum length for event buffer
  static const EventNumber_t fgMaxEvNum = 10000000;  // Maximum event number

  // Static members
  static TaEvent fgLastEv;  // last ev
  static Bool_t fgDidInit,fFirstDecode;

  // Data members
  map < string, VaDevice* > fDevices;
  map<pair<int,int>, pair<string,string> > adcs;
  Int_t *fEvBuffer;
  UInt_t fEvType;
  EventNumber_t fEvNum;
  UInt_t fEvLen,fSizeConst;
  vector<pair<ECutType,Int_t> > fCutFail;
  vector<pair<ECutType,Int_t> > fCutPass;
  vector<TaLabelledQuantity> fResults;
  map <string, string> fKeyDev;
  map <string, int> fKeyUni;
  EHelicity fDelHel; // Delayed helicity

#ifdef DICT
ClassDef(TaEvent,0)  // An event
#endif

};

#endif
