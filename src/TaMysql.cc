//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaMysql.cc
//
//    Authors :  R. Suleiman
//
//  Mysql based database
//  Inherits interface from class VaDataBase.
//
//  Reads control from a mysql database
//
//////////////////////////////////////////////////////////////////////////

#include "TaMysql.hh"
#include "TaString.hh" 


#ifdef DICT
ClassImp(TaMysql)
#endif

TaMysql::TaMysql() { 
     didinit = kFALSE; 
     initdm  = kFALSE;
     firstiter = kTRUE;
     dacparam = new Double_t[2*MAXADC*MAXCHAN];     
     memset(dacparam,0,2*MAXADC*MAXCHAN*sizeof(Double_t));
     pedvalue = new Double_t[MAXADC*MAXCHAN];
     memset(pedvalue,0,MAXADC*MAXCHAN*sizeof(Double_t));
}

TaMysql::~TaMysql() {
     delete [] dacparam;
     delete [] pedvalue;
}

void TaMysql::Load(int run) {
// Load the database for this run.
  InitDB();
  fRunIndex = run;
  return;
};

void TaMysql::Write() {
// Write (update) database for this run.
};

Double_t TaMysql::GetData(const string& key) const {
// Generic Get method, works if the key is unique
   Double_t dummy = 0;
   return dummy; 
};

vector<Double_t> TaMysql::GetData(string table, vector<string> keys) const {
// Generic get method if you know the 'table' and 'keys' you want.
// Structure is 'table (key, data) (key, data)' i.e. a series of
// pairs of (string key, double data) after the string table
   vector<Double_t> result;  result.clear();
   return result;
};

string TaMysql::GetData(string table, string key, Int_t index) const {
// Return key'd data at index in dtype vector.
   return "0";
};

Double_t TaMysql::GetData(dtype *d) const {
  if (d->GetType() == "i") return (Double_t)d->GetI();
  if (d->GetType() == "d") return d->GetD();
  if (d->GetType() == "s") return 0;
  return 0;
};

string TaMysql::GetRunType() const {
// Get run type, e.g. runtype = 'beam' 

  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  TString *anatype = db.GetDBChar("ana","pan","runtype",fRunIndex,&length); 
  return anatype->Data();
};

string TaMysql::GetAnaType() const {
// Get analysis type, e.g. anatype = 'beam' 

  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  TString *anatype = db.GetDBChar("ana","pan","anatype",fRunIndex,&length); 
  return anatype->Data();
};

Double_t TaMysql::GetDacNoise(const Int_t& adc, const Int_t& chan, const string& key) const {
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'

  if (chan < 0 ) return 0;
  if (adc > 9 ) return 0;
  if (adc < 0 ) return 0;

  static char cadc[10];  sprintf(cadc,"adc%d",adc);
  static char cchan[10];  sprintf(cchan,"chan%d",chan);

  TFloatBuffer Noise; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBFloatBuff("dacnoise",cadc,cchan,fRunIndex,&Noise);  
  
  if (TaString(key).CmpNoCase("slope") == 0) return Noise.Get(0);
  if (TaString(key).CmpNoCase("int") == 0 || TaString(key).CmpNoCase("intercept") == 0)
    {
      return Noise.Get(1);
    } 
  return 0;
};

Double_t TaMysql::GetPedestal(const Int_t& adc, const Int_t& chan) const{
// Get Pedestals for adc, chan 

  if (chan < 0 ) return 0;
  if (adc > 9 ) return 0;
  if (adc < 0 ) return 0;

  static char cadc[10];  sprintf(cadc,"adc%d",adc);
  static char cchan[10];  sprintf(cchan,"chan%d",chan);

  TIntBuffer Ped; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ped",cadc,cchan,fRunIndex,&Ped);  
  return Ped.Get(0);
};

UInt_t TaMysql::GetHeader(const string& device) const {
// Get Headers for decoding

  char *myc;
  myc = new char[strlen(device.c_str())];
  strcpy(myc,device.c_str());
  
  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  TString *Hdr = db.GetDBChar("header",myc,"hdr",fRunIndex,&length);
  cout << "HEADER: " << TaString(Hdr->Data()).Hex() << endl;
  return TaString(Hdr->Data()).Hex();
};

UInt_t TaMysql::GetMask(const string& device) const {
// Get Mask for decoding 

  char *myc;
  myc = new char[strlen(device.c_str())];
  strcpy(myc,device.c_str());
  
  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  TString *Mask = db.GetDBChar("header",myc,"mask",fRunIndex,&length);
  cout << "MASK: " << TaString(Mask->Data()).Hex() << endl;
  return TaString(Mask->Data()).Hex();
};

