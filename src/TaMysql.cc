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
Int_t TaMysql::Connect(TSQLServer *db) {
  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer
  
  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  // Print server information
  printf("Server info: %s\n", db->ServerInfo());

  return 0;
}

Int_t TaMysql::DisConnect(TSQLServer *db) {
  delete db;
  db = NULL;
  
 return 0;
}

void TaMysql::Load(int run) {
// Load the database for this run.

 

    InitDB();

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

  //   TSQLServer *db;

   // Int_t Stat = Connect(db);

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;


  //
  
  Char_t *dbtablename = new Char_t[50];
  Char_t *column = new Char_t[50];
  
  dbtablename = "ana_pan_runtype_VALUE";
  column = "value_1"; 
  
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;
  
  // start timer
  //  TStopwatch timer;
  //  timer.Start();
  
  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;

  res = db->Query(sql);

  row = res->Next();

  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;
  delete db;
  db = NULL;
 
  //   DisConnect(db);

  // Process results
  //  Int_t nrows = res->GetRowCount();
  // cout << "Got " << nrows << " rows in result." << endl;

  // Int_t nfields = res->GetFieldCount();
  //cout << "Got " << nfields << " fields in result." << endl;

  //  row = res->Next();

  return row->GetField(0);

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

 
   return "unknown";
};

string TaMysql::GetAnaType() const {
// Get analysis type, e.g. anatype = 'beam' 


  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[50];

   dbtablename = "ana_pan_runtype_VALUE";
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

   return row->GetField(0);

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;


  ////////

   return "unknown";
};

Double_t TaMysql::GetDacNoise(const Int_t& adc, const Int_t& chan, const string& key) const {
// Get Dac noise parameters for adc,chan with key = 'slope' or 'intercept'

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   if (chan <= 0 ) return 0;
   if (adc > 9 ) return 0;
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   static char cadc[10];  sprintf(cadc,"%d",adc);
   static char cchan[10];  sprintf(cchan,"%d",chan);

   sprintf (dbtablename, "dacnoise_adc"); strcat(dbtablename, cadc);  strcat(dbtablename, "_chan"); 
   strcat(dbtablename, cchan); strcat(dbtablename, "_VALUE"); 

   //   cout <<  dbtablename << endl;
 
   column = "value_1, value_2"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //cout << " dacnoise adc " << adc << " chan " << chan << " Slope "<< atof(row->GetField(0))<< " Intercept "<< atof(row->GetField(1)) <<endl;

    if (TaString(key).CmpNoCase("slope") == 0) return atof(row->GetField(0));
    if (TaString(key).CmpNoCase("int") == 0 || TaString(key).CmpNoCase("intercept") == 0)
      {
      return atof(row->GetField(1));
      } 

    return 0;
  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);


};

Double_t TaMysql::GetPedestal(const Int_t& adc, const Int_t& chan) const{
// Get Pedestals for adc, chan 

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   if (chan <= 0 ) return 0;
   if (adc > 9 ) return 0;
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   static char cadc[10];  sprintf(cadc,"%d",adc);
   static char cchan[10];  sprintf(cchan,"%d",chan);

   sprintf (dbtablename, "ped_adc"); strcat(dbtablename, cadc);  strcat(dbtablename, "_chan"); 
   strcat(dbtablename, cchan); strcat(dbtablename, "_VALUE"); 

   //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  //cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //cout << " ped adc " << adc << " chan " << chan << " value "<< row->GetField(0) <<endl;

  return  atoi(row->GetField(0));

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

};

UInt_t TaMysql::GetHeader(const string& device) const {
// Get Headers for decoding
  //    string table = "header";
  //  return str_to_base16(GetData(table, device, 0));

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[50];

   char *myc;
   myc = new char[strlen(device.c_str())];
   strcpy(myc,device.c_str());

     sprintf (dbtablename, "header_"); strcat(dbtablename,myc); strcat(dbtablename, "_hdr_VALUE"); 

   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  //cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  //cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //cout << " hdr: " << str_to_base16(row->GetField(0)) << endl;

   return TaString(row->GetField(0)).Hex();


  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;



};

