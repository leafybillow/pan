//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaLabelledQuantity.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    A class of quantities with associated labels and units.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaLabelledQuantity
#define PAN_TaLabelledQuantity

#include "TObject.h"
#include <string>

class TaLabelledQuantity
{

public:

  // Constructors/destructors/operators
  TaLabelledQuantity (const string& qn = "", 
		      const Double_t qv = 0.0, 
		      const string& qu = ""): 
    fName(qn), fVal(qv), fUnits(qu) {};
  TaLabelledQuantity (const TaLabelledQuantity& qty):
    fName(qty.fName), fVal(qty.fVal), fUnits(qty.fUnits) {};
  virtual ~TaLabelledQuantity() {}
  TaLabelledQuantity& operator=(const TaLabelledQuantity& qty);

  // Major functions

  // Data access functions
  void SetVal (const Double_t qv) { fVal = qv; }
  const string& GetName() const { return fName; }
  Double_t GetVal() const { return fVal;}
  const string& GetUnits() const { return fUnits; }

private:

  // Data members
  string fName;   // Name of the quantity
  Double_t fVal;  // Value of the quantity
  string fUnits;  // Units for the quantity

#ifdef DICT
  ClassDef(TaLabelledQuantity, 0)
#endif

};

// Non member functions

ostream& operator<< (ostream& s, const TaLabelledQuantity q);

#endif
