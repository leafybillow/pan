//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaAsciiDB.hh   (header file)
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Ascii based database, also used to read the 'control' file, 
//    and appended database with output.
//    Inherits interface from class VaDataBase.
//
//////////////////////////////////////////////////////////////////////////

#include "TaAsciiDB.hh"
#include "TaString.hh"

#ifdef DICT
ClassImp(TaAsciiDB)
#endif

TaAsciiDB::TaAsciiDB() { 
     didinit = kFALSE; 
     initdm  = kFALSE;
     firstiter = kTRUE;
     dacparam = new Double_t[2*MAXADC*MAXCHAN];     
     memset(dacparam,0,2*MAXADC*MAXCHAN*sizeof(Double_t));
     pedvalue = new Double_t[MAXADC*MAXCHAN];
     memset(pedvalue,0,MAXADC*MAXCHAN*sizeof(Double_t));
}

TaAsciiDB::~TaAsciiDB() {
     delete [] dacparam;
     delete [] pedvalue;
     tables.clear();
     for (multimap<string, vector<dtype*> >::iterator im = database.begin();
	  im != database.end();  im++) {
       vector<dtype *> ddata = im->second;
       for (vector<dtype* >::iterator id = ddata.begin();  
	    id != ddata.end(); id++) {
          	 delete *id;
       }
     }
     database.clear();
}

void TaAsciiDB::Load(int run) {
// Load the database for this run.
  InitDB();
  static char cnum[10];  sprintf(cnum,"%d",run);
  string rundbfile = "run_";  rundbfile += cnum;  rundbfile += ".db";
  dbfile = new ifstream(rundbfile.c_str());
  if ( ! (*dbfile) ) {
    cerr << "TaAsciiDB:: WARNING: run file "<<rundbfile<<" does not exist"<<endl;
    dbfile = new ifstream("control.db");
    if ( ! (*dbfile) ) {
      cerr << "TaAsciiDB:: WARNING: no 'control.db' file either !"<<endl;
      cerr << "You need a database to run.  Ask an expert."<<endl;
      return;
    }
    cerr << "TaAsciiDB:: Using control.db as default. (May be ok.)"<<endl;
  }
  string comment = "#";
  vector<string> strvect;
  TaString sinput,sline;
  while (getline(*dbfile,sinput)) {
    strvect.clear();   strvect = sinput.Split();
    sline = "";
    for (vector<string>::iterator str = strvect.begin();
      str != strvect.end(); str++) {
      if ( *str == comment ) break;
      sline += *str;   sline += " ";
    }
    if (sline == "") continue;
    strvect = sline.Split();
    multimap<string, vector<dtype*> >::iterator dmap = 
          database.find(FindTable(strvect[0]));
    if (dmap == database.end()) continue; 
    vector<dtype*> dv = dmap->second;
    vector<dtype*> datavect;  datavect.clear();
    for (int i = 1; i < (long)strvect.size(); i++) {
      if (i-1 < (long)dv.size()) {
        dtype *dat = new dtype(dv[i-1]->GetType()); 
        dat->Load(strvect[i]);   
        datavect.push_back(dat);
      }
    }
    LoadTable(FindTable(strvect[0]), datavect);
  }
  delete dbfile;
};

void TaAsciiDB::Write() {
// Write (update) database for this run.
};

Double_t TaAsciiDB::GetData(const string& key) const {
// Generic Get method, works if the key is unique
   Double_t dummy = 0;
   return dummy; 
};


vector<Double_t> TaAsciiDB::GetData(string table, vector<string> keys) const {
// Generic get method if you know the 'table' and 'keys' you want.
// Structure is 'table (key, data) (key, data)' i.e. a series of
// pairs of (string key, double data) after the string table
   vector<Double_t> result;  result.clear();
   multimap<string, vector<dtype*> >::const_iterator lb =
        database.lower_bound(table);
   multimap<string, vector<dtype*> >::const_iterator ub = 
        database.upper_bound(table);
   for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
        dmap != ub; dmap++) {
     vector<dtype*> datatype = dmap->second;
     for (vector<string>::const_iterator str = keys.begin(); 
       str != keys.end(); str++) {
       string key = *str;
       for (int k = 0; k < (long)datatype.size(); k++) {         
         if (datatype[k]->GetType() == "s") {
           if ( TaString(datatype[k]->GetS()).CmpNoCase(key) == 0 ) {
             result.push_back(GetData(datatype[k+1]));
           }
         }
       }
     }
   }
   return result;
};