UInt_t TaMysql::GetMask(const string& device) const {
// Get Mask for decoding 
    string table = "header";
    //     cout << " mask control: " << str_to_base16(GetData(table, device, 1)) << endl;
//  return str_to_base16(GetData(table, device, 1));

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[50];

   char *myc;
   myc = new char[strlen(device.c_str())];
   strcpy(myc,device.c_str());

     sprintf (dbtablename, "header_"); strcat(dbtablename,myc); strcat(dbtablename, "_mask_VALUE"); 

   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  //cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  //cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //cout << " mask: " << str_to_base16(row->GetField(0)) << endl;

   return TaString(row->GetField(0)).Hex();



  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;




};

Double_t TaMysql::GetCutValue(const string& cutname) const {
// Get a cut value for 'value'.  e.g. value = 'lobeam', 'burpcut' 
  //   return GetValue(cutname);

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   char *myc;
   myc = new char[strlen(cutname.c_str())];
   strcpy(myc,cutname.c_str());

     sprintf (dbtablename, "ana_pan_"); strcat(dbtablename,myc); strcat(dbtablename, "_VALUE"); 

 
     //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //  cout << "Cut " << cutname <<" Value: "<< atof(row->GetField(0)) <<endl;

  return  atof(row->GetField(0));

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

};

Int_t TaMysql::GetNumCuts() const {
// Get number of cuts "ncuts" in the database.
  //   return (Int_t)GetValue("ncuts");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_cut_ncuts_VALUE"); 

   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  cout << "Number of cut= " << atoi(row->GetField(0)) << endl;

   return atoi(row->GetField(0));



  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;




};

vector<Int_t> TaMysql::GetEvLo() const {
// Get cut evlo event intervals 
  //     return GetValueVector("evlo");
  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[500];

   sprintf (dbtablename, "ana_cut_evlo_VALUE"); 

   column = "value_1, value_2, value_3, value_4, value_5, value_6, value_7, value_8, value_9, value_10,  value_11, value_12, value_13"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  cout << "Cut evlo = " << atoi(row->GetField(0)) << endl;

   vector<Int_t> result;
   result.clear();
   //   multimap<string, vector<dtype*> >::const_iterator lb =
   //          database.lower_bound(stlow(table));
   //   multimap<string, vector<dtype*> >::const_iterator ub = 
   //        database.upper_bound(stlow(table));
   //for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
   //      dmap != ub; dmap++) {
   //  vector<dtype*> datav = dmap->second;
   //  for (vector<dtype*>::const_iterator idat = datav.begin();
   //       idat != datav.end(); idat++) {
   //    if ((*idat)->GetType() == "i") 
   //        result.push_back((*idat)->GetI());
   //  }
   // }

   for (Int_t i=0;i<=12;i++){
       result.push_back(atoi(row->GetField(i)));
   }

   return result;


  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;



};

vector<Int_t> TaMysql::GetEvHi() const {
// Get cut evhi event intervals 
  //   return GetValueVector("evhi");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[500];

   sprintf (dbtablename, "ana_cut_evhi_VALUE"); 


   column = "value_1, value_2, value_3, value_4, value_5, value_6, value_7, value_8, value_9, value_10,  value_11, value_12, value_13"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  cout << "Cut evhi = " << atoi(row->GetField(0)) << endl;

   vector<Int_t> result;
   result.clear();
   //   multimap<string, vector<dtype*> >::const_iterator lb =
   //          database.lower_bound(stlow(table));
   //   multimap<string, vector<dtype*> >::const_iterator ub = 
   //        database.upper_bound(stlow(table));
   //for (multimap<string, vector<dtype*> >::const_iterator dmap = lb;
   //      dmap != ub; dmap++) {
   //  vector<dtype*> datav = dmap->second;
   //  for (vector<dtype*>::const_iterator idat = datav.begin();
   //       idat != datav.end(); idat++) {
   //    if ((*idat)->GetType() == "i") 
   //        result.push_back((*idat)->GetI());
   //  }
   // }

   for (Int_t i=0;i<=12;i++){
       result.push_back(atoi(row->GetField(i)));
   }

   return result;


  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;


};