Double_t TaMysql::GetCutValue(const string& cutname) const {
// Get a cut value for 'value'.  e.g. value = 'lobeam', 'burpcut' 

  char *myc;
  myc = new char[strlen(cutname.c_str())];
  strcpy(myc,cutname.c_str());
  
  TIntBuffer Cut; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","pan",myc,fRunIndex,&Cut);  
  return Double_t(Cut.Get(0));
};

Int_t TaMysql::GetNumCuts() const {
// Get number of cuts "ncuts" in the database.
  
  TIntBuffer Ncut; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","cut","ncuts",fRunIndex,&Ncut);  
  return Ncut.Get(0);  
};

vector<Int_t> TaMysql::GetExtLo() const {
// Get cut extensions, low and high 

  TIntBuffer Extlo; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","cut","extlo",fRunIndex,&Extlo);  

  vector<Int_t> result;
  result.clear();

  Int_t ncuts = GetNumCuts();
  for (Int_t i=0;i<ncuts;i++){
    result.push_back(Extlo.Get(i));
  }

  return result;
};

vector<Int_t> TaMysql::GetExtHi() const {
// Get cut evhi event intervals 

  TIntBuffer Exthi; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","cut","exthi",fRunIndex,&Exthi);  

  vector<Int_t> result;
  result.clear();

  Int_t ncuts = GetNumCuts();
  for (Int_t i=0;i<ncuts;i++){
    result.push_back(Exthi.Get(i));
  }

  return result;
};

Int_t TaMysql::GetNumBadEv() const {
// Get number of bad event intervals

  TIntBuffer Extlo; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","badevt","extlo",fRunIndex,&Extlo);  
  return Extlo.GetSize();
};

map <Int_t, vector<Int_t> > TaMysql::GetCutValues() const {
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)
  
  TDBTools db = TDBTools("pandb"); 

  TIntBuffer Extlo; 
  db.GetDBIntBuff("ana","badevt","extlo",fRunIndex,&Extlo);  
  
  TIntBuffer Exthi; 
  db.GetDBIntBuff("ana","badevt","exthi",fRunIndex,&Exthi);  
  
  TIntBuffer Ncut; 
  db.GetDBIntBuff("ana","badevt","ncut",fRunIndex,&Ncut);  
  
  TIntBuffer State; 
  db.GetDBIntBuff("ana","badevt","state",fRunIndex,&State);  

  //

  map <Int_t, vector<Int_t> > result;
  vector<Int_t> temp;
  result.clear();
  temp.clear();
  Int_t NumBadEv = GetNumBadEv();

  for (Int_t k=0; k<NumBadEv; k++){  
    
    temp.push_back(Extlo.Get(k));
    temp.push_back(Exthi.Get(k));
    temp.push_back(Ncut.Get(k));
    temp.push_back(State.Get(k));
    
    result.insert(make_pair(k, temp));
  
  }
   return result; 
};

Int_t TaMysql::GetMaxEvents() const {
// returns number of events to process

  TIntBuffer Maxev; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","pan","maxevents",fRunIndex,&Maxev);  
  return Maxev.Get(0);
};

Int_t TaMysql::GetDelay() const {
// returns helicity delay (in windows)

  TIntBuffer Wind; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","pan","windelay",fRunIndex,&Wind);  
  return Wind.Get(0);
};

Int_t TaMysql::GetOverSamp() const {
// returns oversample factor
  TIntBuffer Oversamp; 
  TDBTools db = TDBTools("pandb"); 
  db.GetDBIntBuff("ana","pan","oversamp",fRunIndex,&Oversamp);  
  return Oversamp.Get(0);
};
 
string TaMysql::GetPairType()  const {
// returns pair type (pair or quad) for this run.

  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  TString *anatype = db.GetDBChar("ana","pan","pairtype",fRunIndex,&length); 
  return anatype->Data();
};

Double_t TaMysql::GetValue(const string& table) const {
// Return single value from table "table".  This assumes the data
// are in a pair  "table   value" where table is a unique string and
// value the single Double_t that belongs to it.
   return 0;
};

string TaMysql::GetString(const string& table) const {
// Return "string" from table "table".  This assumes the data
// are in a pair  "table  string" where table is unique and
// value the single string that belongs to it.
   return 0;
};

vector<Int_t> TaMysql::GetValueVector(const string& table) const {
   vector<Int_t> result;
   result.clear();
   return result;
};

void TaMysql::ClearAll() {
// Clear the database for this run
   ClearAll(runnum);
};

