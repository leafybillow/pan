#ifndef ROOT_VaDataBase
#define ROOT_VaDataBase

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        VaDataBase.hh   (header file)
//        ^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//  Abstract interface to database.  Classes that inherit from
//  this are TaAsciiDB and TaMySql which read from Ascii files
//  and MySql respectively.
//  
//  The database is organized in tables, below is a list of tables
//  (which may grow over time).  The tables are denoted by a string
//  table name (e.g. 'dacnoise').  In each table is a series of
//  columns of information.  The columns are labelled by 'keys'
//  which are optional.  E.g. key='adc' for table='ped', while for
//  table='lowbeam' there is no key.  The software is case
//  insensitive.
//
//  Tables include:
//
//      1. run  (the CODA run number)
//      2. run type
//      3. analysis type
//      4. maxevents (the number of events to analyze)
//      5. pair type ('pair' or 'quad')
//      6. window delay
//      7. oversampling factor
//      8. dac noise parameters
//      9. pedestals 
//     10. datamap and header info
//     11. named cuts ('lobeam', 'burpcut', etc, each a table)
//     12. event intervals where data are cut.
//      
//  For more info, see /doc/DATABASE.RULES
//
/////////////////////////////////////////////////////////////////////

#define MAXADC  20
#define MAXCHAN 10
#define DMAPSIZE 6

#include "Rtypes.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdlib.h>


class dtype {
// Utility class of typed data
public:
  dtype(string typ) : type(typ) {     
     ddata = 0;  idata = 0;  sdata = string(" ");
     loaded = false;
  }
  ~dtype() { }
  void Load(string data) {
    if (type == "d") ddata = (Double_t)atof(data.c_str());
    if (type == "i") idata = (Int_t)atoi(data.c_str());
    if (type == "s") sdata = data;
    if (Valid(type)) loaded = true;
  };
  bool Valid(string tp) {
    if (tp != "d" && tp != "i" && tp != "s") return false;
    return true;
  };
  bool IsLoaded() { return loaded; };
  string GetType() { return type; };
  Double_t GetD() { return ddata; };
  Int_t GetI() { return idata; };
  string GetS() { return sdata; };
private:
  Double_t ddata;
  Int_t idata;
  string type,sdata;
  bool loaded;
};


class TaKeyMap {
// Utility class of device keys mapped to data locations
public: 
  TaKeyMap() { type == ""; }
  ~TaKeyMap() { }
  TaKeyMap(const TaKeyMap& copy) {
    type   = copy.type;
    adc    = copy.adc;
    chan   = copy.chan;
    evboff = copy.evboff;
  };
  TaKeyMap &TaKeyMap::operator=(const TaKeyMap& rhs) {
    if (this != &rhs) {
      type   = rhs.type;
      adc    = rhs.adc;
      chan   = rhs.chan;
      evboff = rhs.evboff;
    }
    return *this;
  }
  void LoadType(string t) { 
    if (type != "") {
      if (t != type) cout << "TaKeyMap::ERROR: contradictory type"<<endl;
    } else {
      type = t; 
    } 
  };
  void LoadData(string key, int adcn, int chann, int evboffn) {
    pair<string, int> psi;  
    psi.first = key;
    psi.second = adcn;  adc.insert(psi);
    psi.second = chann;  chan.insert(psi);
    psi.second = evboffn;  evboff.insert(psi);    
  };
  string GetType() { return type; };
  Int_t GetAdc(const string& key) {
    if (adc.find(key) != adc.end() ) return adc[key];
    return -1;
  };
  Int_t GetChan(const string& key) {
    if (chan.find(key) != chan.end() ) return chan[key];
    return -1;
  };
  Int_t GetEvOffset(const string& key) {
    if (evboff.find(key) != evboff.end() ) return evboff[key];
    return -1;
  };
  vector<string> GetKeys() {
    vector<string> result;
    for(map<string, int>::iterator i = adc.begin(); i != adc.end(); i++) {
      result.push_back(i->first);
    }
    return result;
  };
  void Print() {
    cout << "TaKeyMap for type = "<<type<<endl;
    vector<string> keys = GetKeys();
    for (vector<string>::iterator ikey = keys.begin(); 
	 ikey != keys.end();  ikey++) {
      string key = *ikey;
      cout << "key "<<key<<"  adc "<<GetAdc(key)<<" channel "<<GetChan(key);
      cout <<"  event buffer offset "<<GetEvOffset(key)<<endl;
    }
  };
private:
  string type;
  map<string, Int_t> adc, chan, evboff;
};


class VaDataBase {

public:
 
