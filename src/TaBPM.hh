#ifndef PAN_TaBPM
#define PAN_TaBPM
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBPM.hh  (header file)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam position monitor data.  
//
//////////////////////////////////////////////////////////////////////////

#include "TaADC.hh"
#include <string>

class TTree;
class VaDataBase;
class TaEvent;

class TaBPM : public TaADC {
 
 public:

    TaBPM(string name);
    virtual ~TaBPM();
    TaBPM(const TaBPM& copy);
    TaBPM& operator=( const TaBPM& assign);  

    virtual void Init(const VaDataBase& db);
    virtual void Decode(const TaEvent& event);
 
  private:
 
    void Calibrate(const TaEvent& event);
    Double_t Rotate(Double_t x, Double_t y, Int_t xy);
    Double_t kappa,x,y;
    string xkey,ykey;

#ifdef DICT
ClassDef (TaBPM, 0)  // A Beam Position Monitor
#endif

}; 

#endif
