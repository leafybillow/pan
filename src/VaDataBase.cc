
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        VaDataBase.cc   (implementation)
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

//#define PUNT 1

#include "VaDataBase.hh"

#ifdef DICT
ClassImp(VaDataBase)
#endif

VaDataBase::VaDataBase() { }

VaDataBase::~VaDataBase() { }

// Utilites

string VaDataBase::stlow(const string sinput) const {
// Convert a string to all lower case.
  string::const_iterator p = sinput.begin();
  string result = "";
  while (p != sinput.end()) {
    result += tolower(*p);
    ++p;
  }
  return result;
}

bool VaDataBase::bstrstr(const string a, const string b) const {
// Case insensitive string match.  True if match.  False otherwise.
// If define PUNT, then it is case sensitive instead.

#ifdef PUNT
  if (strstr(a.c_str(), b.c_str()) != NULL) return true;
  return false;
#endif

  static char csmall[]="abcdefghijklmnopqrstuvwxyz0123456789";
  static char cbig[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static map<char, int> mapsmall, mapbig;
  static pair<char, int> apair;
  static map<char, int>::iterator asmall,abig,bsmall,bbig;
  static bool strfirst = true;
  if ( strfirst ) {
     strfirst = false;
     for (int i = 0; i < 36; i++) {
       apair.first = csmall[i];  apair.second = i;  mapsmall.insert(apair);
       apair.first = cbig[i];  apair.second = i;  mapbig.insert(apair);
     }
  }
  if ( a.length() != b.length() ) return false;
  for (int i = 0; i < (long)a.length(); i++) {
    asmall = mapsmall.find(a[i]);  abig = mapbig.find(a[i]);
    bsmall = mapsmall.find(b[i]);  bbig = mapbig.find(b[i]);
    if (asmall == mapsmall.end()  &&
        abig   == mapbig.end()   )    return false;
    if (bsmall == mapsmall.end()  && 
        bbig   == mapbig.end()   )    return false;
    if (asmall != mapsmall.end()) {
      if (asmall->second != bsmall->second &&
          asmall->second != bbig->second ) return false;
    }
    if (abig != mapbig.end()) {
      if (abig->second != bbig->second && 
          abig->second != bsmall->second) return false;
    }
  }
  return  true;
};

vector<string> VaDataBase::vsplit(const string s) {
// Split a string into whitespace-separated strings
  vector<string> ret;
  ret.clear();
  typedef string::size_type string_size;
  string_size i = 0;
  while ( i != s.size()) {
    while (i != s.size() && isspace(s[i])) ++i;
      string_size j = i;
      while (j != s.size() && !isspace(s[j])) ++j;
      if (i != j) {
         ret.push_back(s.substr(i, j-i));
         i = j;
      }
  }
  return ret;
};

UInt_t VaDataBase::str_to_base16(string str) const {
// Convert string to base 16 integer
  static bool hs16_first = true;
  static map<char, int> strmap;
  static pair<char, int> pci;
  static char chex[]="0123456789abcdef";
  static vector<int> numarray; 
  static int linesize = 12;
  if (hs16_first) {
    hs16_first = false;
    for (int i = 0; i < 16; i++) {
      pci.first = chex[i];  pci.second = i;  strmap.insert(pci);
    }
    numarray.reserve(linesize);
  }
  numarray.clear();
  for (string::const_iterator p = str.begin(); p != str.end(); p++) {
    map<char, int>::const_iterator pm = strmap.find(*p);     
    if (pm != strmap.end()) numarray.push_back(pm->second);
    if ((long)numarray.size() > linesize-1) break;
  }
  UInt_t result = 0;  UInt_t power = 1;
  for (vector<int>::reverse_iterator p = numarray.rbegin(); 
      p != numarray.rend(); p++) {
      result += *p * power;  power *= 16;
  }
  return result;
};

Double_t VaDataBase::GetData(const string& key) const{
  return 0;
};