string TaAsciiDB::GetData(string table, string key, Int_t index) const {
// Return key'd data at index in dtype vector.
   multimap<string, vector<dtype*> >::const_iterator lb =
        database.lower_bound(table);
   multimap<string, vector<dtype*> >::const_iterator ub = 
        database.upper_bound(table);
   for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
        dmap != ub; dmap++) {
     vector<dtype*> datatype = dmap->second;
     if (index >= 0 && index+1 < (long)datatype.size()) {
       if (datatype[0]->GetType() == "s") {
         if ( TaString(datatype[0]->GetS()).CmpNoCase( key) == 0 ) {
            if (datatype[index+1]->GetType() == "s") 
               return datatype[index+1]->GetS();
         }
       }
     }
   }
   return "0";
};

Double_t TaAsciiDB::GetData(dtype *d) const {
  if (d->GetType() == "i") return (Double_t)d->GetI();
  if (d->GetType() == "d") return d->GetD();
  if (d->GetType() == "s") return 0;
  return 0;
};

string TaAsciiDB::GetRunType() const {
// Get run type, e.g. runtype = 'beam' 
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound("runtype");
   if (dmap == database.end()) return "unknown";
   if (database.count("runtype") != 1) return "ambiguous";
   vector<dtype*> datatype = dmap->second;
   if (datatype.size() > 1) return "ambiguous";
   if (datatype[0]->GetType() == "s") return datatype[0]->GetS(); 
   return "unknown";
};

string TaAsciiDB::GetAnaType() const {
// Get analysis type, e.g. anatype = 'beam' 
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound("anatype");
   if (dmap == database.end()) return "unknown";
   if (database.count("anatype") != 1) return "ambiguous";
   vector<dtype*> datatype = dmap->second;
   if (datatype.size() > 1) return "ambiguous";
   if (datatype[0]->GetType() == "s") return datatype[0]->GetS(); 
   return "unknown";
};

string TaAsciiDB::GetFdbkSwitch( const string &fdbktype )const {
  // get the feedback switch state corresponding to feedback type fdbktype.
  string table = "feedback";
  if ( database.count("feedback") >1){
    clog<<" feedback"<<fdbktype<<" switched "
        <<GetData(table,fdbktype, 0)<<endl;
    return GetData(table,fdbktype, 0);
  }
  else{
    // Commented out: Why should this be an error?  Why not just return
    // "off" (or "", which is not "on")?
    //      cerr<<"TaAsciiDB::GetFdbkSwitch ERROR\n"
    //  	<<"TABLE not declared correctly, need feedback table. "
    //  	<<"Example of line format in .db file:\n"
    //          <<" feedback    AQ   on   3 "<<endl;
    return "";
  }     
}; 

Int_t TaAsciiDB::GetFdbkTimeScale( const string &fdbktype ) const{
  // get the timescale of feedback type fdbktype
  string table = "feedback";
 if ( database.count("feedback") >1){ 
     multimap<string, vector< dtype* > >::const_iterator lb = database.lower_bound(table);        
     multimap<string, vector< dtype* > >::const_iterator ub = database.upper_bound(table);        
     for ( multimap<string, vector< dtype* > >::const_iterator dmap = lb; dmap != ub; dmap++){
       vector< dtype* > datatype = dmap->second;
       if (datatype[0]->GetType() == "s"){
	 if ( TaString(datatype[0]->GetS()).CmpNoCase(fdbktype) == 0 ){
	   if (datatype[2]->GetType() == "i") return datatype[2]->GetI();
           clog<<" feedback "<<fdbktype<<" timescsale "<<datatype[2]->GetI()<<endl;
	 }
       }
     }
     return 0;
 }
 else{
   cerr<<"TaAsciiDB::GetFdbkTimeScale( const string &fdbktype ) ERROR\n"
	<<"TABLE not declared correctly, need feedback table. "
	<<"Example of line format in .db file:\n"
        <<" feedback    AQ   on   3 "<<endl;
   return 0;
 }
};   