Int_t TaMysql::GetNumBadEv() const {
// Get number of bad event intervals
  //   return database.count("evint");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename = new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_badevt_evlo_VALUE"); 

   column = "value_1, value_2, value_3, value_4, value_5"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  cout << "Number of bad event intervals= " << nfields << endl;

   return nfields;



  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;




};

map <Int_t, vector<Int_t> > TaMysql::GetCutValues() const {
// For bad event intervals, get formatted results 
// First element of map goes from 0 to GetNumBadEv(), second is a vector of
// results in prescribed order: (evlo, evhi, cut num, cut value)


  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //

   Char_t *dbtablename1 = new Char_t[50]; Char_t *dbtablename2 = new Char_t[50];
   Char_t *dbtablename3 = new Char_t[50]; Char_t *dbtablename4 = new Char_t[50];
   Char_t *column = new Char_t[500];

   sprintf (dbtablename1, "ana_badevt_evhi_VALUE"); 
   sprintf (dbtablename2, "ana_badevt_evlo_VALUE"); 
   sprintf (dbtablename3, "ana_badevt_ncut_VALUE"); 
   sprintf (dbtablename4, "ana_badevt_state_VALUE"); 


   column = "value_1, value_2, value_3, value_4, value_5"; 
   
  // Construct query
  Char_t *sql1 = new Char_t[4200];Char_t *sql2 = new Char_t[4200];
  Char_t *sql3 = new Char_t[4200];Char_t *sql4 = new Char_t[4200];
  sprintf(sql1, "SELECT %s FROM %s", column, dbtablename1);
  sprintf(sql2, "SELECT %s FROM %s", column, dbtablename2);
  sprintf(sql3, "SELECT %s FROM %s", column, dbtablename3);
  sprintf(sql4, "SELECT %s FROM %s", column, dbtablename4);


  //  cout << sql1 << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row1;TSQLRow *row2;TSQLRow *row3;TSQLRow *row4;
  TSQLResult *res1; TSQLResult *res2; TSQLResult *res3; TSQLResult *res4;
  res1 = db->Query(sql1);res2 = db->Query(sql2);res3 = db->Query(sql3);res4 = db->Query(sql4);
  delete db;
  db = NULL;

  // Process results
  Int_t nrows = res1->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  Int_t nfields = res1->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row1 = res1->Next();row2 = res2->Next();row3 = res3->Next();row4 = res4->Next();

  //  cout << "Badevt evhi = " << atoi(row1->GetField(0)) << endl;


  map <Int_t, vector<Int_t> > result;
   vector<Int_t> temp;
   result.clear();
   temp.clear();
  for (Int_t k=0;k<=4;k++){  

  temp.push_back(atoi(row1->GetField(0)));
  temp.push_back(atoi(row2->GetField(0)));
  temp.push_back(atoi(row3->GetField(0)));
  temp.push_back(atoi(row4->GetField(0)));

  result.insert(make_pair(k, temp));
  
  }

   return result; 

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql1;
  delete []column;
  delete []dbtablename1;

   
};





Int_t TaMysql::GetMaxEvents() const {
// returns number of events to process
  //   return (Int_t)GetValue("maxevents");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];


   sprintf (dbtablename, "ana_pan_maxevents_VALUE"); 

   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

cout << " Maximum Events: "<< atof(row->GetField(0)) <<endl;

  return  atoi(row->GetField(0));

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;


};

Int_t TaMysql::GetDelay() const {
// returns helicity delay (in windows)
  //   return (Int_t)GetValue("windelay");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_pan_windelay_VALUE"); 

 
     //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //  cout << "Cut " << cutname <<" Value: "<< atof(row->GetField(0)) <<endl;

  return  atoi(row->GetField(0));

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

};

Int_t TaMysql::GetOverSamp() const {
// returns oversample factor
  //   return (Int_t)GetValue("oversamp");

  const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_pan_oversamp_VALUE"); 

 
     //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  //  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  cout << "Oversample = " << atoi(row->GetField(0)) << endl;

  return  atoi(row->GetField(0));

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

};
 
