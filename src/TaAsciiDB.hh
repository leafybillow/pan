#ifndef PAN_TaAsciiDB
#define PAN_TaAsciiDB

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaAsciiDB.hh   (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//
//    Ascii based database, also used to read the 'control' file, 
//    and appended database with output.
//    Inherits interface from class VaDataBase.
//
//////////////////////////////////////////////////////////////////////////


#include "Rtypes.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdlib.h>

#include "VaDataBase.hh"

class TaAsciiDB : public VaDataBase {

public:
 
  TaAsciiDB();
  virtual ~TaAsciiDB();

  void Load(int run);   // Load the database for this run.
  void Write();         // Write (update) database for this run.

// Generic get methods if you know the 'table' and 'key's you want.
  vector<Double_t> GetData(string table, vector<string> key) const;
// Get run type, e.g. runtype = 'beam' (case insensitive)
  string GetRunType()const ;
// Get analysis type
  string GetAnaType()const ;
  // Get feedback switch
  string GetFdbkSwitch( const string &fdbktype) const;
// Get feedback timescale 
  Int_t  GetFdbkTimeScale(const string &fdbktype) const;  
// FIXME: I drop a lot of 'const' everywhere... no time to put it in.
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  Double_t GetDacNoise(const Int_t& adc, const Int_t& chan, const string& key) const;  
// Get Pedestals for adc, chan
  Double_t GetAdcPed(const Int_t& adc, const Int_t& chan) const;
// Get Pedestals for scaler, chan
  Double_t GetScalPed(const Int_t& adc, const Int_t& chan) const;
// Get Headers for decoding (needed by datamap)
  UInt_t GetHeader(const string& device) const ;
// Get Masks for decoding (needed by datamap)
  UInt_t GetMask(const string& device) const ;
// Get datamap info 
  void DataMapReStart();  // reset the iterator (suggest you do it before using NextDataMap)
  Bool_t NextDataMap();
  string GetDataMapName() const;
  string GetDataMapType() const;
  TaKeyMap GetKeyMap(string device) const;
// Get a value from 'table', assumed a single value
  Double_t GetValue(const string& table) const;
// Get a vector of integer values from 'table'
  vector<Int_t> GetValueVector(const string& table) const;
// Get string from 'table'
  string GetString(const string& table) const;
// Get Maximum number of events to process
  Int_t GetMaxEvents() const;
// Get Window Delay
  Int_t GetDelay() const;
// Get oversample factor
  Int_t GetOverSamp() const;
// Get pair type (pair or quad) for this run
  string GetPairType() const;  // returns 'pair' or 'quad', etc.
// Get a cut value for 'cutname'.  e.g. cutname = 'lowbeam', 'burpcut' 
  Double_t GetCutValue(const string& cutname) const;
// Get number of cuts 
  Int_t GetNumCuts() const;
// Get cut extensions, low and high 
  vector<Int_t> GetExtLo() const;
  vector<Int_t> GetExtHi() const;
// Get number of bad event intervals
  Int_t GetNumBadEv() const;
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  map <Int_t, vector<Int_t> > GetCutValues() const;
// Generic Get method, works if the key is unique
  virtual Double_t GetData(const string& key) const ;
// Self check
  Bool_t SelfCheck();

// Updating the database
  void ClearAll();                 // clears all (for this run only)
  void ClearAll(Int_t run);        // clears all (for this run only)
  void ClearTable(string table);   // wipe clean a table for this run
  void ClearTable(Int_t run, string table);  // wipe clean a table for this run
// Generic get method if you know the 'table' and 'key's you want.
  void PutData(string table, const vector<string>& key, 
         const vector<Double_t>& data);
// Put run type, e.g. runtype = 'beam' (case insensitive)
  void PutRunType(string);
// Put analysis type
  void PutAnaType(string);
// Put Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  void PutDacNoise(Int_t adc, Int_t chan, string key, Double_t value);  
// Put Pedestals for adc, chan
  void PutPedestal(Int_t adc, Int_t chan, Double_t value);
// Put Headers for decoding (needed by datamap)
  void PutHeader(string device, Int_t header);
// Put datamap info 
  void PutDataMap(string table, vector<string> keys, vector<Int_t>& map);
// Put a cut value for 'cutname'.  e.g. cutname = 'lowbeam', 'burpcut' 
  void PutCutValue(string cutname, Double_t value);
// Put number of cuts 
  void PutNumCuts(Int_t numcuts);
// Put cut extensions, low and high 
  void PutExts(const vector<Int_t>& extlo, 
                       const vector<Int_t>& exthi);
// Put number of bad event intervals
  void PutNumBadEv(Int_t num_intervals);
// For bad event intervals, get formatted results 
// First element of map goes from 0 to PutNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  void PutCutValues(const map <Int_t, vector<Int_t> > & cuts);

  
private:

  TaAsciiDB(const TaAsciiDB &fn);
  TaAsciiDB& operator=(const TaAsciiDB &fn);

// Probably most of the following methods and data need to be part 
// of the parent class VaDataBase, but I put it here for now.
  ifstream *dbfile;
  void InitDB();
  void LoadTable(string table, vector<dtype*>);
  void PrintDataBase();
  void PrintDataMap();
  string FindTable(string table);
  Double_t GetData(dtype* dat) const;
  string GetData(string table, string key, Int_t index) const;
  void InitDataMap();
  vector <string> tables;
  multimap<string, vector<dtype*> > database;
  map<string, TaKeyMap > datamap; 
  map<string, TaKeyMap >::const_iterator cdmapiter;
  map<string, TaKeyMap >::iterator dmapiter;
  map<string, int> dbinit,colsize;
  Bool_t didinit,initdm,firstiter;
  Double_t *dacparam,*adcped,*scalped;
  Bool_t *fFirstgdn;
  Bool_t *fFirstAdcPed, *fFirstScalPed;

#ifdef DICT
ClassDef(TaAsciiDB,0)  // Ascii database or control file
#endif

};

#endif
