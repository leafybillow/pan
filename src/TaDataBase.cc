//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaAnalysis.cc  (implementation)
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
//          /doc/DATABASE.DOC
//
/////////////////////////////////////////////////////////////////////

// Turn off MYSQL.  Hopefully it will be turned on ... soon.
#define TURN_OFF_MYSQL

#include "TaDataBase.hh"
#include "TaString.hh"

#ifdef DICT
ClassImp(TaDataBase)
ClassImp(TaRootRep)
#endif

TaDataBase::TaDataBase() {
     didinit   = kFALSE; 
     didread   = kFALSE; 
     didput    = kFALSE;
     initdm    = kFALSE;
     usemysql  = kFALSE;
     useroot   = kFALSE;
     usectrl   = kFALSE;
     firstiter = kTRUE;
     fFirstgdn = new Bool_t(kTRUE);
     fFirstAdcPed = new Bool_t(kTRUE);
     fFirstScalPed = new Bool_t(kTRUE);
     nbadev = 0;
     rootdb = new TaRootRep();
     dacparam = new Double_t[2*MAXADC*MAXCHAN];     
     memset(dacparam,0,2*MAXADC*MAXCHAN*sizeof(Double_t));
     adcped = new Double_t[MAXADC*MAXCHAN];
     memset(adcped,0,MAXADC*MAXCHAN*sizeof(Double_t));
     scalped = new Double_t[MAXSCAL*MAXSCALCHAN];
     memset(scalped,0,MAXSCAL*MAXSCALCHAN*sizeof(Double_t));
 }

