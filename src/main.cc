//////////////////////////////////////////////////////////////////////////
//
//  The Hall A Parity ROOT Analyzer
//
//  Main program
//  Usage:  pan [-r runnum] [-f filename] [-o]
//
//  Authors: R. Holmes, A. Vacheret, R. Michaels
//
//////////////////////////////////////////////////////////////////////////

#include "TaAnalysisManager.hh"
#include "TROOT.h"
#include "TRint.h"

extern void InitGui();
VoidFuncPtr_t initfuncs[] = { InitGui, 0 };

static TROOT root( "Pan", "Parity Analyzer Interactive Interface", initfuncs );

void usage();

int main(int argc, char **argv) 
{

  int runnum = 0;
  int choice = 0;
  char *cfilename;

  for (int i=0 ; i < argc; i++) 
    {
      if (strcasecmp(argv[i],"-r") == 0) 
	{
	  if (i < argc-1) 
	    {
	      runnum = atoi(argv[i+1]);
	      choice = 1;
	    }
	}
      else if (strcasecmp(argv[i],"-f") == 0) 
	{
	  if (i < argc-1) 
	    {
	      cfilename = new char[strlen(argv[i+1])+1];
	      strcpy(cfilename,argv[i+1]);
	      choice = 2;
	    }
	}
      else if (strcasecmp(argv[i],"-o") == 0)
	choice = 3;
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
      if (am.Process() != 0)
	return 1;
      if (am.End() != 0)
	return 1;
      return 0;
    }
}


void usage() {
// Prints usage instructions
  cerr << "Usage:  pan [-r runnum] [-f filename]" << endl;
  cerr << "runnum is the run number" << endl;
  cerr << "filename is the full-path-name of CODA file." << endl;
  cerr << "(Specify at most one of the above two.  Online data" << endl;
  cerr << "are analyzed if no run number or path is given.)"<<endl;
  cerr << "Valid examples: " << endl;
  cerr << "   ./pan -r 1824 -n 1000" << endl;
  cerr << "   ./pan -f /work/halla/parity/rom/parity01_1824.dat" << endl;
#ifndef ONLINE
  cerr << "Since you did not compile with ONLINE flag in Makefile" << endl;
  cerr << "a filename or run number must be specified. " << endl;
#else
  cerr << "   ./pan" << endl;
#endif
  cerr << "Try again..." << endl;
}
