#ifndef ROOT_TaDataBase
#define ROOT_TaDataBase

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaAnalysis.cc  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
//////////////////////////////////////////////////////////////////////////
//
//  Database class.  This always reads database from an ASCII file.
//  For MYSQL access, it first executes a Perl script which generates
//  the ASCII file.
//
//  The database is organized in tables, see list below.
//  The tables are denoted by a string table name (e.g. 'dacnoise').  
//  In each table is a series of columns of information.  The columns 
//  are 'typed' data, i.e. data of a type string, int, or double.
//
//  Tables include:
//
//      1. run  (the CODA run number)
//      2. analysis type
//      3. maxevents (the number of events to analyze)
//      4. pair type ('pair' or 'quad')
//      5. window delay
//      6. oversampling factor
//      7. dac noise parameters
//      8. pedestals 
//      9. datamap and header info
//     10. named cuts ('lobeam', 'burpcut', etc, each a table)
//     11. event intervals where data are cut.
//      
//  For usage instructions, syntax rules, and other details, see
//          /doc/DATABASE.TXT
//
/////////////////////////////////////////////////////////////////////

#define MAXADC  20
#define MAXSCAL 6
#define MAXCHAN 10
#define MAXSCALCHAN 32
#define DMAPSIZE 6
#define MAXLINES 500

#include "Rtypes.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#ifndef __CINT__
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <stdlib.h>
#include "PanTypes.hh"
#include "TFile.h"
#include "TArrayC.h"
#include "TObject.h"

using namespace std;