Double_t TaAsciiDB::GetDacNoise(const Int_t& adc, const Int_t& chan, const string& key) const {
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  int idx;
  static Bool_t firstgdn = kTRUE;
  if (firstgdn)  {
     firstgdn = kFALSE;
     vector<string> keys;
     keys.clear();
     keys.push_back("adc");   
     keys.push_back("chan");
     keys.push_back("slope");
     keys.push_back("int");
     vector<Double_t> data = GetData("dacnoise",keys);
     int iadc,ichan;
     int index = 0;
     while (index < (long)data.size()) 
       {
        if ((index+(long)keys.size()-1) < (long)data.size()) 
	  {
           iadc = (int)data[index];
           ichan= (int)data[index+1];
           idx = iadc*MAXCHAN+ichan;
           if (idx < MAXADC*MAXCHAN) 
             {
              dacparam[idx] = data[index+2];
              dacparam[idx+MAXADC*MAXCHAN] = data[index+3];
             }
          }
        index += keys.size();
      }
  }
    idx = adc*MAXCHAN+chan;
    if (idx >= 0 && idx < MAXADC*MAXCHAN) 
      {
       if (TaString(key).CmpNoCase("slope") == 0) return dacparam[idx];
      }
    if (TaString(key).CmpNoCase("int") == 0 || TaString(key).CmpNoCase("intercept") == 0)
      {
      return dacparam[idx+MAXADC*MAXCHAN];
      } 
    else 
      {
     cerr << "WARNING: TaAsciiDB::GetDacNoise:";
     cerr << "  illegal combination of adc and channel #"<<endl;
     return 0;
      }
  return 0;
};

Double_t TaAsciiDB::GetPedestal(const Int_t& adc, const Int_t& chan) const{
// Get Pedestals for adc, chan 
  int idx;
  static Bool_t firstped = kTRUE;
  if (firstped) {
    firstped = kFALSE;
    vector<string> keys;
    keys.clear();
    keys.push_back("adc");   
    keys.push_back("chan");
    keys.push_back("value");
    vector<Double_t> data = GetData("ped",keys);
    int iadc,ichan;
    int index = 0;
    while (index < (long)data.size()) {
      if ((index+(long)keys.size()-1) < (long)data.size()) {
        iadc = (int)data[index];
        ichan= (int)data[index+1];
        idx = iadc*MAXCHAN+ichan;
        if (idx < MAXADC*MAXCHAN) {
           pedvalue[idx] = data[index+2];
         }
      }
      index += keys.size();
    }
  }
  idx = adc*MAXCHAN+chan;
  if (idx >=0 && idx < MAXADC*MAXCHAN) {
     return pedvalue[idx];
  } else {
     cerr << "WARNING: TaAsciiDB::GetPedestal:";
     cerr << "  illegal combination of adc and channel #"<<endl;
     return 0;
  }
};

UInt_t TaAsciiDB::GetHeader(const string& device) const {
// Get Headers for decoding
   string table = "header";
   return TaString(GetData(table, device, 0)).Hex();
};

UInt_t TaAsciiDB::GetMask(const string& device) const {
// Get Mask for decoding 
   string table = "header";
   return TaString(GetData(table, device, 1)).Hex();
};

Double_t TaAsciiDB::GetCutValue(const string& cutname) const {
// Get a cut value for 'value'.  e.g. value = 'lobeam', 'burpcut' 
   return GetValue(cutname);
};

Int_t TaAsciiDB::GetNumCuts() const {
// Get number of cuts "ncuts" in the database.
   return (Int_t)GetValue("ncuts");
};

vector<Int_t> TaAsciiDB::GetEvLo() const {
// Get cut evlo event intervals 
   return GetValueVector("evlo");
};

vector<Int_t> TaAsciiDB::GetEvHi() const {
// Get cut evhi event intervals 
   return GetValueVector("evhi");
};

Int_t TaAsciiDB::GetNumBadEv() const {
// Get number of bad event intervals
   return database.count("evint");
};