string TaMysql::GetPairType()  const {
// returns pair type (pair or quad) for this run.
  //   return GetString("pairtype");
 const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_pan_pairtype_VALUE"); 

 
     //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
  //  cout << "Got " << nrows << " rows in result." << endl;

  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
  //  cout << "Got " << nfields << " fields in result." << endl;

  row = res->Next();

  //  cout << "Pair Type: " <<  row->GetField(0) << endl;

  return  row->GetField(0);

  // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  delete []sql;
  delete []column;
  delete []dbtablename;

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

void TaMysql::PutCuts(const vector<Int_t>& evlo, 
                        const vector<Int_t>& evhi) {
// Put cut evlo, evhi event intervals 
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
  tables.push_back("evlo");          //  11
  tables.push_back("evhi");          //  12
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





////////////////////////////////////




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
  string table = "datamap";

 const Char_t *dbusername = "dbmanager";  // username for DB access
  const Char_t *dbpasswd = "parity";    // password for DB access
  const Char_t *dbname = "pandb";      // name of DB
  const Char_t *dbhostname = "alquds.jlab.org";  // hostname of DB server computer

  // Connect to MySQL server
  Char_t *dburl = new Char_t[50];
  sprintf(dburl,"mysql://%s/%s", dbhostname, dbname);
  TSQLServer *db = TSQLServer::Connect(dburl, dbusername, dbpasswd);
  delete []dburl;
  dburl=NULL;

  //
   
   Char_t *dbtablename =  new Char_t[50];
   Char_t *column = new Char_t[50];

   sprintf (dbtablename, "ana_pan_datamap_VALUE"); 

 
     //   cout <<  dbtablename << endl;
 
   column = "value_1"; 
   
  // Construct query
  Char_t *sql = new Char_t[4200];
  sprintf(sql, "SELECT %s FROM %s", column, dbtablename);
  //  cout << sql << endl;

  // start timer
  //  TStopwatch timer;
  //  timer.Start();

  // Submit query to server
  TSQLRow *row;
  TSQLResult *res;
  res = db->Query(sql);

  delete db;
  db = NULL;
  // Process results
  Int_t nrows = res->GetRowCount();
    cout << "Got " << nrows << " rows in result." << endl;

    //  if (nrows <= 0 ) return 0;



  Int_t nfields = res->GetFieldCount();
    cout << "Got " << nfields << " fields in result." << endl;

  for(Int_t i=1; i<=nrows; i++){
    row = res->Next();
    cout << "datamap: " <<  row->GetField(0) << endl;
  

   const char *myc = row->GetField(0);

   cout << "myc: " <<  myc << endl;

   Int_t len = strlen(myc);

   cout << "len: " <<  len << endl;
   
   string str; string data[10];
      str = myc[0];  
     
      //    cout << "str: " <<  str << endl; 


   
     Int_t k =0;
   for (Int_t i=1; i<len; i++){
     if(myc[i]==','){
       //       cout << "str: " <<  str << endl;  
       str = "";
       k++;
     }
     else{
       str += myc[i];
       data[k]= str;
     }
   }
   //       cout << "str: " <<  str << endl; 


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

   //   char *myc;
   // myc = new char[strlen(data[j].c_str())];
   //strcpy(myc,data[j].c_str());

   //   cout << "myccc: " <<  myc << endl; 
    
   dtype *dat = new dtype("s");
 
   dat->Load(data[j]);
   datav.push_back(dat);

 }





 //  for (Int_t j=0; j<=k; j++){
 // cout << "data: " <<  data[j] << endl; 
 //  datav.push_back((dtype*)data[j]); 
 //  }

 //} 
  
   
   // stop timer and print results
  //  timer.Stop();
  //  Double_t rtime = timer.RealTime();
  //  Double_t ctime = timer.CpuTime();

  //  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // Clean up
  //  delete []sql;
  // delete []column;
  // delete []dbtablename;

   
 /*
     multimap<string, vector<dtype*> >::iterator lb =
         database.lower_bound(table);        
	   multimap<string, vector<dtype*> >::iterator ub = 
        database.upper_bound(table);
   for (multimap<string, vector<dtype*> >::iterator dmap = lb;
	       dmap != ub; dmap++) {
  
 
    
        vector<dtype*> datav =  dmap->second;

 */
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





//////////////////////////////////






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

