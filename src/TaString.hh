//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaString.hh  (header file)
//       ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Class for improved strings
//
//    Inherits from STL string, but with additional methods.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaString
#define PAN_TaString

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
  
#ifdef DICT
  ClassDef(TaString, 0);
#endif

};

#endif