map <Int_t, vector<Int_t> > TaAsciiDB::GetCutValues() const {
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
   map <Int_t, vector<Int_t> > result;
   vector<Int_t> temp;
   result.clear();
   Int_t k = 0;
   multimap<string, vector<dtype*> >::const_iterator lb =
          database.lower_bound("evint");
   multimap<string, vector<dtype*> >::const_iterator ub = 
          database.upper_bound("evint");
   for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
         dmap != ub; dmap++) {
     vector<dtype*> datav = dmap->second;
     temp.clear();
     for (vector<dtype*>::iterator idat = datav.begin();
          idat != datav.end(); idat++) {
       if ((*idat)->GetType() == "i") 
           temp.push_back((*idat)->GetI());
     }
     result.insert(make_pair(k++, temp));
   }
   return result;   
};

Int_t TaAsciiDB::GetMaxEvents() const {
// returns number of events to process
   return (Int_t)GetValue("maxevents");
};

Int_t TaAsciiDB::GetDelay() const {
// returns helicity delay (in windows)
   return (Int_t)GetValue("windelay");
};

Int_t TaAsciiDB::GetOverSamp() const {
// returns oversample factor
   return (Int_t)GetValue("oversamp");
};
 
string TaAsciiDB::GetPairType()  const {
// returns pair type (pair or quad) for this run.
   return GetString("pairtype");
};

Double_t TaAsciiDB::GetValue(const string& table) const {
// Return single value from table "table".  This assumes the data
// are in a pair  "table   value" where table is a unique string and
// value the single Double_t that belongs to it.
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound(TaString(table).ToLower());
   if (dmap == database.end() || 
       database.count(TaString(table).ToLower()) == 0) {
     cerr << "ERROR: TaAsciiDB: Unknown database table "<<table<<endl;
     return 0;
   }
   if (database.count(TaString(table).ToLower()) > 1) {
     cerr << "ERROR: TaAsciiDB: Mulitply defined table "<<table<<endl;
     cerr << "Fix the database to have one instance."<<endl;
     return 0;
   }
   vector<dtype*> datatype = dmap->second;
   if (datatype[0]->GetType() == "i") return (Double_t)datatype[0]->GetI(); 
   if (datatype[0]->GetType() == "d") return datatype[0]->GetD(); 
   cerr << "ERROR: TaAsciiDB: Illegal data type for table "<<table<<endl;
   cerr << "Must be an integer or double."<<endl;
   return 0;
};

string TaAsciiDB::GetString(const string& table) const {
// Return "string" from table "table".  This assumes the data
// are in a pair  "table  string" where table is unique and
// value the single string that belongs to it.
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound(TaString(table).ToLower());
   if (dmap == database.end() || 
       database.count(TaString(table).ToLower()) == 0) {
     cerr << "ERROR: TaAsciiDB: Unknown database table "<<table<<endl;
     return 0;
   }
   if (database.count(TaString(table).ToLower()) > 1) {
     cerr << "ERROR: TaAsciiDB: Mulitply defined table "<<table<<endl;
     cerr << "Fix the database to have one instance."<<endl;
     return 0;
   }
   vector<dtype*> datatype = dmap->second;
   if (datatype[0]->GetType() == "s") return datatype[0]->GetS(); 
   cerr << "ERROR: TaAsciiDB: Illegal data type for table "<<table<<endl;
   cerr << "Must be a string."<<endl;
   return 0;
};

vector<Int_t> TaAsciiDB::GetValueVector(const string& table) const {
   vector<Int_t> result;
   result.clear();
   multimap<string, vector<dtype*> >::const_iterator lb =
          database.lower_bound(TaString(table).ToLower());
   multimap<string, vector<dtype*> >::const_iterator ub = 
          database.upper_bound(TaString(table).ToLower());
   for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
         dmap != ub; dmap++) {
     vector<dtype*> datav = dmap->second;
     for (vector<dtype*>::const_iterator idat = datav.begin();
          idat != datav.end(); idat++) {
       if ((*idat)->GetType() == "i") 
           result.push_back((*idat)->GetI());
     }
   }
   return result;
};

void TaAsciiDB::ClearAll() {
// Clear the database for this run
   ClearAll(runnum);
};

