
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
#include "TaString.hh"

#ifdef DICT
ClassImp(VaDataBase)
#endif

VaDataBase::VaDataBase() { }

VaDataBase::~VaDataBase() { }

Double_t VaDataBase::GetData(const string& key) const{
  return 0;
};

void 
VaDataBase::Checkout()
{
  // Debug checkout of database

  cout << "\n\nCHECKOUT of DATABASE for Run " << GetRunNum() << endl;
  cout << "Run type  = " << GetRunType() << endl;
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
  map<Int_t, vector<Int_t> > evint = GetCutValues();
  Int_t k = 0;
  for (map<Int_t, vector<Int_t> >::iterator icut = evint.begin();
     icut != evint.end(); icut++) {
     vector<Int_t> cutint = icut->second;
     cout << "Cut interval " << dec << k++;
     cout << "  from event " << cutint[0] << " to " << cutint[1];
     cout << "  mask " << cutint[2] << "   veto " << cutint[3] << endl;
  }
}

Cut_t
VaDataBase::GetCutNumber (TaString s) const
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
