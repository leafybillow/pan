#ifndef PAN__TaTB
#define PAN__TaTB
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaTB.hh  (header file)
//           ^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Timing board data.  
//
//////////////////////////////////////////////////////////////////////////


#include "VaDevice.hh"
#include <string>
#include <TObject.h>
#include <TTree.h>

class TaTB : public VaDevice {
 
 public:

    TaTB( string name );
    virtual ~TaTB();
    TaTB(const TaTB& copy);
    TaTB& operator=( const TaTB& assign);  

    void Init(const VaDataBase& db);  // initialize Timing Board with database info
    void Decode(const TaEvent& event); // get data from database


#ifdef DICT
ClassDef (TaTB, 0)    // Timing board of parity DAQ.
#endif

}; 

#endif
