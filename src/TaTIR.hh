#ifndef PAN__TaTIR
#define PAN__TaTIR
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaTIR.hh  (header file)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Trigger Interrupt Register data.  Contains bits from an
//    I/O register, such as 'helicity', 'realtime', etc.
//
//////////////////////////////////////////////////////////////////////////


#include "VaDevice.hh"
#include <string>
#include <TObject.h>
#include <TTree.h>

class TaTIR : public VaDevice {
 
 public:

    TaTIR( string name );
    virtual ~TaTIR();
    TaTIR(const TaTIR& copy);
    TaTIR& operator=( const TaTIR& assign );  

    void Init(const VaDataBase& db);  // initialize TIR with database info
    void Decode(const TaEvent& event); // get data from database
  private:
    void ExtractSignal( const TaEvent& event ); // extract mtrigg, hel, ps from tirdata      

#ifdef DICT
ClassDef (TaTIR, 0)    // TIR of parity DAQ.
#endif

}; 

#endif



