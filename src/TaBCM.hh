#ifndef PAN_TaBCM
#define PAN_TaBCM
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBCM.hh  (header file)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam current monitor data.  
//
//////////////////////////////////////////////////////////////////////////

#include "TaADC.hh"
#include <string>
#include <TObject.h>

class TTree;
class VaDataBase;
class TaEvent;

class TaBCM : public TaADC {
 
 public:


    TaBCM(string name);
    virtual ~TaBCM();
    TaBCM(const TaBCM& copy);
    TaBCM& operator=( const TaBCM& assign);  

    virtual void Init(const VaDataBase& db);
    virtual void Decode(const TaEvent& event);
 
  private:

    string fName;
    void Calibrate(const TaEvent& event);

#ifdef DICT
ClassDef (TaBCM, 0)  // A Beam Current Monitor
#endif

}; 

#endif