void TaAsciiDB::ClearAll(Int_t run) { 
// Clear the databaase for run #run
};

void TaAsciiDB::ClearTable(string table) {
// wipe clean a table for this run
  ClearTable(runnum, table);
};

void TaAsciiDB::ClearTable(Int_t run, string table) {
// wipe clean a table for run #run
};

void TaAsciiDB::PutData(string table, const vector<string>& key, 
		        const vector<Double_t>& data) {
// Generic pet method if you know the 'table' and 'key's you want.
};

void TaAsciiDB::PutRunType(string) {
// Put run type, e.g. runtype = 'beam' (case insensitive)
};

void TaAsciiDB::PutAnaType(string) {
// Put analysis type, e.g. anatype = 'beam' (case insensitive)
};

void TaAsciiDB::PutDacNoise(Int_t adc, Int_t chan, string key, Double_t value) {
// Put Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
};

void TaAsciiDB::PutPedestal(Int_t adc, Int_t chan, Double_t value) {
// Put Pedestals for adc, chan
};

void TaAsciiDB::PutHeader(string device, Int_t header) {
// Put Headers for decoding (needed by datamap)
};

void TaAsciiDB::PutDataMap(string table, vector<string> keys, vector<Int_t>& map) {
// Put datamap info 
};

void TaAsciiDB::PutCutValue(string cutname, Double_t value) {
// Put a cut value for 'cutname'.  e.g. cutname = 'lobeam', 'burpcut' 
};

void TaAsciiDB::PutNumCuts(Int_t numcuts) {
// Put number of cuts 
};

void TaAsciiDB::PutCuts(const vector<Int_t>& evlo, 
                        const vector<Int_t>& evhi) {
// Put cut evlo, evhi event intervals 
};

void TaAsciiDB::PutNumBadEv(Int_t num_intervals) {
// Put number of bad event intervals
};

void TaAsciiDB::PutCutValues
     (const map <Int_t, vector<Int_t> > & cuts) {
};

void TaAsciiDB::InitDB() {
// Define the database structure.
// Table names correspond to first column in Ascii DB.
// Careful, an error here can lead to incomprehensible core dump.
// Probably this method, and others, need to be part of the 
// super class VaDataBase, but I put it here for now.

  if (didinit) return;
  didinit = kTRUE;

  vector <dtype*> columns;
  columns.reserve(20);

  tables.clear();
  tables.push_back("run");           //   0
  tables.push_back("runtype");       //   1
  tables.push_back("anatype");       //   2
  tables.push_back("maxevents");     //   3
  tables.push_back("lobeam");        //   4
  tables.push_back("burpcut");       //   5
  tables.push_back("dacnoise");      //   6
  tables.push_back("ped");           //   7
  tables.push_back("header");        //   8
  tables.push_back("datamap");       //   9
  tables.push_back("ncuts");         //  10
  tables.push_back("evlo");          //  11
  tables.push_back("evhi");          //  12
  tables.push_back("evint");         //  13
  tables.push_back("windelay");      //  14
  tables.push_back("oversamp");      //  15
  tables.push_back("pairtype");      //  16
  tables.push_back("feedback");      //  17 

  pair<string, int> sipair;
  int k;

  for (int i = 0; i < (long)tables.size(); i++ ) { 
    sipair.first = tables[i];
    sipair.second = 0;
    dbinit.insert(sipair);
    columns.clear();
    if (i == 0) columns.push_back(new dtype("i"));  // run
    if (i == 1) columns.push_back(new dtype("s"));  // runtype
    if (i == 2) columns.push_back(new dtype("s"));  // anatype
    if (i == 3) columns.push_back(new dtype("i"));  // maxevents
    if (i == 4) columns.push_back(new dtype("d"));  // lobeam
    if (i == 5) columns.push_back(new dtype("d"));  // burpcut
    if (i == 6) {  // dac noise
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 7) {  // pedestals
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 8) { // header
       for (int k = 0; k < 3; k++) columns.push_back(new dtype("s"));
    }
    if (i == 9) { // datamap
       for (k = 0; k < 2; k++) columns.push_back(new dtype("s"));
       for (k = 0; k < 3; k++) columns.push_back(new dtype("i"));
       for (k = 0; k < 10; k++) columns.push_back(new dtype("s"));
    }
    if (i == 10) columns.push_back(new dtype("i"));  // ncuts
    if (i == 11) {   // evlo
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 12) {  // evhi
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 13) {  // evint
      for (k = 0; k < 20; k++) columns.push_back(new dtype("i"));
    }
    if (i == 14) columns.push_back(new dtype("i"));  // windelay
    if (i == 15) columns.push_back(new dtype("i"));  // oversamp
    if (i == 16) columns.push_back(new dtype("s"));  // pairtype
    if (i == 17) {
      columns.push_back(new dtype("s"));
      columns.push_back(new dtype("s"));
      columns.push_back(new dtype("i"));
    }
    sipair.second = columns.size(); 
    colsize.insert(sipair);
    LoadTable(tables[i],columns);
  }
};