void TaMysql::ClearAll(Int_t run) { 
// Clear the databaase for run #run
};

void TaMysql::ClearTable(string table) {
// wipe clean a table for this run
  ClearTable(runnum, table);
};

void TaMysql::ClearTable(Int_t run, string table) {
// wipe clean a table for run #run
};

void TaMysql::PutData(string table, const vector<string>& key, 
		        const vector<Double_t>& data) {
// Generic pet method if you know the 'table' and 'key's you want.
};

void TaMysql::PutRunType(string) {
// Put run type, e.g. runtype = 'beam' (case insensitive)
};

void TaMysql::PutAnaType(string) {
// Put analysis type, e.g. anatype = 'beam' (case insensitive)
};

void TaMysql::PutDacNoise(Int_t adc, Int_t chan, string key, Double_t value) {
// Put Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
};

void TaMysql::PutPedestal(Int_t adc, Int_t chan, Double_t value) {
// Put Pedestals for adc, chan
};

void TaMysql::PutHeader(string device, Int_t header) {
// Put Headers for decoding (needed by datamap)
};

void TaMysql::PutDataMap(string table, vector<string> keys, vector<Int_t>& map) {
// Put datamap info 
};

void TaMysql::PutCutValue(string cutname, Double_t value) {
// Put a cut value for 'cutname'.  e.g. cutname = 'lobeam', 'burpcut' 
};

void TaMysql::PutNumCuts(Int_t numcuts) {
// Put number of cuts 
};

void TaMysql::PutExts(const vector<Int_t>& extlo, 
                        const vector<Int_t>& exthi) {
// Put cut extensions, low and high 
};

void TaMysql::PutNumBadEv(Int_t num_intervals) {
// Put number of bad event intervals
};

void TaMysql::PutCutValues
     (const map <Int_t, vector<Int_t> > & cuts) {
};

void TaMysql::InitDB() {
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
  tables.push_back("extlo");          //  11
  tables.push_back("exthi");          //  12
  tables.push_back("evint");         //  13
  tables.push_back("windelay");      //  14
  tables.push_back("oversamp");      //  15
  tables.push_back("pairtype");      //  16

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
    if (i == 11) {   // extlo
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 12) {  // exthi
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 13) {  // evint
      for (k = 0; k < 20; k++) columns.push_back(new dtype("i"));
    }
    if (i == 14) columns.push_back(new dtype("i"));  // windelay
    if (i == 15) columns.push_back(new dtype("i"));  // oversamp
    if (i == 16) columns.push_back(new dtype("s"));  // pairtype
    sipair.second = columns.size(); 
    colsize.insert(sipair);
    LoadTable(tables[i],columns);
  }
};

