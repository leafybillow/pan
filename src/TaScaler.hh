#ifndef PAN__TaScaler
#define PAN__TaScaler
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaScaler.hh  (header file)
//           ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Scaler data.  
//
//////////////////////////////////////////////////////////////////////////


#include "VaDevice.hh"
#include <string>
#include <TObject.h>
#include <TTree.h>

class TaScaler : public VaDevice {
 
 public:

    TaScaler( string name );
    virtual ~TaScaler();
    TaScaler(const TaScaler& copy);
    TaScaler& operator=( const TaScaler& assign);  

    virtual void Init(const VaDataBase& db);
    virtual void Decode(const TaEvent& event);

  protected:



#ifdef DICT
ClassDef (TaScaler, 0)    // Scaler(s) for parity
#endif

}; 

#endif
