//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaLabelledQuantity.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    A class of quantities with associated labels, units, and flags.
//
////////////////////////////////////////////////////////////////////////

#include "TaLabelledQuantity.hh"

#ifndef NODICT
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
      fFlags = qty.fFlags;
    }
  return *this;
}

// Major functions

// Data access functions

// Non member functions

ostream& operator<< (ostream& s, const TaLabelledQuantity q)
{

  // Print a labelled quantity to the given output stream as
  // "name: value units".  (Flags are not printed.)

  return s << q.GetName() << ": " << q.GetVal() << " " << q.GetUnits();
}

