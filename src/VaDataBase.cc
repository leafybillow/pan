
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




