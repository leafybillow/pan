#ifndef PAN_TaFileName
#define PAN_TaFileName

//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaFileName.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Provides constructors to generate Pan-standard filenames.
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include <string>
#include "PanTypes.hh"

class TaFileName
{

public:

  // Constructors/destructors/operators

  TaFileName () {};
  TaFileName (const string s, 
	      const string c = "", 
	      const string suf = "");
  TaFileName (const TaFileName&);
  virtual ~TaFileName() {};
  TaFileName& operator= (const TaFileName&);

  // Major functions

  static void Setup (RunNumber_t r, string a);

  // Access functions

  const string& String() { return fFileName; }

private:
  
  // static data members

  static string fgAnaStr;
  static string fgBaseName;

  // Data members

  string fFileName;

#ifdef DICT
  ClassDef(TaFileName, 0)   // Improved string class
#endif

};

string GetEnvOrDef (string e, const string d);

#endif
