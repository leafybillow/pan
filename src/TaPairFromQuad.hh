#ifndef PAN_TaPairFromQuad
#define PAN_TaPairFromQuad
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPairFromQuad.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Class which makes and analyzes opposite helicity event pairs
//    from a data stream structured as helicity window pairs.  Derived
//    from VaPair.
//
//////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "PanTypes.hh"
#include "VaPair.hh"

class TaCutList;
class TaEvent;
class TaRun;

class TaPairFromQuad : public VaPair {
  
public:
  
  // Constructors/destructors/operators
  TaPairFromQuad();
  TaPairFromQuad(const TaPairFromQuad& copy);
  TaPairFromQuad& operator=( const TaPairFromQuad& assign);  
  
  ~TaPairFromQuad();
  
  // Major functions
  ErrCode_t RunInit(const TaRun&);
  
private:
  
  // Private member functions
  void CheckSequence (TaEvent&, TaRun&); // look for helicity/synch errors
  UInt_t RanBit (UInt_t hRead = 2);

  // Data members
  static Int_t   fgQuadCount; // Quad window counter

#ifdef DICT
  ClassDef(TaPairFromQuad, 0)  // Event pair from window pair helicity structure
#endif
};

#endif
