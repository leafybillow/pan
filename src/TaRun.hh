#ifndef PAN_TaRun
#define PAN_TaRun

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRun.hh  (header file)
//           ^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    A run of data (where 'run' is defined by CODA).  A run is 
//    typically a 1 hour period where the setup parameters are fixed.
//
//////////////////////////////////////////////////////////////////////////


#include "Rtypes.h"
#include "TTree.h"
#include "PanTypes.hh"
#include <vector>
#include <string>
#include <map>

class THaCodaData;
class TaEvent;
class VaPair;
class VaDataBase;
class TaCutList;
class VaDevice;
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
  virtual void Init();
  Bool_t NextEvent();
  void Decode();
  virtual void AccumEvent(const TaEvent&);
  virtual void AccumPair(const VaPair&);
  void AddCutToEvent(const ECutType, const Int_t val);
  void AddCuts();
  virtual void Finish();

  // Data access functions
  const TaCutList& GetCutList() const { return *fCutList; }
  SlotNumber_t GetOversample() const { return fOversamp; };
  UInt_t GetRate() const { return 30 * fOversamp; }; // events per second
  Double_t GetDBValue(string key) const;
  TaEvent& GetEvent() const { return *fEvent; };
  Int_t GetRunNumber() const { return fRunNumber; };
  VaDataBase* GetDataBase() const { return fDataBase; };
  void SendEPICSInfo( pair< char*, Double_t> value); // used by feedbacks 
                                                        // to send EPICS var in ONLINE mode.
private:

  // Copy constructor and operator= -- defined null and private
  TaRun(const TaRun& run);
  TaRun& operator=(const TaRun& run);

  // Member functions
  virtual void Uncreate();
  Int_t GetBuffer();
  Int_t FindRunNumber();  
  virtual void InitDevices();
  void PrintStats (TaStatistics s, vector<string> n, vector<string> u) const;

  // Static data
  // Event types
  static const UInt_t fgPHYSICSMAX  = 12;
  static const UInt_t fgPRESTART = 17;
  // Flags
  static const Int_t TARUN_ERROR = -1;
  static const Int_t TARUN_VERBOSE = 1; // verbose(1) or not(0) warnings
  // Others
  static const EventNumber_t fgSLICELENGTH = 1000;

  // Data members
  RunNumber_t fRunNumber;
  Int_t fEventNumber;
  VaDataBase* fDataBase;
  TaCutList* fCutList;
  map<string, VaDevice* > devices;
  SlotNumber_t fOversamp;
  THaCodaData* fCoda;
  string fCodaFileName,fComputer,fSession;
  TaEvent* fEvent;
  TTree *evtree;
  TaStatistics* fESliceStats;
  TaStatistics* fPSliceStats;
  TaStatistics* fERunStats;
  TaStatistics* fPRunStats;
  vector<string> fEStatsNames;
  vector<string> fPStatsNames;
  vector<string> fEStatsUnits;
  vector<string> fPStatsUnits;
  EventNumber_t fSliceLimit;

#ifdef DICT
ClassDef (TaRun, 0)      //  One run of CODA data
#endif

};

#endif