void TaMysql::LoadTable(string table, vector<dtype*> columns) {
// Load the database for a 'table'
// The first call will set up the structure but load empty data.
// The 2nd call (loadflg==1) must overwrite the empty data of 1st call.
// Subsequent calls load data.
  if  ( !didinit ) {
    cout << "TaMysql:: ERROR: cannot LoadTable before InitDB"<<endl;
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

void TaMysql::PrintDataBase() {
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

string TaMysql::FindTable(string table) {
// Return 'case insensitive' table name if table exists
  return " ";
};

void TaMysql::DataMapReStart() {
// To reset the datamap iterator.  Must be done before using
// NextDataMap() the first time.
  InitDataMap();
  dmapiter = datamap.begin();
};

Bool_t TaMysql::NextDataMap() {
  InitDataMap();
  if (firstiter) {
      firstiter = kFALSE;
      return kTRUE;
  } 
  dmapiter++;
  if (dmapiter != datamap.end()) return kTRUE;
  return kFALSE; 
};

string TaMysql::GetDataMapName() const {
  if ( !initdm ) {
    cout << "TaMysql:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  return dmiter->first;
};
  
string TaMysql::GetDataMapType() const {
  if ( !initdm ) {
    cout << "TaMysql:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  TaKeyMap keymap = dmiter->second;
  return keymap.GetType();
};
 
TaKeyMap TaMysql::GetKeyMap(string devicename) const {
  map< string, TaKeyMap >::const_iterator mapiter = datamap.find(devicename);
  TaKeyMap nothing;  nothing.LoadType("unknown");
  if (mapiter == datamap.end()) return nothing;
  return mapiter->second;
};

void TaMysql::InitDataMap() {
  static pair<string, vector<dtype*> > sdt;
  int adc, chan, evb;
  string key;
  if ( !didinit ) {
    cout << "TaMysql:: ERROR: Cannot init datamap without first init DB"<<endl;
    return;
  }
  if (initdm) return;  // already initialized
  datamap.clear();

  Int_t length;
  TDBTools db = TDBTools("pandb"); 
  db.PrintInfo();
  //  db.SetDebugMode(0x20);  

  TString *DM = db.GetDBChar("ana","pan","datamap",fRunIndex,&length); 

//
// Decode the DataMap:
//   

  Int_t nrows;
  const char *Myc = DM->Data();
  Int_t Len = strlen(Myc);
  
  string Str; 
  string Data[1000];
  Str = Myc[0];  
  
  Int_t K =0;
  for (Int_t i=1; i<Len; i++){
    if(Myc[i]=='+'){
      Str = "";
      K++;
    }
    else{
       Str += Myc[i];
       Data[K]= Str;
    }
  }

  nrows = K;

  for(Int_t i=0; i<=nrows; i++){
    
    char *myc;
    myc = new char[strlen(Data[i].c_str()+1)];
    strcpy(myc,Data[i].c_str()); 
    Int_t len = strlen(myc);
    
    string str; 
    string data[10];
    str = myc[0];  
     
    Int_t k =0;
    for (Int_t i=1; i<len; i++){
      if(myc[i]==','){
	str = "";
	k++;
      }
      else{
	str += myc[i];
	data[k]= str;
      }
    }
    
    vector<dtype*> datav;
    
    dtype *dat0 = new dtype("s"); 
    dat0->Load(data[0]);
    datav.push_back(dat0);
    
    dtype *dat1 = new dtype("s");
    dat1->Load(data[1]);
    datav.push_back(dat1);
    
    dtype *dat2 = new dtype("i");
    dat2->Load(data[2]);
    datav.push_back(dat2);
    
    dtype *dat3 = new dtype("i");
    dat3->Load(data[3]);
    datav.push_back(dat3);
    
    dtype *dat4 = new dtype("i");
    dat4->Load(data[4]);
    datav.push_back(dat4);

    cout << "k = " << k << endl;
    
    for (Int_t j=5; j<=k; j++){
      dtype *dat = new dtype("s");       
      dat->Load(data[j]);
      datav.push_back(dat);
    }
    
    string devname = datav[1]->GetS();    
    cout << "devname: " << datav[1]->GetS() <<endl;
    map<string, TaKeyMap >::iterator dm = datamap.find(devname);
    TaKeyMap keymap;
    if (dm != datamap.end()) keymap = dm->second;
    keymap.LoadType(datav[0]->GetS());
    cout << "keymap: " << datav[0]->GetS() <<endl;
    
    adc  = datav[2]->GetI();
    chan = datav[3]->GetI();
    evb  = datav[4]->GetI();
    cout << "adc: " << adc << " chan: " << chan << " evb: " << evb <<endl;
    
    int istart = 5;
    for (int k = istart; k < (long)datav.size(); k++) {
      if ( !datav[k]->IsLoaded() ) continue;
      key = datav[k]->GetS();
      cout << "Key: " << key <<endl;
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

void TaMysql::PrintDataMap() {
  cout << "\n\n--- Printout of datamap ---"<<endl;
  for (multimap<string, TaKeyMap >::const_iterator dmap = datamap.begin();
	 dmap != datamap.end() ;  dmap++) {
    string devname = dmap->first;
    cout << "\nDevice name "<<devname<<"  and associated keymaps: "<<endl;
    TaKeyMap keymap = dmap->second;
    keymap.Print();
  }
};

Bool_t TaMysql::SelfCheck() {
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
     cout << "TaMysql:: SelfCheck ERROR:  No tirdata defined in database"<<endl;
     return kFALSE;
   }
   stest = GetPairType();
   if (strlen(stest.c_str())-1 <= 0) {
      cout << "TaMysql:: SelfCheck ERROR:  No pair type defined in database"<<endl;
      //      return kFALSE;
   }
   itest = GetDelay();
   if (itest != 0 && itest != 8 ) {
     cout << "TaMysql:: SelfCheck ERROR:  'windelay' = " << itest << " outside range 0-8" <<endl;
     //      return kFALSE;
   }
   itest = GetOverSamp();
   if (itest <= 0 || itest > 12 ) {
     cout << "TaMysql:: SelfCheck ERROR:  'oversamp' = " << itest << " outside range 0-12" <<endl;
     //      return kFALSE;
   }
   stest = GetRunType();
   if (strlen(stest.c_str())-1 <= 0) {
      cout << "TaMysql:: SelfCheck WARNING:  No 'runtype' defined in database"<<endl;
   }
   return kTRUE;
};