TaDataBase::~TaDataBase() {
     delete [] dacparam;
     delete [] adcped;
     delete [] scalped;
     delete rootdb;
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

void TaDataBase::Read(int run, const vector<string>& dbcomm) {
// Load the database for this run which is for example
//     ./db/parity03_1441.db for run = 1441
// The flag 'usectrl' can be set via the '-D control.db' command
// line option; this forces us to use 'control.db' as the database.
// See also the comments in ChkDbCommand()
  InitDB();
  runnum = run;
  dbcommand = dbcomm;
  if (ChkDbCommand() == 0) return;  // ERROR
  if (useroot) return;  // All done, db was read from ROOT.
  rootdb->Clear();
  TaFileName dbFileName ("db");
  ifstream *dbfile = new ifstream(dbFileName.String().c_str());
  if ( ! (*dbfile) || usectrl ) {
    if ( !usectrl ) {
      cerr << "TaDataBase::Load WARNING: run file " << dbFileName.String()
	   << " does not exist" << endl;
    }
    dbFileName = TaFileName ("dbdef");
    dbfile = new ifstream(dbFileName.String().c_str());
    if ( ! (*dbfile) ) {
      cerr << "TaDataBase::Load WARNING: no file "
	   << dbFileName.String() <<endl;
      cerr << "You need a database to run.  Ask an expert."<<endl;
      return;
    }
    cerr << "TaDataBase::Load: Using " << dbFileName.String() << " as default database. (May be ok.)"<<endl;
  }

  clog << "TaDataBase::Load: Database loading from " 
       << dbFileName.String() << endl;
    
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
  didread = kTRUE;
// Command-line over-ride
  SetDbCommand();
  ToRoot();
};

Int_t TaDataBase::ChkDbCommand() {
// Command line over-ride of database
// Logic does not allow infinite combinations, NOTE:
//  *  If  '-D control.db' we read from control.db.  Other 
//     '-D' command subsequently over-ride table(s).
//  *  If  '-D useroot filename' we read database from ROOT
//     file 'filename' and ignore all other commands.
//  *  If  '-D mysql' we read database from MYSQL. Other '-D'
//     commands subsequently over-ride table(s).
//  *  It is an error to specify both mysql and useroot. If
//     you try that, you'll get no data at all (ERROR).
//  *  '-D table values'  will over-write the table with those
//     values, provided the line has proper syntax.  NOTE:  The 
//     table is dropped; therefore, you cannot add to a table,
//     instead you must add all lines of a table when you start one.
//     Normally this is ok for trivial tables like 'lobeam'.
  usemysql = kFALSE;
  useroot = kFALSE;
  string rootfile;
  if (dbcommand.size() == 0) return 1;
  for (int i = 0; i < (long)dbcommand.size(); i++) {
    if (strcasecmp(dbcommand[i].c_str(),"mysql") == 0) 
          usemysql = kTRUE;
    if (strcasecmp(dbcommand[i].c_str(),"control.db") == 0) 
          usectrl = kTRUE;
    if (strcasecmp(dbcommand[i].c_str(),"useroot") == 0) {
      useroot = kTRUE;
      if (i+1<(long)dbcommand.size()) rootfile = dbcommand[i+1];
    }
  }
  if (usemysql && useroot) {
    cout << "ERROR: TaDataBase::SetDbCommand: Cannot use both"<<endl;
    cout << " MYSQL and ROOT file as database input. Choose ONE !!"<<endl;
    return 0;
  }
  if (usemysql) Mysql("read");
  if (useroot) {
    ReadRoot(rootfile);
    ToRoot();
  }
  return 1;
}  

void TaDataBase::SetDbCommand() {
// Use the '-D' database command to over-ride table(s) 
// of database.  See comments for ChkDbCommand()
  if (dbcommand.size() == 0) return;
  for (int i = 0; i < (long)dbcommand.size(); i++) {
    multimap<string, vector<dtype*> >::iterator dmap = 
         database.find(FindTable(dbcommand[i]));
    if (dmap == database.end()) continue; 
    vector<dtype*> dv = dmap->second;
    vector<dtype*> datavect;  datavect.clear();
    int j = i+1;  int k = 0;
    while (j < (long)dbcommand.size() && k < (long)dv.size()) {
       multimap<string, vector<dtype*> >::iterator dnext = 
          database.find(FindTable(dbcommand[j]));
       if (dnext != database.end()) break;  // found next table
       dtype *dat = new dtype(dv[k]->GetType()); 
       dat->Load(dbcommand[j]);   
       datavect.push_back(dat);
       j++;  k++;
    }
    PutData(dbcommand[i], datavect);
  }
};

void 
TaDataBase::ReadRoot(string filename) {
// Load the database from a ROOT file 'filename'.
// This is done with '-D useroot filename' command line option.
// This choice, if chosen, takes precedence, i.e. all other source
// of data including command line is therefore ignored.
  didread = kFALSE;
  TFile *rfile = new TFile(filename.c_str());
  TaRootRep *rdb = (TaRootRep*)rfile->Get("TaRootRep;1");
  if ( !rdb ) {
    cout << "ERROR: TaDataBase::ReadRoot:";
    cout << " TaRootRep object not found !!"<<endl;
    cout << " Database was *NOT* read from ROOT file "<<filename<<endl;
    cout << " This means that either: "<<endl;
    cout << "   1. The root file you specified does not exist."<<endl;
    cout << "   2. The root file does not contain the TaRootRep object."<<endl;
    return;
  }
  cout << "Loading database from ROOT file "<<filename<<endl;
  string comment = "#";
  InitDB();
  vector<string> strvect;
  TaString sinput,sline;
  while (rdb->NextLine()) {
     strvect.clear();  
     sinput = TaString(rdb->GetLine());
//     cout << sinput;
     strvect = sinput.Split();
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
  didread = kTRUE;
};

void TaDataBase::Print() {
// Human-readable printout for end-of-run summary
// This tells the user what variables were used for this run.
// Print() should be called BEFORE any Put() method is used
// because it shows the state of database at start of analysis.
  int i,j;
  clog << endl; 
  for (i = 0; i < 50; i++) clog << "-"; 
  clog << endl;
  clog << "Database Variables Used for This Run " << endl;
  clog << "   Analysis type :  " << GetAnaType() << endl;
  clog << "   Max events :  " << GetMaxEvents() << endl;
  clog << "   Window delay :  " << GetDelay() << endl;
  clog << "   Oversample :   " << GetOverSamp() << endl;
  clog << "   Pair type :  " << GetPairType() << endl;
  vector<string> cutnames = GetCutNames();
  clog << "   Number of cuts used :  " << GetNumCuts()<<endl;
  for (i = 0; i < GetNumCuts(); i++) {
   clog << "     Cut name  " << cutnames[i] <<endl;
  }
  clog << "   Low beam cut :  " << GetCutValue("lobeam") << endl;
  clog << "   Burp cut :  " << GetCutValue("burpcut") << endl;
  vector<Int_t> extlo = GetExtLo();
  vector<Int_t> exthi = GetExtHi();
  Int_t nlo = extlo.size();
  Int_t nhi = exthi.size();
  Int_t n = nlo;
  if (nlo != nhi) {
    clog << "ERROR: Unequal number of low and high intervals"<<endl;
    if (nhi < nlo) n = nhi;
  }
  Int_t ncutext = 0;
  for (i = 0; i < n; i++) {
    if (extlo[i] != 0 || exthi[i] != 0) ncutext++;
  }
  clog << "   Num of (non-zero) cut extensions :  " << ncutext << endl;
  if (n != 0) clog << "    cut     extlo     exthi"<<endl;  
  for (i = 0; i < ncutext; i++) {
    clog << "      " << i << "      " << extlo[i];
    clog << "       " << exthi[i] << endl;
  }
  clog << "   Number of cut intervals :  "<<GetNumBadEv()<<endl;
  map <Int_t, vector<Int_t> > cutint = GetCutInt();
  n = GetNumBadEv();
  if (GetNumBadEv() > (long)cutint.size()) {
    clog << "ERROR: Contradictory number of cut intervals"<<endl;
    n = cutint.size();
  }
  if (n != 0) 
   clog << "   interval    evlo    evhi   cut num   cut value"<<endl;
  for (i = 0; i < n; i++) {
    clog << "     " << i << "   ";
    vector<Int_t> cuts = cutint[i];
    int nc = cuts.size();
    if (nc > 4) nc = 4;
    for (j = 0; j < nc; j++) {
      clog << "     "<<cuts[j];
    }
    clog << endl;
  }
  for (i = 0; i < 50; i++) clog << "-"; 
  clog << endl;
};

Bool_t TaDataBase::SelfCheck() {
// Returns kTRUE if the database makes good sense.  
// This enforces some rules.
   string stest;
   Int_t itest;
   vector<string> keys;
   TaKeyMap keymap = datamap["tir"];  // must have tir data in datamap
   keys = keymap.GetKeys();
   Bool_t allok = kTRUE;
   Int_t tirok = 0;
   for (vector<string>::iterator ikey = keys.begin(); ikey != keys.end(); ikey++) {
     string key = *ikey;
     if (key == "tirdata") tirok = 1;
   }
   if ( !tirok ) {
     cerr << "DataBase:: SelfCheck ERROR:  No tirdata defined in database"<<endl;
     allok = kFALSE;
   }
   stest = GetPairType();
   if (strlen(stest.c_str())-1 <= 0) {
      cerr << "DataBase:: SelfCheck ERROR:  No pair type defined in database"<<endl;
      allok = kFALSE;
   }
   itest = GetDelay();
   if (itest != 0 && itest != 8 ) {
     cerr << "DataBase:: WARNING:  windelay = "<<itest<<" seems wrong."<<endl;
   }
   if (itest < 0 || itest > 8 ) {
     cerr << "DataBase:: SelfCheck ERROR:  'windelay' = " << itest << " outside range 0-8" <<endl;
     allok = kFALSE;
   }
   itest = GetOverSamp();
   if (itest <= 0 || itest > 12 ) {
     cerr << "DataBase:: SelfCheck ERROR:  'oversamp' = " << itest << " outside range 0-12" <<endl;
     allok = kFALSE;
   }
   stest = GetAnaType();
   if (strlen(stest.c_str())-1 <= 0) {
      cerr << "DataBase:: SelfCheck WARNING:  No 'anatype' defined in database"<<endl;
      allok = kFALSE;
   }
   return allok;
};

void 
TaDataBase::Checkout()
{
 // Thorough debug checkout of database.  Ok, this is
 // partly redundant with Print()
  cout << "\n\nCHECKOUT of DATABASE for Run " << GetRunNum() << endl;
  cout << "Max events = " << GetMaxEvents() << endl;
  cout << "lobeam  cut = " << GetCutValue("LOBEAM") << endl;
  cout << "burpcut  cut = " << GetCutValue("BURPCUT") << endl;
  cout << "window delay = " << GetDelay() << endl;
  cout << "oversampling factor = " << GetOverSamp() << endl;
  cout << "pair type (i.e. pair or quad) =  " << GetPairType() << endl;
  cout << "\n\nPedestal and Dac noise parameters by ADC# and channel# : " << endl;
  for (int adc = 0; adc < 10 ; adc++) {
    cout << "\n\n-----  For ADC " << adc << endl;
    for (int chan = 0; chan < 4; chan++) {
      cout << "\n  channel " << chan;
      cout << "   ped = " << GetAdcPed(adc,chan);
      cout << "   dac slope = " << GetDacNoise(adc,chan,"slope");
      cout << "   dac int = " << GetDacNoise(adc,chan,"int");
    }
  }  
  cout << "\n\nPedestal parameters for a few scalers : " << endl;
  for (int scal = 0; scal < 2 ; scal++) {
    cout << "\n\n-----  For Scaler " << scal << endl;
    for (int chan = 0; chan < 8; chan++) {
      cout << "\n  channel " << chan;
      cout << "   ped = " << GetScalPed(scal,chan);
    }
  }  
  cout << "\n\nNumber of cuts " << GetNumCuts() << endl;
  vector<Int_t> extlo = GetExtLo();
  vector<Int_t> exthi = GetExtHi();
  for (int i = 0; i < GetNumCuts(); i++) { 
    if (i >= (long)exthi.size()) cout << "extlo and exthi mismatched size" << endl;
    cout << "Cut " << i << "   Extlo  = " << extlo[i] << "  Exthi = " << exthi[i] << endl;
  }  
  cout << "\n\nNum cut event intervals " << GetNumBadEv() << endl;
  map<Int_t, vector<Int_t> > evint = GetCutInt();
  Int_t k = 0;
  for (map<Int_t, vector<Int_t> >::iterator icut = evint.begin();
     icut != evint.end(); icut++) {
     vector<Int_t> cutint = icut->second;
     cout << "Cut interval " << dec << k++;
     cout << "  from event " << cutint[0] << " to " << cutint[1];
     cout << "  mask " << cutint[2] << "   veto " << cutint[3] << endl;
  }
}

void TaDataBase::PrintDataBase() {
  // Technical printout of entire database
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
      cout << " Value = ";
      if (dat->GetType() == "i") cout << dat->GetI();
      if (dat->GetType() == "d") cout << dat->GetD();
      if (dat->GetType() == "s") cout << dat->GetS();
    }
  }
};

