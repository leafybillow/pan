#ifndef PAN_TaRun
#define PAN_TaRun

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRun.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// This class treats the data of one run. The Init method initializes
// the event TTree, attaches the Coda file or online data, and gets
// the (ASCII or MySQL) database.  It initializes the storage of
// devices and cuts.
//
// In the event loop, the NextEvent method is called to get and decode
// an event from the data stream.  AddCuts is called after
// preprocessing each event, to update the list of cut intervals.
// AccumEvent and AccumPair accumulate statistics for results of event
// and pair analysis, respectively, and periodically write statistics
// summaries to STDOUT.
//
// When analysis is complete, Finish is called to print final
// statistics summaries.
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "TTree.h"
#include "PanTypes.hh"
#include "DevTypes.hh"
#include <vector>
#include <string>
#include <map>

class THaCodaData;
class TaEvent;
class TaDevice;
class VaPair;
class VaDataBase;
class TaCutList;
class TaStatistics;

class TaRun
{

public:

  // Constructors/destructors/operators
  TaRun();   
  TaRun(const Int_t& run);
  TaRun(const string& filename);
  virtual ~TaRun() { Uncreate(); } 
  // Copy constructor and operator= are defined privately.

  // Major functions
  virtual ErrCode_t Init();
  virtual ErrCode_t ReInit();
  Bool_t NextEvent();
  void Decode();
  virtual void AccumEvent(const TaEvent&);
  virtual void AccumPair(const VaPair&);
  void UpdateCutList(const ECutType, const Int_t, EventNumber_t);
  virtual void Finish();

  // Data access functions
  const TaCutList& GetCutList() const { return *fCutList; }
  SlotNumber_t GetOversample() const { return fOversamp; };
  UInt_t GetRate() const { return 30 * fOversamp; }; // events per second
  Double_t GetDBValue(string key) const;
  TaEvent& GetEvent() const { return *fEvent; };
  Int_t GetRunNumber() const { return fRunNumber; };
  VaDataBase& GetDataBase() const { return *fDataBase; };
  Int_t GetKey(string keystr) const;
  string GetKey(Int_t key) const;

  // Static data

  static const ErrCode_t fgTARUN_ERROR;  // returned on error
  static const ErrCode_t fgTARUN_OK;      // returned on success
  static const ErrCode_t fgTARUN_VERBOSE; // verbose(1) or not(0) warnings
  static const EventNumber_t fgSLICELENGTH;  // events in a statistics slice

private:

  // Copy constructor and operator= -- defined null and private
  TaRun(const TaRun& run);
  TaRun& operator=(const TaRun& run);

  // Member functions
  virtual void Uncreate();
  Int_t GetBuffer();
  Int_t FindRunNumber();  
  virtual void InitDevices();
  void PrintStats (const TaStatistics& s, const vector<string>& n, const vector<string>& u) const;

  // Data members
  RunNumber_t fRunNumber;        // Number of this run
  Int_t fEventNumber;            // Number of the recently read event
  VaDataBase* fDataBase;         // Database for this run
  TaCutList* fCutList;           // Cut list for this run
  SlotNumber_t fOversamp;        // Oversample value for this run
  THaCodaData* fCoda;            // CODA data source
  string fCodaFileName;          // Name of CODA data file
  string fComputer;              // Computer to ask for online data
  string fSession;               // CODA session for online data
  TaEvent* fEvent;               // The most recently read event
  TaDevice* fDevices;            // Device map for this run
  TTree *fEvtree;                // Event tree for Root file
  TaStatistics* fESliceStats;    // Incremental event statistics
  TaStatistics* fPSliceStats;    // Incremental pair statistics
  TaStatistics* fERunStats;      // Cumulative event statistics
  TaStatistics* fPRunStats;      // Cumulative pair statistics
  vector<string> fEStatsNames;   // Names of event statistics
  vector<string> fPStatsNames;   // Names of pair statistics
  vector<string> fEStatsUnits;   // Units of event statistics
  vector<string> fPStatsUnits;   // Units of pair statistics
  EventNumber_t fSliceLimit;     // Event number at end of next slice
  Bool_t fFirstPass;             // Pass 1 or 2?

#ifdef DICT
ClassDef (TaRun, 0)      //  One run of CODA data
#endif

};

#endif
