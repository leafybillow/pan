//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           main.cc
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//  Main program
//  Usage:  pan [-r runnum] [-f filename] [-o]
//
////////////////////////////////////////////////////////////////////////

#include "TaAnalysisManager.hh"
#include "TROOT.h"
#include "TRint.h"

extern void InitGui();
VoidFuncPtr_t initfuncs[] = { InitGui, 0 };

static TROOT root( "Pan", "Parity Analyzer Interactive Interface", initfuncs );

void usage();

int main(int argc, char **argv) 
{

  clog << "Pan $Name$:$Id$\n" << endl;

  int runnum = 0;
  int choice = 0;
  char *cfilename;
  Bool_t twopass(false);

  int i = 1;
  while (i < argc) 
    {
      if (strcasecmp(argv[i],"-r") == 0) 
	{
	  if (i < argc-1) 
	    {
	      runnum = atoi(argv[++i]);
	      choice = 1;
	    }
	  else
	    {
	      usage();
	      return 1;
	    }
	}
      else if (strcasecmp(argv[i],"-f") == 0) 
	{
	  if (i < argc-1) 
	    {
	      cfilename = new char[strlen(argv[++i])+1];
	      strcpy(cfilename,argv[i]);
	      choice = 2;
	    }
	  else
	    {
	      usage();
	      return 1;
	    }
	}
      else if (strcasecmp(argv[i],"-o") == 0)
	choice = 3;
      else if (strcasecmp(argv[i],"-2") == 0)
	twopass = true;
      else
	{
	  usage();
	  return 1;
	}
      ++i;
    }
  
  if (twopass && (choice == 0 || choice == 3))
    {
      usage();
      return 1;
    }

  // Done processing command line.  Hand off to analysis manager.  

  TROOT pan ( "pan", "Parity analyzer" );

  if (choice == 0)
    {
      // Interactive use with ROOT prompt
      TRint *theApp = new TRint( "The Hall A parity analyzer", &argc, argv, NULL, 0, kFALSE );
      theApp->Run();
      delete theApp;
      return 0;
    }
  else 
    {
      TaAnalysisManager am;

      // Command line use with run number, file name or online specified
      if (choice == 1) 
	{
	  if (am.Init(runnum) != 0)
	    return 1;
	}
      else if (choice == 2) 
	{
	  string filename = cfilename;
	  if (am.Init(filename) != 0)
	    return 1;
	}
      else if (choice == 3) 
	{
#ifdef ONLINE
	  if (am.Init() != 0)
	    return 1;
#else
	  usage();
	  return 1;
#endif
	}
      if (twopass)
	{
	  if (am.Process() != 0 || 
	      am.EndPass1() != 0 || 
	      am.InitPass2() != 0)
	    return 1;
	}
      if (am.Process() != 0 ||
	  am.End() != 0)
	return 1;
      return 0;
    }
}


void usage() {
// Prints usage instructions

  cerr << "Usage:  pan [data source specifier] [-2]" << endl;
  cerr << "" << endl;
  cerr << "  where data source specifier is" << endl;
  cerr << "    -r runnum   to analyze run number <runnum>" << endl;
  cerr << "    -f filepath to analyze CODA file at <filepath>" << endl;
#ifdef ONLINE
  cerr << "    -o          to analyze online data" << endl;
#else
  cerr << "                [Recompile with ONLINE flag in Makefile to allow" << endl;
  cerr << "                analysis of online data]" << endl;
#endif
  cerr << "  Interactive prompt will be given if no data source is specified." << endl;
  cerr << "" << endl;
  cerr << "  Other options:" << endl;
  cerr << "    -2          Do 2-pass analysis (with -r or -f only)." << endl;
  cerr << "" << endl;
  cerr << "  Valid examples:" << endl;
  cerr << "     ./pan -r 1824 -2" << endl;
  cerr << "     ./pan -f /work/halla/parity/rom/parity01_1824.dat" << endl;
#ifdef ONLINE
  cerr << "     ./pan -o " << endl;
#endif
  cerr << "     ./pan" << endl;
}