void TaDataBase::PrintDataMap() {
  // Technical printout of data map.
  cout << "\n\n--- Printout of datamap ---"<<endl;
  for (multimap<string, TaKeyMap >::const_iterator dmap = datamap.begin();
	 dmap != datamap.end() ;  dmap++) {
    string devname = dmap->first;
    cout << "\nDevice name "<<devname<<"  and associated keymaps: "<<endl;
    TaKeyMap keymap = dmap->second;
    keymap.Print();
  }
};

string TaDataBase::GetAnaType() const {
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

string TaDataBase::GetFdbkSwitch( const string &fdbktype )const {
  // get the feedback switch state corresponding to feedback type fdbktype.
  string table = "feedback";
  if ( database.count("feedback") > 1 ){
    clog<<" feedback"<<fdbktype<<" switched "
        <<GetData(table,fdbktype, 0)<<endl;
    return GetData(table,fdbktype, 0);
  }
  else{
    return "";
  }     
}; 

Int_t TaDataBase::GetFdbkTimeScale( const string &fdbktype ) const{
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
   cerr<<"DataBase::GetFdbkTimeScale( const string &fdbktype ) ERROR\n"
	<<"TABLE not declared correctly, need feedback table. "
	<<"Example of line format in .db file:\n"
        <<" feedback    AQ   on   3  bcm1 "<<endl;
   return 0;
 }
};
   
