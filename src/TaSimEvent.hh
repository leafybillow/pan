#ifndef PAN_TaSimEvent
#define PAN_TaSimEvent

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaSimEvent.hh  (interface)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    An event of data, probably munged up by some varity of simulation!
//
////////////////////////////////////////////////////////////////////////

#define TASIMEVENT_VERBOSE 1

#include "VaEvent.hh"
#include "Rtypes.h"
#include "PanTypes.hh"
#include "DevTypes.hh"
#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <utility>
#include "TRandom.h"

//#define FAKEHEL
#ifdef FAKEHEL
#include <iostream>
#include <fstream>
#endif


class TRandom;
class TaDevice;
class TTree;
class TaCutList;
class TaLabelledQuantity;
class TaRun;
class TaDataBase;

class TaSimEvent : public VaEvent {

public:

  // Constructors/destructors/operators
  TaSimEvent();
  virtual ~TaSimEvent();
  TaSimEvent(const TaSimEvent &ev);
  TaSimEvent& operator=(const TaSimEvent &ev);
  TaSimEvent& CopyInPlace (const TaSimEvent& rhs);

  // Major functions
  ErrCode_t RunInit(const TaRun& run);    // initialization at start of run
  void Decode( TaDevice& devices );             // decode the event 

 // Data access functions

private:

  // Private methods
  void Create(const TaSimEvent&);
  void SetSimConstants();

  // Static members
#ifdef FAKEHEL
  static ifstream fgHelfile;   // fake helicity data file
#endif


  TRandom fgR;      // Random number object
  Double_t fDetVsBcm[4];
  Double_t fDetNoiseR[4];
  // modify position by dithering slope for 12x, 4a x,y and 4b x,y given value
  //  and ident of dithering object
  Double_t fBpm4AXvCoil[7];   Double_t fBpm4AYvCoil[7];
  Double_t fBpm4BXvCoil[7];   Double_t fBpm4BYvCoil[7];
  Double_t fBpm12XvCoil[7];
  // modify detector by position
  Double_t fDetVsBpm12X[4];  
  Double_t fDetVsBpm4AX[4];  Double_t fDetVsBpm4BX[4];
  Double_t fDetVsBpm4AY[4];  Double_t fDetVsBpm4BY[4];
  Double_t fDet12Xoff;  
  Double_t fDet4AXoff; Double_t fDet4BXoff;
  Double_t fDet4AYoff; Double_t fDet4BYoff;

  // Data members
  
#ifndef NODICT
ClassDef(TaSimEvent,0)  // An event containing simulated data
#endif

};

#endif


