//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaLabelledQuantity.cc  (implementation)
//        ^^^^^^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    A class of quantities with associated labels and units.
//
//////////////////////////////////////////////////////////////////////////

#include "TaLabelledQuantity.hh"

#ifdef DICT
ClassImp(TaLabelledQuantity)
#endif

// Constructors/destructors/operators

TaLabelledQuantity& TaLabelledQuantity::operator=(const TaLabelledQuantity& qty)
{
  if ( &qty != this )
    {
      fName = qty.fName;
      fVal = qty.fVal;
      fUnits = qty.fUnits;
    }
  return *this;
}

// Major functions

// Data access functions

// Non member functions

ostream& operator<< (ostream& s, const TaLabelledQuantity q)
{
  return s << q.GetName() << ": " << q.GetVal() << " " << q.GetUnits();
}