string TaDataBase::GetFdbkMonitor( const string &fdbktype ) const{
  // get the timescale of feedback type fdbktype
  string table = "feedback";
 if ( database.count("feedback") >1){ 
     multimap<string, vector< dtype* > >::const_iterator lb = database.lower_bound(table);        
     multimap<string, vector< dtype* > >::const_iterator ub = database.upper_bound(table);        
     for ( multimap<string, vector< dtype* > >::const_iterator dmap = lb; dmap != ub; dmap++){
       vector< dtype* > datatype = dmap->second;
       if (datatype[0]->GetType() == "s"){
	 if ( TaString(datatype[0]->GetS()).CmpNoCase(fdbktype) == 0 ){
	   if (datatype[3]->GetType() == "s") {
             return datatype[3]->GetS();
	   }
	 }
       }
     }
     return 0;
 }
 else{
   cerr<<"DataBase::GetFdbkMonitor( const string &fdbktype ) ERROR\n"
	<<"TABLE not declared correctly, need feedback table. "
	<<"Example of line format in .db file:\n"
        <<" feedback    AQ   on   3  bcm1 "<<endl;
   return 0;
 }
};   

void TaDataBase::Mysql(string action) {
// Read or Write the database from Mysql server.
// If reading, the ASCII database will be overwritten, so it is a 
// good idea to keep backups.
// If writing, it is assumed the ASCII database was already written.
// Syntax of the Mysql script command is:
//     $MYSQL_SCRIPT_DIR/mysql.pl  action  dbfile  run
// where
//   $MYSQL_SCRIPT_DIR = environment variable pointing to script
//   mysql.pl = Perl script that does all the work.
//   action = "read" or "write"
//   dbfile = name of ASCII database file to read or write
//   run = run number

#ifdef TURN_OFF_MYSQL
  
  cout << "Mysql script is not yet ready.  No Mysql I/O."<<endl;

#else

  // The rest of this code should work right away when the
  // Mysql Perl script is ready.

  if (action != "read" && action != "write") {
    cerr << "ERROR: Mysql:  Action must be either read or write"<<endl;
    cerr << "Attempted to use bogus action = "<<action<<endl;
    return;
  }
  rootdb->Clear();
  TaFileName dbFileName ("db");
  char mysql_command[100],srun[20];
  char *script_dir = getenv("MYSQL_SCRIPT_DIR");
  if (script_dir != NULL) strcpy(mysql_command, script_dir);
  sprintf(mysql_command,"mysql.pl ");
  strcat(mysql_command, action.c_str());
  strcat(mysql_command, " ");
  strcat(mysql_command, dbFileName.String().c_str());
  sprintf(srun,"  %d",runnum);
  strcat(mysql_command, srun);
  cout << "Executing MYSQL script command : ";
  cout << mysql_command << endl << endl;
  int systat, retstat, err;
  systat = system(mysql_command);  // Execute the Perl script.
// Parse the system status and return status from script.
  err = 1;
  if (WIFEXITED(systat) != 0) {
    retstat = WEXITSTATUS(systat);
    switch (retstat) {
      case 0:
	cout << "Mysql script succesful."<<endl;
        err = 0;
        break;
      case 1:
// FIXME: We must decide what these errors from MYSQL script mean.
	cout << "Mysql::ERROR: 1"<<endl;
        break;
      case 2:
	cout << "Mysql::ERROR: 2"<<endl;
        break;
      case 3:
	cout << "Mysql::ERROR: 3"<<endl;
        break;
      default:
        cout << "Mysql:: ERROR: Abnormal system status !!"<<endl;
        cout << "Possibly the script does not exist or has a typo ?"<<endl;
        cout << "Try to execute the script by hand.  Type at shell:"<<endl;
        cout << "    " << mysql_command << endl<<endl;
    }
  } else {
// In the following, one could figure out what is wrong by 'man waitpid'
// and use the macros explained there to parse 'systat' but I guess this 
// will be a very rare failure, so I leave the complaint simple.
     cout << "TaDataBase::Mysql:: ERROR: ";
     cout << "  Abnormal system status !!"<<endl;
     cout << "Possibilities include: "<<endl;
     cout << "  process killed, stopped, core dumped, etc."<<endl;
  }
  if (err) {
    cout << "TaDataBase::Mysql: ERROR initializing database"<<endl;
    return;
  }
#endif
}

