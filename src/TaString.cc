//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaString.cc  (implementation)
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

#include "TaString.hh"
#include <strstream>

#ifndef NODICT
ClassImp(TaString)
#endif

// Constructors/destructors/operators



// Major functions

int 
TaString::CmpNoCase (const string& s)
{
  // Case insensitive compare.  Returns -1 if "less", 0 if equal, 1 if
  // "greater".

  string::const_iterator p = this->begin();
  string::const_iterator p2 = s.begin();

  while (p != this->end() && p2 != s.end())
    {
      if (toupper(*p) != toupper(*p2))
	return (toupper(*p) < toupper(*p2)) ? -1 : 1;
      ++p;
      ++p2;
    }

  return (s.size() == this->size()) ? 0 : (this->size() < s.size()) ? -1 : 1;
}

vector<string> 
TaString::Split()
{
  // Split on whitespace.
  istrstream ist(this->c_str());
  string w;
  vector<string> v;

  while (ist >> w)
    v.push_back(w);
  return v;
}


UInt_t 
TaString::Hex()
{
  // Conversion to to unsigned interpreting as hex.
  istrstream ist(this->c_str());
  UInt_t in;
  ist >> hex >> in;
  return in;
}

TaString 
TaString::ToLower ()
{
  // Conversion to lower case.
  TaString::const_iterator p = this->begin();
  TaString result("");
  while (p != this->end()) 
    {
      result += tolower(*p);
      ++p;
    }
  return result;
}

TaString 
TaString::ToUpper ()
{
 // Conversion to lower case.
  TaString::const_iterator p = this->begin();
  TaString result("");
  while (p != this->end()) 
    {
      result += toupper(*p);
      ++p;
    }
  return result;
}

// Private member functions



// Non member functions

