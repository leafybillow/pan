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

#include "TaString.hh"
#include <strstream>

#ifdef DICT
ClassImp(TaString)
#endif

// Constructors/destructors/operators



// Major functions

int 
TaString::CmpNoCase (const string& s)
{
  // case insensitive compare
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
  // split on whitespace
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
  // conversion to to unsigned interpreting as hex
  istrstream ist(this->c_str());
  UInt_t in;
  ist >> hex >> in;
  return in;
}

TaString 
TaString::ToLower ()
{
  // conversion to lower case
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
 // conversion to lower case
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