void 
TaDataBase::Write() {
// Write the database for this run (if data was "Put").
// Output goes to both an ASCII file and to MYSQL.
// This method should be called AFTER all Put() methods are used.
  if ( !didput ) return;
  ToRoot();
  TaFileName dbFileName ("db");
  ofstream *ofile = new ofstream(dbFileName.String().c_str());
  TaString soutput;
  while (rootdb->NextLine()) {
     soutput = TaString(rootdb->GetLine());
     int linesize = strlen(soutput.c_str());   
     ofile->write ( soutput.c_str(), linesize );
  }
  Mysql("write");
};


Cut_t
TaDataBase::GetCutNumber (TaString s) const
{
  // Return cut number corresponding to a given name (case insensitive match). 
  // If no match, return fNumCuts.

  UInt_t numcuts = (UInt_t) GetNumCuts();
  vector<string> cutnames = GetCutNames();
  Cut_t i;
  for (i = 0; i < numcuts && s.CmpNoCase(cutnames[i]) != 0; ++i)
    {} // null loop body
  return i;
}

Double_t TaDataBase::GetData(const string& key) const {
// FIXME -- this needs to be written
// Generic Get method, works if the key is unique
   Double_t dummy = 0;
   return dummy; 
};

vector<Double_t> TaDataBase::GetData(string table, vector<string> keys) const {
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

string TaDataBase::GetData(string table, string key, Int_t index) const {
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

Double_t TaDataBase::GetData(dtype *d) const {
  if (d->GetType() == "i") return (Double_t)d->GetI();
  if (d->GetType() == "d") return d->GetD();
  if (d->GetType() == "s") return 0;
  return 0;
};

Double_t TaDataBase::GetDacNoise(const Int_t& adc, const Int_t& chan, const string& key) const {
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'
  if (!didinit) {
      cerr << "DataBase::GetDacNoise ERROR: Database not initialized\n";
      return 0;
  }
  int idx;
  if (*fFirstgdn)  {
     *fFirstgdn = kFALSE;
     if (!didread) {
         cerr << "DataBase::GetDacNoise:: WARNING: ";
         cerr << "Did not read any database yet\n";
     }
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
     cerr << "WARNING: DataBase::GetDacNoise:";
     cerr << "  illegal combination of adc and channel #"<<endl;
     return 0;
      }
  return 0;
};

Double_t TaDataBase::GetAdcPed(const Int_t& adc, const Int_t& chan) const {
// Get Pedestals for adc, chan 
  if (!didinit) {
      cerr << "DataBase::GetAdcPed ERROR: Database not initialized\n";
      return 0;
  }
  int idx;
  if (*fFirstAdcPed) {
    *fFirstAdcPed = kFALSE;
    if (!didread) {
         cerr << "DataBase::GetAdcPed:: WARNING: ";
         cerr << "Did not read any database yet\n";
    }
    vector<string> keys;
    keys.clear();
    keys.push_back("adc");   // this must match a string in database
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
           adcped[idx] = data[index+2];
         }
      }
      index += keys.size();
    }
  }
  idx = adc*MAXCHAN+chan;
  if (idx >=0 && idx < MAXADC*MAXCHAN) {
     return adcped[idx];
  } else {
     cerr << "WARNING: DataBase::GetAdcPed:";
     cerr << "  illegal combination of adc and channel #"<<endl;
     return 0;
  }
};

Double_t TaDataBase::GetScalPed(const Int_t& scal, const Int_t& chan) const {
// Get Pedestals for scaler, chan 
  if (!didinit) {
      cerr << "DataBase::GetScalPed ERROR: Database not initialized\n";
      return 0;
  }
  int idx;
  if (*fFirstScalPed) {
    *fFirstScalPed = kFALSE;
    if (!didread) {
         cerr << "DataBase::GetScalPed:: WARNING: ";
         cerr << "Did not read any database yet\n";
    }
    vector<string> keys;
    keys.clear();
    keys.push_back("scaler");   // this must match a string in the database
    keys.push_back("chan");
    keys.push_back("value");
    vector<Double_t> data = GetData("ped",keys);
    int iscal,ichan;
    int index = 0;
    while (index < (long)data.size()) {
      if ((index+(long)keys.size()-1) < (long)data.size()) {
        iscal = (int)data[index];
        ichan= (int)data[index+1];
        idx = iscal*MAXSCALCHAN+ichan;
        if (idx < MAXSCAL*MAXSCALCHAN) {
           scalped[idx] = data[index+2];
         }
      }
      index += keys.size();
    }
  }
  idx = scal*MAXSCALCHAN+chan;
  if (idx >=0 && idx < MAXSCAL*MAXSCALCHAN) {
     return scalped[idx];
  } else {
     cerr << "WARNING: DataBase::GetScalPed:";
     cerr << "  illegal combination of scaler and channel #"<<endl;
     return 0;
  }
};

UInt_t TaDataBase::GetHeader(const string& device) const {
// Get Headers for decoding
   string table = "header";
   return TaString(GetData(table, device, 0)).Hex();
};

UInt_t TaDataBase::GetMask(const string& device) const {
// Get Mask for decoding 
   string table = "header";
   return TaString(GetData(table, device, 1)).Hex();
};

