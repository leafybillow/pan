//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       TaFileName.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Provides constructors to generate Pan-standard filenames.
//
////////////////////////////////////////////////////////////////////////

#include "TaFileName.hh"
#include <strstream>
#include <ctime>

#ifdef DICT
ClassImp(TaFileName)
#endif

string TaFileName::fgAnaStr = "";
string TaFileName::fgBaseName = "";

// Constructors/destructors/operators

TaFileName::TaFileName (const string s, 
			const string com = "", 
			const string suf = "")
{
  string path (".");
  string tags ("");
  string suffix ("");
  string base = fgBaseName;

  if (fgBaseName == "")
    Setup (0, "");

  if (s == "coda")
    {
      suffix = GetEnvOrDef ("PAN_CODA_FILE_SUFFIX", "dat");
      path = GetEnvOrDef ("PAN_CODA_FILE_PATH", ".");
    }
  else if (s == "db")
    {
      suffix = GetEnvOrDef ("PAN_DB_FILE_SUFFIX", "db");
      path = GetEnvOrDef ("PAN_DB_FILE_PATH", ".");
    }
  else if (s == "dbdef")
    {
      suffix = GetEnvOrDef ("PAN_DB_FILE_SUFFIX", "db");
      path = GetEnvOrDef ("PAN_DB_FILE_PATH", ".");
      base = "control";
    }
  else if (s == "root")
    {
      suffix = GetEnvOrDef ("PAN_ROOT_FILE_SUFFIX", "root");
      path = GetEnvOrDef ("PAN_ROOT_FILE_PATH", ".");
    }
  else if (s == "output")
    {
      suffix = suf;
      path = GetEnvOrDef ("PAN_OUTPUT_FILE_PATH", ".");
    }
  else
    {
      clog << "TaFileName::TaFileName ERROR: Unknown filename type " << s << endl;
      fFileName = "ERROR";
      return;
    }

  if (s == "root" || s == "output")
    {
      if (fgAnaStr != "")
	tags += string ("_") + fgAnaStr;
      if (com != "")
	tags += string ("_") + com;
    }

  fFileName = path + string("/") + base + tags + string(".") + suffix;
}

TaFileName::TaFileName (const TaFileName& fn)
{
  fFileName = fn.fFileName;
}

TaFileName&
TaFileName::operator= (const TaFileName& fn)
{
  if (this != &fn)
    fFileName = fn.fFileName;
  return *this;
}

// Major functions

void 
TaFileName::Setup (RunNumber_t r, string a)
{
  string runstr ("xxxx");
  if (r != 0)
    {
      char temp[20];
      sprintf (temp, "%4.4d", r);
      runstr = temp;
    }
  string prefix (GetEnvOrDef ("PAN_FILE_PREFIX", ""));
  if (prefix == "")
    {
      time_t t;
      string ystr;
      if (time (&t) == time_t (-1))
	{
	  clog << "TaFileName::Setup ERROR: Can't get time" << endl;
	  ystr = "xx";
	}
      else
	{
	  tm* lt = gmtime(&t);
	  char yc[2];
	  sprintf (yc, "%2.2d", (lt->tm_year) % 10);
	  ystr = yc;
	}
      prefix = "parity";
      prefix += ystr;
    }
  fgBaseName = prefix + string ("_") + runstr;
  fgAnaStr = a;
}

// Private member functions


// Non member functions

string
GetEnvOrDef (string e, const string d)
{
  // Return value of environment variable e, or default d

  char *env = getenv (e.c_str());
  if (env == 0)
    return d;
  else
    return env;
}