void TaAsciiDB::LoadTable(string table, vector<dtype*> columns) {
// Load the database for a 'table'
// The first call will set up the structure but load empty data.
// The 2nd call (loadflg==1) must overwrite the empty data of 1st call.
// Subsequent calls load data.
  if  ( !didinit ) {
    cerr << "TaAsciiDB:: ERROR: cannot LoadTable before InitDB"<<endl;
    return;
  }
  map<string, int>::iterator im = dbinit.find(table);
  if (im == dbinit.end()) return;
  int loadflg = im->second;
  if (loadflg == 1) {  
     multimap<string, vector<dtype*> >::iterator dmap = 
          database.find(table);
     if (dmap == database.end()) return;
     map<string, int>::iterator si = colsize.find(table);   
     if (si != colsize.end() && columns.size() > 0) {
       dtype *dt = columns[columns.size()-1];
       for (int k = 0; k < colsize[table]-(long)columns.size(); k++) 
	 columns.push_back(new dtype(dt->GetType()));
     }
     dmap->second = columns;
  } else {
     pair<string, vector<dtype*> > dbpair;
     dbpair.first = table;  
     dbpair.second = columns;
     database.insert(dbpair);
  }
  loadflg++;
  im->second = loadflg;
//  PrintDataBase();   // DEBUG
};

void TaAsciiDB::PrintDataBase() {
  cout << "\n\n--- Printout of database ---"<<endl;
  for (multimap<string, vector<dtype*> >::iterator dmap = database.begin();
       dmap != database.end() ;  dmap++) {
    string table = dmap->first;
    vector<dtype*> datatypes = dmap->second;
    cout << "\nTable :  "<<table<<endl;
    for (vector<dtype*>::iterator d = datatypes.begin();
         d != datatypes.end(); d++) {
         dtype* dat = *d;
         cout << " | Data Type "<<dat->GetType();
         cout << " Value = "<<GetData(dat);
    }
  }
};

string TaAsciiDB::FindTable(string table) {
// Return 'case insensitive' table name if table exists
  for (int i = 0; i < (long)tables.size(); i++) {
    if ( TaString(table).CmpNoCase(tables[i]) == 0 ) return tables[i];
  }
  return " ";
};

void TaAsciiDB::DataMapReStart() {
// To reset the datamap iterator.  Must be done before using
// NextDataMap() the first time.
  InitDataMap();
  dmapiter = datamap.begin();
};

Bool_t TaAsciiDB::NextDataMap() {
  InitDataMap();
  if (firstiter) {
      firstiter = kFALSE;
      return kTRUE;
  } 
  dmapiter++;
  if (dmapiter != datamap.end()) return kTRUE;
  return kFALSE; 
};