Double_t TaDataBase::GetCutValue(const string& cutname) const {
// Get a cut value for 'value'.  e.g. value = 'lobeam', 'burpcut' 
   return GetValue(cutname);
};

Int_t TaDataBase::GetNumCuts() const {
// Get number of cuts "ncuts" in the database.
   return (Int_t)GetValue("ncuts");
};

vector<string> 
TaDataBase::GetCutNames () const 
{
  vector<string> result;
  result.clear();
  multimap<string, vector<dtype*> >::const_iterator lb =
    database.lower_bound(TaString("cutnames").ToLower());
  multimap<string, vector<dtype*> >::const_iterator ub = 
    database.upper_bound(TaString("cutnames").ToLower());
  for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
       dmap != ub; dmap++) {
    vector<dtype*> datav = dmap->second;
    for (vector<dtype*>::const_iterator idat = datav.begin();
	 idat != datav.end(); idat++) {
      if ((*idat)->GetType() == "s") 
	result.push_back((*idat)->GetS());
    }
  }
  return result;
};

vector<Int_t> TaDataBase::GetExtLo() const {
// Get cut extensions, low and high 
   return GetValueVector("extlo");
};

vector<Int_t> TaDataBase::GetExtHi() const {
// Get cut evhi event intervals 
   return GetValueVector("exthi");
};

Int_t TaDataBase::GetNumBadEv() const{
// Get number of bad event intervals
   return nbadev;
};

