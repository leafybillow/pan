#ifndef PAN__TaADC
#define PAN__TaADC
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaADC.hh  (header file)
//           ^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Data from a Princeton/Harvard ADC used by HAPPEX DAQ.
//
//////////////////////////////////////////////////////////////////////////

#include "VaDevice.hh"
#include <string>
#include <TObject.h>
#include <TTree.h>

class TaADC : public VaDevice {
 
 public:


    TaADC( string name );
    virtual ~TaADC();
    TaADC(const TaADC& copy);
    TaADC& operator=( const TaADC& assign);  

    Int_t GetADCNumber() const;
    string GetADCLabel()  const;
    Int_t GetChannel(string key) const;

    virtual void Init(const VaDataBase& db);
    virtual void Decode(const TaEvent& event);
    Bool_t IsADC() const;

  protected:

    Int_t fadcnumber;
    string fadclabel;
    virtual void Calibrate(const TaEvent& event);
    map<string, Int_t> pedestal, channels;
    map<string, Double_t> dacslope, dacint, corr_data;


#ifdef DICT
ClassDef (TaADC, 0)    // An ADC channel
#endif

}; 

#endif