string TaAsciiDB::GetDataMapName() const {
  if ( !initdm ) {
    cerr << "TaAsciiDB:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  return dmiter->first;
};
  
string TaAsciiDB::GetDataMapType() const {
  if ( !initdm ) {
    cerr << "TaAsciiDB:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  TaKeyMap keymap = dmiter->second;
  return keymap.GetType();
};
 
TaKeyMap TaAsciiDB::GetKeyMap(string devicename) const {
  map< string, TaKeyMap >::const_iterator mapiter = datamap.find(devicename);
  TaKeyMap nothing;  nothing.LoadType("unknown");
  if (mapiter == datamap.end()) return nothing;
  return mapiter->second;
};

void TaAsciiDB::InitDataMap() {
  static pair<string, vector<dtype*> > sdt;
  int adc, chan, evb;
  string key;
  if ( !didinit ) {
    cerr << "TaAsciiDB:: ERROR: Cannot init datamap without first init DB"<<endl;
    return;
  }
  if (initdm) return;  // already initialized
  datamap.clear();
  string table = "datamap";
  multimap<string, vector<dtype*> >::iterator lb =
          database.lower_bound(table);        
  multimap<string, vector<dtype*> >::iterator ub = 
          database.upper_bound(table);
  for (multimap<string, vector<dtype*> >::iterator dmap = lb;
         dmap != ub; dmap++) {
     vector<dtype*> datav = dmap->second;
     string devname = datav[1]->GetS();       
     map<string, TaKeyMap >::iterator dm = datamap.find(devname);
     TaKeyMap keymap;
     if (dm != datamap.end()) keymap = dm->second;
     keymap.LoadType(datav[0]->GetS());
     adc  = datav[2]->GetI();
     chan = datav[3]->GetI();
     evb  = datav[4]->GetI();
     int istart = 5;
     for (int k = istart; k < (long)datav.size(); k++) {
       if ( !datav[k]->IsLoaded() ) continue;
       key = datav[k]->GetS();
       keymap.LoadData( key, adc, chan + k - istart, evb + k - istart);
     }
     pair<string, TaKeyMap > skm;
     skm.first = devname;  skm.second = keymap;
     pair<map<string, TaKeyMap>::iterator, bool> p = datamap.insert(skm);
     if ( !p.second ) {
           datamap.erase(dm);
           datamap.insert(skm);
     }
  }
  dmapiter = datamap.begin();
  initdm = kTRUE;
// Comment: should CHECK datamap, in case of typo errors.
//  PrintDataMap();   // DEBUG
};

void TaAsciiDB::PrintDataMap() {
  cout << "\n\n--- Printout of datamap ---"<<endl;
  for (multimap<string, TaKeyMap >::const_iterator dmap = datamap.begin();
	 dmap != datamap.end() ;  dmap++) {
    string devname = dmap->first;
    cout << "\nDevice name "<<devname<<"  and associated keymaps: "<<endl;
    TaKeyMap keymap = dmap->second;
    keymap.Print();
  }
};

Bool_t TaAsciiDB::SelfCheck() {
// Returns kTRUE if the database makes good sense.  This enforces some rules.
// (First version, to be expanded...  see /doc/DATABASE.RULES)
   string stest;
   Int_t itest;
   vector<string> keys;
   TaKeyMap keymap = datamap["tir"];       // must have tir data in datamap
   keys = keymap.GetKeys();
   Int_t ok = 0;
   for (vector<string>::iterator ikey = keys.begin(); ikey != keys.end(); ikey++) {
     string key = *ikey;
     if (key == "tirdata") ok = 1;
   }
   if ( !ok ) {
     cerr << "TaAsciiDB:: SelfCheck ERROR:  No tirdata defined in database"<<endl;
     return kFALSE;
   }
   stest = GetPairType();
   if (strlen(stest.c_str())-1 <= 0) {
      cerr << "TaAsciiDB:: SelfCheck ERROR:  No pair type defined in database"<<endl;
      return kFALSE;
   }
   itest = GetDelay();
   if (itest != 0 && itest != 8 ) {
     cerr << "TaAsciiDB:: SelfCheck ERROR:  'windelay' = " << itest << " outside range 0-8" <<endl;
      return kFALSE;
   }
   itest = GetOverSamp();
   if (itest <= 0 || itest > 12 ) {
     cerr << "TaAsciiDB:: SelfCheck ERROR:  'oversamp' = " << itest << " outside range 0-12" <<endl;
      return kFALSE;
   }
   stest = GetRunType();
   if (strlen(stest.c_str())-1 <= 0) {
      cerr << "TaAsciiDB:: SelfCheck WARNING:  No 'runtype' defined in database"<<endl;
   }
   return kTRUE;
};