map <Int_t, vector<Int_t> > TaDataBase::GetCutInt() const {
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

Int_t TaDataBase::GetMaxEvents() const {
// returns number of events to process
   return (Int_t)GetValue("maxevents");
};

Int_t TaDataBase::GetDelay() const {
// returns helicity delay (in windows)
   return (Int_t)GetValue("windelay");
};

Int_t TaDataBase::GetOverSamp() const {
// returns oversample factor
   return (Int_t)GetValue("oversamp");
};
 
string TaDataBase::GetPairType()  const {
// returns pair type (pair or quad) for this run.
   return GetString("pairtype");
};

Double_t TaDataBase::GetValue(const string& table) const {
// Return single value from table "table".  This assumes the data
// are in a pair  "table   value" where table is a unique string and
// value the single Double_t that belongs to it.
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound(TaString(table).ToLower());
   if (dmap == database.end() || 
       database.count(TaString(table).ToLower()) == 0) {
     cerr << "ERROR: DataBase: Unknown database table "<<table<<endl;
     return 0;
   }
   if (database.count(TaString(table).ToLower()) > 1) {
     cerr << "ERROR: DataBase: Mulitply defined table "<<table<<endl;
     cerr << "Fix the database to have one instance."<<endl;
     return 0;
   }
   vector<dtype*> datatype = dmap->second;
   if (datatype[0]->GetType() == "i") return (Double_t)datatype[0]->GetI(); 
   if (datatype[0]->GetType() == "d") return datatype[0]->GetD(); 
   cerr << "ERROR: DataBase: Illegal data type for table "<<table<<endl;
   cerr << "Must be an integer or double."<<endl;
   return 0;
};

string TaDataBase::GetString(const string& table) const {
// Return "string" from table "table".  This assumes the data
// are in a pair  "table  string" where table is unique and
// value the single string that belongs to it.
   static multimap<string, vector<dtype*> >::const_iterator dmap;
   dmap = database.lower_bound(TaString(table).ToLower());
   if (dmap == database.end() || 
       database.count(TaString(table).ToLower()) == 0) {
     cerr << "ERROR: DataBase: Unknown database table "<<table<<endl;
     return 0;
   }
   if (database.count(TaString(table).ToLower()) > 1) {
     cerr << "ERROR: DataBase: Mulitply defined table "<<table<<endl;
     cerr << "Fix the database to have one instance."<<endl;
     return 0;
   }
   vector<dtype*> datatype = dmap->second;
   if (datatype[0]->GetType() == "s") return datatype[0]->GetS(); 
   cerr << "ERROR: DataBase: Illegal data type for table "<<table<<endl;
   cerr << "Must be a string."<<endl;
   return 0;
};

vector<Int_t> 
TaDataBase::GetValueVector(const string& table) const {
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

void TaDataBase::PutDacNoise(const Int_t& adc, const Int_t& chan, const Double_t& slope, const Double_t& intcpt) {
// Put Dac noise parameters slope and intercept for adc,chan 
  if (!didinit) {
      cerr << "DataBase::PutDacNoise ERROR: Database not initialized\n";
  }
  string table = "dacnoise";
  vector<dtype *> dvec;   dvec.clear();
  dtype *dat;
  dat = new dtype("s");  dat->Load("adc");   dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(adc);     dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("chan");  dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(chan);    dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("slope"); dvec.push_back(dat);
  dat = new dtype("d");  dat->Load(slope);   dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("int");   dvec.push_back(dat);
  dat = new dtype("d");  dat->Load(intcpt);  dvec.push_back(dat);
  didput = kTRUE;
  PutData(table, dvec);
};

void TaDataBase::PutAdcPed(const Int_t& adc, const Int_t& chan, const Double_t& ped) {
// Put Pedestals for adc, chan 
  if (!didinit) {
      cerr << "DataBase::PutDacNoise ERROR: Database not initialized\n";
  }
  string table = "ped";
  vector<dtype *> dvec;   dvec.clear();
  dtype *dat;
  dat = new dtype("s");  dat->Load("adc");   dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(adc);     dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("chan");  dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(chan);    dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("value"); dvec.push_back(dat);
  dat = new dtype("d");  dat->Load(ped);     dvec.push_back(dat);
  didput = kTRUE;
  PutData(table, dvec);
};

void TaDataBase::PutScalPed(const Int_t& scal, const Int_t& chan, const Double_t& ped) {
// Put Pedestals for scaler, chan 
  if (!didinit) {
      cerr << "DataBase::PutDacNoise ERROR: Database not initialized\n";
  }
  string table = "ped";
  vector<dtype *> dvec;   dvec.clear();
  dtype *dat;
  dat = new dtype("s");  dat->Load("scaler"); dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(scal);     dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("chan");   dvec.push_back(dat);
  dat = new dtype("i");  dat->Load(chan);     dvec.push_back(dat);
  dat = new dtype("s");  dat->Load("value");  dvec.push_back(dat);
  dat = new dtype("d");  dat->Load(ped);      dvec.push_back(dat);
  didput = kTRUE;
  PutData(table, dvec);
};

void TaDataBase::PutCutInt(const vector<Int_t> myevint) {
// Put a single bad event interval into the database.
// The user must call this for each cut interval.
// The vector 'myevint' is a prescribed order: 
//   (evlo, evhi, cut num, cut value)
    string table  = "evint";
    vector<dtype*> dvect;  dvect.clear();
    for (int i = 0; i < (long)myevint.size(); i++) {
       dtype *dat = new dtype("i");
       dat->Load(myevint[i]);
       dvect.push_back(dat);
    } 
    didput = kTRUE;
    PutData(table, dvect);
}

void TaDataBase::PutData(string table, vector<dtype *> dvect) {
// To put data into the database.
// PRIVATE method used by the public "Put" methods.
// WARNINGS: PutData() will REPLACE the table.  
// And, no, you do not want to set 'didput' here.
// Performance is poor but its ok because you will not call this often.
  multimap<string, vector<dtype*> >::iterator dmap = 
          database.find(FindTable(table));
  if (dmap == database.end()) return;
  if (dvect.size() == 0) return;
  multimap<string, vector<dtype*> > mapcopy = database;
  database.clear();
  database.insert(make_pair(table, dvect));
  for (multimap<string, vector<dtype*> >::iterator im = mapcopy.begin();
       im != mapcopy.end(); im++) {
       string sname = im->first;
       vector<dtype*> vdata = im->second;
       if (dbput[table] == kTRUE || table != sname) {
          database.insert(make_pair(sname, vdata));
       }
  }
  if (dbput[table] == kFALSE) dbput[table] = kTRUE;
}

void TaDataBase::InitDB() {
// Define the database structure.
// Table names correspond to first column in Ascii DB.
// Careful, an error here can lead to incomprehensible core dump.

  if (didinit) return;
  didinit = kTRUE;

  vector <dtype*> columns;
  columns.reserve(20);

  tables.clear();
  tables.push_back("run");           //   0
  tables.push_back("anatype");       //   1
  tables.push_back("maxevents");     //   2
  tables.push_back("lobeam");        //   3
  tables.push_back("burpcut");       //   4
  tables.push_back("dacnoise");      //   5
  tables.push_back("ped");           //   6
  tables.push_back("header");        //   7
  tables.push_back("datamap");       //   8
  tables.push_back("ncuts");         //   9
  tables.push_back("extlo");         //  10
  tables.push_back("exthi");         //  11
  tables.push_back("evint");         //  12
  tables.push_back("windelay");      //  13
  tables.push_back("oversamp");      //  14
  tables.push_back("pairtype");      //  15
  tables.push_back("feedback");      //  16 
  tables.push_back("IAparam");       //  17
  tables.push_back("PZTparam");      //  18
  tables.push_back("cutnames");      //  19

  pair<string, int> sipair;
  int k;

  for (int i = 0; i < (long)tables.size(); i++ ) { 
    sipair.first = tables[i];
    sipair.second = 0;
    dbinit.insert(sipair);
    columns.clear();
    if (i == 0) columns.push_back(new dtype("i"));  // run
    if (i == 1) columns.push_back(new dtype("s"));  // anatype
    if (i == 2) columns.push_back(new dtype("i"));  // maxevents
    if (i == 3) columns.push_back(new dtype("d"));  // lobeam
    if (i == 4) columns.push_back(new dtype("d"));  // burpcut
    if (i == 5) {  // dac noise
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 6) {  // pedestals
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("i"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 7) { // header
       for (int k = 0; k < 3; k++) columns.push_back(new dtype("s"));
    }
    if (i == 8) { // datamap
       for (k = 0; k < 3; k++) columns.push_back(new dtype("s"));
       for (k = 0; k < 3; k++) columns.push_back(new dtype("i"));
       for (k = 0; k < 10; k++) columns.push_back(new dtype("s"));
    }
    if (i == 9) columns.push_back(new dtype("i"));  // ncuts
    if (i == 10) {   // extlo
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 11) {  // exthi
      for (k = 0; k < 40; k++) columns.push_back(new dtype("i"));
    }
    if (i == 12) {  // evint
      for (k = 0; k < 20; k++) columns.push_back(new dtype("i"));
    }
    if (i == 13) columns.push_back(new dtype("i"));  // windelay
    if (i == 14) columns.push_back(new dtype("i"));  // oversamp
    if (i == 15) columns.push_back(new dtype("s"));  // pairtype
    if (i == 16) {
      columns.push_back(new dtype("s"));
      columns.push_back(new dtype("s"));
      columns.push_back(new dtype("i"));
      columns.push_back(new dtype("s"));
    }
    if (i == 17) {   // IA slope 
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 18) {   // PZT matrix 
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
       columns.push_back(new dtype("s"));
       columns.push_back(new dtype("d"));
    }
    if (i == 19) {   // cutnames
      for (k = 0; k < 40; k++) columns.push_back(new dtype("s"));
    }
    sipair.second = columns.size(); 
    colsize.insert(sipair);
    LoadTable(tables[i],columns);
  }
};

void TaDataBase::LoadTable(string table, vector<dtype*> columns) {
// Load the database for a 'table'
// The first call will set up the structure but load empty data.
// The 2nd call (loadflg==1) must overwrite the empty data of 1st call.
// Subsequent calls load data.
  if  ( !didinit ) {
    cerr << "DataBase:: ERROR: cannot LoadTable before InitDB"<<endl;
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
     database.insert(make_pair(table, columns));
     dbput.insert(make_pair(table, kFALSE));
  }
  loadflg++;
  im->second = loadflg;
  if (dbinit["evint"] <= 1) {
       nbadev = 0;
  } else {
       nbadev = database.count("evint");
  }
//  PrintDataBase();   // DEBUG
};


string TaDataBase::FindTable(string table) {
// Return 'case insensitive' table name if table exists
  for (int i = 0; i < (long)tables.size(); i++) {
    if ( TaString(table).CmpNoCase(tables[i]) == 0 ) return tables[i];
  }
  return " ";
};

void 
TaDataBase::ToRoot() {
// Copy database to ROOT representation.  
  rootdb->Clear();
  char cval[40];
  string sline,sval;
  for (multimap<string, vector<dtype*> >::iterator im = database.begin();
    im != database.end(); im++) {
    string sname = im->first;
    vector<dtype*> vdata = im->second;
    sline = sname;
    for (vector<dtype*>::iterator iv = vdata.begin(); 
     iv != vdata.end(); iv++) {
       dtype *dtp = *iv;
       if (dtp->GetType() == "s") {
         sprintf(cval,"  %s  ",(dtp->GetS()).c_str());
       }
       if (dtp->GetType() == "i") {
         sprintf(cval,"  %d  ",dtp->GetI());
       }
       if (dtp->GetType() == "d") {
         sprintf(cval,"  %7.2f  ",(float)dtp->GetD());
       }
       sval = cval;
       sline += sval;
    }
    sline += " \n";
    rootdb->Put(sline.c_str());
  }
};

void TaDataBase::DataMapReStart() {
// To reset the datamap iterator.  Must be done before using
// NextDataMap() the first time.
  firstiter = kTRUE;
  InitDataMap();
  dmapiter = datamap.begin();
};

Bool_t TaDataBase::NextDataMap() {
  InitDataMap();
  if (firstiter) {
      firstiter = kFALSE;
      return kTRUE;
  } 
  dmapiter++;
  if (dmapiter != datamap.end()) return kTRUE;
  return kFALSE; 
};

string TaDataBase::GetDataMapName() const {
  if ( !initdm ) {
    cerr << "DataBase:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  return dmiter->first;
};
  
string TaDataBase::GetDataMapType() const {
  if ( !initdm ) {
    cerr << "DataBase:: ERROR: must first InitDataMap before using it"<<endl;
    string nothing = "";
    return nothing;
  }
  map< string , TaKeyMap >::const_iterator dmiter; 
  dmiter = dmapiter;
  TaKeyMap keymap = dmiter->second;
  return keymap.GetType();
};
 
TaKeyMap TaDataBase::GetKeyMap(string devicename) const {
  map< string, TaKeyMap >::const_iterator mapiter = datamap.find(devicename);
  TaKeyMap nothing;  nothing.LoadType("unknown");
  if (mapiter == datamap.end()) return nothing;
  return mapiter->second;
};

void TaDataBase::InitDataMap() {
  static pair<string, vector<dtype*> > sdt;
  int devnum, chan, evb;
  string key,readout;
  if ( !didinit ) {
    cerr << "DataBase:: ERROR: Cannot init datamap without first init DB"<<endl;
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
     readout = datav[2]->GetS();
     devnum  = datav[3]->GetI();
     chan = datav[4]->GetI();
     evb  = datav[5]->GetI();
     int istart = 6;
     for (int k = istart; k < (long)datav.size(); k++) {
       if ( !datav[k]->IsLoaded() ) continue;
       key = datav[k]->GetS();
       keymap.LoadData( key, readout, devnum, chan + k - istart, evb + k - istart);
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
// Debug printouts 
// PrintDataBase();
// PrintDataMap();   
};


