#ifndef PAN_TaString
#define PAN_TaString

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaString.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Derives from STL string; provides additional methods.  No
// additional storage is defined, so strings and TaStrings can be
// converted back and forth as needed; e.g. to convert a string to
// lowercase you can do something like
//
//      string mixedstring ("MixedCaseString");
//      TaString temp = mixedstring;
//      string lowerstring = temp.ToLower();
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include <string>
#include <vector>

typedef unsigned UInt_t;  // take out when done debugging!!!!!!


class TaString : public string
{

public:

  // Constructors/destructors/operators

  TaString () {};
  TaString (const string s): string(s) {};
  TaString (const char* c): string(c) {};
  virtual ~TaString() {};

  // Major functions

  int CmpNoCase (const string& s); // case insensitive compare
  vector<string> Split();   // split on whitespace
  UInt_t Hex();      // conversion to to unsigned interpreting as hex
  TaString ToLower (); // conversion to lower case
  TaString ToUpper (); // conversion to lower case

private:
  
#ifndef NODICT
  ClassDef(TaString, 0)   // Improved string class
#endif

};

#endif