  VaDataBase();
  virtual ~VaDataBase();
  virtual void Load(int run)=0;   // Load the database for this run.
  virtual void Write()=0;         // Write (update) database for this run.

// Getting things from database
  virtual Int_t GetRunNum() const { return runnum; };
// Generic get methods if you know the 'table' and 'key's you want.
  virtual vector<Double_t> GetData(string table, vector<string> key) const =0;
// Get run type, e.g. runtype = 'beam' (case insensitive)
  virtual string GetRunType()const =0 ;
// Get analysis type
  virtual string GetAnaType()const =0;
// Get feedback switch
 virtual string GetFdbkSwitch( const string &fdbktype) const=0;
// Get feedback timescale 
 virtual Int_t  GetFdbkTimeScale(const string &fdbktype) const=0;  
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  virtual Double_t GetDacNoise(const Int_t& adc, const Int_t& chan, 
       const string& key)const =0;  
// Get Pedestals for adc, chan
  virtual Double_t GetPedestal(const Int_t& adc, const Int_t& chan) const =0;
// Get Headers for decoding (needed by datamap)
  virtual UInt_t GetHeader(const string& device) const =0;
// Get Masks for decoding (needed by datamap)
  virtual UInt_t GetMask(const string& device) const =0;
// Get datamap info 
  virtual void DataMapReStart()=0;  // reset the iterator
  virtual Bool_t NextDataMap()=0;
  virtual string GetDataMapName()const =0;
  virtual string GetDataMapType()const =0;
  virtual TaKeyMap GetKeyMap(string device) const =0;
// Get a value from 'table', assumed a single value
  virtual Double_t GetValue(const string& table) const =0;
// Get a vector of integer values from 'table'
  virtual vector<Int_t> GetValueVector(const string& table) const =0;
// Get string from 'table'
  virtual string GetString(const string& table) const =0;
// Get Maximum number of events to process
  virtual Int_t GetMaxEvents() const =0;
// Get Window Delay
  virtual Int_t GetDelay() const =0;
// Get oversample factor
  virtual Int_t GetOverSamp() const =0;
// Get pair type (pair or quad) for this run
  virtual string GetPairType() const =0;  // returns 'pair' or 'quad', etc.
// Get a cut value for 'cutname'.  e.g. cutname = 'lowbeam', 'burpcut' 
  virtual Double_t GetCutValue(const string& cutname) const =0;
// Get number of cuts 
  virtual Int_t GetNumCuts() const =0;
// Get cut extensions, low and high 
  virtual vector<Int_t> GetExtLo() const=0;
  virtual vector<Int_t> GetExtHi() const=0;
// Get number of bad event intervals
  virtual Int_t GetNumBadEv() const=0;
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  virtual map <Int_t, vector<Int_t> > GetCutValues() const=0;
// Generic Get method, works if the key is unique
  virtual Double_t GetData(const string& key) const ;
// Self check
  virtual Bool_t SelfCheck() =0;

// Updating the database
  virtual void ClearAll()=0;                 // clears all (for this run only)
  virtual void ClearAll(Int_t run)=0;        // clears all (for this run only)
  virtual void ClearTable(string table)=0;   // wipe clean a table for this run
  virtual void ClearTable(Int_t run, string table)=0;   // wipe clean a table for this run
// Generic get method if you know the 'table' and 'key's you want.
  virtual void PutData(string table, const vector<string>& key, 
         const vector<Double_t>& data)=0;
// Put run type, e.g. runtype = 'beam' (case insensitive)
  virtual void PutRunType(string)=0;
// Put analysis type
  virtual void PutAnaType(string)=0;
// Put Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  virtual void PutDacNoise(Int_t adc, Int_t chan, string key, Double_t value)=0;  
// Put Pedestals for adc, chan
  virtual void PutPedestal(Int_t adc, Int_t chan, Double_t value)=0;
// Put Headers for decoding (needed by datamap)
  virtual void PutHeader(string device, Int_t header)=0;
// Put datamap info 
  virtual void PutDataMap(string table, vector<string> keys, vector<Int_t>& map)=0;
// Put a cut value for 'cutname'.  e.g. cutname = 'lowbeam', 'burpcut' 
  virtual void PutCutValue(string cutname, Double_t value)=0;
// Put number of cuts 
  virtual void PutNumCuts(Int_t numcuts)=0;
// Put cut extensions, low and high 
  virtual void PutExts(const vector<Int_t>& extlo, 
                       const vector<Int_t>& exthi)=0;
// Put number of bad event intervals
  virtual void PutNumBadEv(Int_t num_intervals)=0;
// For bad event intervals, get formatted results 
// First element of map goes from 0 to PutNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  virtual void PutCutValues(const map <Int_t, vector<Int_t> > & cuts)=0;
  
protected:

  int runnum;

private:

  VaDataBase(const VaDataBase &fn);
  VaDataBase& operator=(const VaDataBase &fn);

#ifdef DICT
ClassDef(VaDataBase,0)  // Database interface
#endif

};


#endif