class TaString;

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
  void Load(Double_t data) {
    if (type == "d") ddata = data;
    if (type == "i") idata = (Int_t)data;
    if (type == "s") {
       cout << "dtype::ERROR: d->s not supported"<<endl;
       return;
    }
    if (Valid(type)) loaded = true;
  };
  void Load(Int_t data) {
    if (type == "d") ddata = (Double_t)data;
    if (type == "i") idata = data;
    if (type == "s") {
       cout << "dtype::ERROR: i->s not supported"<<endl;
       return;
    }
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

class TaRootRep : public TObject {
// Character data for ROOT output to read/write database
public:
  TaRootRep()  { carray = new TArrayC(fgMaxChar); Clear(); }
  ~TaRootRep() { delete carray; }
  void Clear() { fNumchar = 0; fptr = 0; carray->Reset(); };
  void Put(const char *tline) {
// Input character lines.  They must end in '\n'
     jfirst = fNumchar;
     if (fNumchar >= fgMaxChar) {
// This error means you need to recompile with bigger fgMaxChar.
// Do this: 'wc -l file' on database 'file'.
        cout << "ERROR: TaRootRep:: Truncated output"<<endl;
        return;
     }
     for (i = jfirst; i < fgMaxChar; i++) {
        j = i - jfirst;
        carray->AddAt(tline[j],i);
        fNumchar++;
        if (tline[j] == '\n') return;
      }
     return;
  };  
  Bool_t NextLine() { return (fptr < carray->GetSize()); };
  string GetLine( ) {
     string result = "";
     for (i = fptr; i < carray->GetSize(); i++) {
       result += carray->At(i);
       if (carray->At(i) == '\n') break;
     }
     fptr = i+1;
     return result;
  };
  void Print() {
   for (i = 0; i < carray->GetSize(); i++) cout << carray->At(i);
   cout << endl << flush;
  }
private:
  static const int fgMaxChar = 20000;  //!  This is about 2x normal size
  int i,j,jfirst,fptr;   //!
  int fNumchar;    
  TArrayC *carray; 

#ifndef NODICT
  ClassDef(TaRootRep,2) // Character data ROOT output for database
#endif
};

class TaKeyMap {
// Utility class of device keys mapped to data locations
public: 
  TaKeyMap() { type == ""; rotate_flag = -1; }
  ~TaKeyMap() { }
  TaKeyMap(const TaKeyMap& copy) {
    type    = copy.type;
    readout = copy.readout;
    devnum  = copy.devnum;
    chan    = copy.chan;
    evboff  = copy.evboff;
    rotate_flag  = copy.rotate_flag;
  };
  TaKeyMap &TaKeyMap::operator=(const TaKeyMap& rhs) {
    if (this != &rhs) {
      type    = rhs.type;
      readout = rhs.readout;
      devnum  = rhs.devnum;
      chan    = rhs.chan;
      evboff  = rhs.evboff;
      rotate_flag  = rhs.rotate_flag;
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
  void LoadData(string key, string read, int devnumn, int chann, int evboffn) {
    pair<string, int> psi;  
    psi.first = key;
    psi.second = devnumn;  devnum.insert(psi);
    psi.second = chann;  chan.insert(psi);
    psi.second = evboffn;  evboff.insert(psi);    
    pair<string,string> pstst;
    pstst.first = key;
    pstst.second = read;  readout.insert(pstst);
  };
  string GetType() { return type; };
  string GetReadOut(const string& key) { return readout[key]; };
  Bool_t IsAdc(const string& key) {
    if (GetReadOut(key) == "adc") return kTRUE;
    return kFALSE;
  };     
  Bool_t IsScaler(const string& key) {
    if (GetReadOut(key) == "scaler") return kTRUE;
    return kFALSE;
  };     
  Int_t GetAdc(const string& key) {
    if (IsAdc(key)) return devnum[key];
    return 0;
  };
  Int_t GetDevNum(const string& key) {
    if (devnum.find(key) != devnum.end() ) return devnum[key];
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
    for(map<string, int>::iterator i = devnum.begin(); i != devnum.end(); i++) {
      result.push_back(i->first);
    }
    return result;
  };
  void SetRotate(Int_t flag) { rotate_flag = flag; };
  Int_t GetRotate() { return rotate_flag; };  // 1=rotated, 0=not, -1=unknown
  void Print() {
    cout << "TaKeyMap for type = "<<type<<endl;
    vector<string> keys = GetKeys();
    for (vector<string>::iterator ikey = keys.begin(); 
	 ikey != keys.end();  ikey++) {
      string key = *ikey;
      cout << "key "<<key<<"  device number "<<GetDevNum(key)<<" channel "<<GetChan(key);
      cout <<"  event buffer offset "<<GetEvOffset(key)<<endl;
      cout << " Readout type "<<GetReadOut(key)<<endl;
    }
  };
private:
  string type;
  Int_t rotate_flag;
  map<string, Int_t> devnum, chan, evboff;
  map<string, string> readout;
};


class TaDataBase {

public:
 
  TaDataBase();
  virtual ~TaDataBase();

// Load the database for this run, with possible command line over-ride
  void Read(int run, const vector<string>& dbcommand);  
  void Write();        // Write database for this run (to ASCII and MYSQL)
                       // (but this happens only if you have 'Put' something)
  void WriteRoot();    // Write database to the opened ROOT output.
  void Print();        // Human readable printout (for end-run summary)
// Self check.  Returns kFALSE if problems.
  Bool_t SelfCheck();
// The following 3 methods are for thorough debugging, not for normal use.
  void Checkout();     // Debug checkout 
  void PrintDataBase();// Debug printout of entire database
  void PrintDataMap(); // Debug printout of datamap

// -------------------------------------------------
// Get() methods to obtain data
// -------------------------------------------------
  Int_t GetRunNum() const { return runnum; };
// Get analysis type
  string GetAnaType()const;
// Get simulation type
  string GetSimulationType() const;
// Get timestamp
  string GetTimestamp() const;
// Get random or toggle mode
  string GetRandomHeli() const;
// Get blinding parameters
  string GetBlindingString() const;
  vector<Double_t> GetBlindingParams() const;
// Get feedback switch
  string GetFdbkSwitch( const string &fdbktype) const;
// Get feedback timescale 
  Int_t GetFdbkTimeScale(const string &fdbktype) const;  
  string GetFdbkMonitor(const string &fdbktype) const;  
// Get Dac noise parameters for adc,chan with key = 'slope'
  Double_t GetDacNoise(const Int_t& adc, const Int_t& chan, 
        const string& key)const;  
// Get Pedestals for adc, chan
  Double_t GetAdcPed(const Int_t& adc, const Int_t& chan) const;
// Get Pedestals for scaler, chan
  Double_t GetScalPed(const Int_t& adc, const Int_t& chan) const;
// Get Headers for decoding (needed by datamap)
  UInt_t GetHeader(const string& device) const;
// Get Masks for decoding (needed by datamap)
  UInt_t GetMask(const string& device) const;
// Generic Get methods
  Double_t GetData(dtype* dat) const;
  string GetData(string table, string key, Int_t index) const;
// Reset the datamap iterator
  void DataMapReStart();  
  Bool_t NextDataMap();
// Get datamap info 
  string GetDataMapName() const;
  string GetDataMapType() const;
  TaKeyMap GetKeyMap(string device) const;
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
// Get names of cuts
  vector<string> GetCutNames () const;
// Get cut extensions, low and high 
  vector<Int_t> GetExtLo() const;
  vector<Int_t> GetExtHi() const;
// Get number of bad event intervals
  Int_t GetNumBadEv() const;
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  map <Int_t, vector<Int_t> > GetCutInt() const;
// Generic get method if you know the 'table' and 'key's you want.
  vector<Double_t> GetData(string table, vector<string> key) const;
// Generic Get method, works if the key is unique
  Double_t GetData(const string& key) const ;
// Get a value from 'table', assumed a single value
  Double_t GetValue(const string& table) const;
// Get a vector of integer values from 'table'
  vector<Int_t> GetValueVector(const string& table) const;
// Get string from 'table'
  string GetString(const string& table) const;
// Return cut number for name
  Cut_t GetCutNumber (TaString s) const;  
  // Return value of checksum
  UInt_t GetCksum () const { return fCksum; }

// -------------------------------------------------
// Put() methods to modify the database.
// Only certain data can be modified by the analyzer,
// the rest must be modified by hand, using scripts.
// -------------------------------------------------
// Put Dac noise parameters for adc,chan with key = 'slope'
  void PutDacNoise(const Int_t& adc, const Int_t& chan, const Double_t& slope);
// Put Pedestals for adc, chan
  void PutAdcPed(const Int_t& adc, const Int_t& chan, const Double_t& value);
// Put Pedestals for scaler, chan
  void PutScalPed(const Int_t& scal, const Int_t& chan, const Double_t& value);
// Put cut intervals
  void PutCutInt(const vector<Int_t> evint); 
  
private:

  Int_t runnum;
  void InitDB();
// Load database from root file. Invoked for dbcommand '-D useroot filename'
  void ReadRoot(string filename); 
  Int_t ChkDbCommand();
  void SetDbCommand();
  void LoadTable(string table, vector<dtype*>);
  string FindTable(string table);
  void InitDataMap();
  string StripRotate (const string long_devname);
  Int_t RotateState(const string long_devname);

// To read from MYSQL, use command line interface, i.e '-D mysql'
  void Mysql(string action);  // MYSQL interface (action = "read", "write")
  void PutData(string table, vector<dtype *> dvect);
  void ToRoot();
  void LoadCksum (const string);

  vector<string> tables;
  multimap<string, vector<dtype*> > database;
  TaRootRep *rootdb;   // ROOT representation
  string fileRead;
  vector<string> dbcommand;
  map<string, TaKeyMap > datamap; 
  map<string, Bool_t> dbput;
  map<string, TaKeyMap >::const_iterator cdmapiter;
  map<string, TaKeyMap >::iterator dmapiter;
  map<string, int> dbinit,colsize;
  Bool_t didinit,didread,didput,initdm,firstiter;
  Bool_t usemysql,useroot,usectrl;
  Double_t *dacparam, *adcped, *scalped;
  Bool_t *fFirstgdn, *fFirstAdcPed, *fFirstScalPed;
  Int_t nbadev;
  UInt_t fCksum;  // checksum

  TaDataBase(const TaDataBase &fn);
  TaDataBase& operator=(const TaDataBase &fn);

#ifndef NODICT
ClassDef(TaDataBase,0)  // Database class
#endif

};

inline void TaDataBase::WriteRoot() {
  if (rootdb) rootdb->Write();
};

#endif
