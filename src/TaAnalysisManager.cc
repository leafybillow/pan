//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaAnalysisManager.cc   (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Manages the overall analysis.  This is a simple class whose
//    three main methods are Init, Process, and End; calling these
//    three in that order, once each, gives an analysis of a run.
//
////////////////////////////////////////////////////////////////////////

#include "TROOT.h"
#include "TFile.h"
#include "TaAnalysisManager.hh" 
#include "TaRun.hh"
#include "TaString.hh"
#include "VaDataBase.hh"
#include "VaAnalysis.hh"
#include "TaBeamAna.hh"
#include "TaADCCalib.hh"

#ifdef DICT
ClassImp(TaAnalysisManager)
#endif

// Constructors/destructors/operators

TaAnalysisManager::TaAnalysisManager (): 
  fRun(0), 
  fAnalysis(0),
  fRootFile(0)
{
}


TaAnalysisManager::~TaAnalysisManager ()
{
}


// Major functions

ErrCode_t
TaAnalysisManager::Init ()
{
  // Initialization routine for use with online data

#ifdef ONLINE
  fRun = new TaRun();
  return InitCommon();
#else
  cerr << "TaAnalysisManager::Init ERROR: Not compiled with ONLINE, cannot analyze online data" << endl;
  return fgTAAM_ERROR;
#endif
}


ErrCode_t 
TaAnalysisManager::Init (RunNumber_t run)
{
  // Initialization routine for replay, data from file derived from
  // run number

  fRun = new TaRun(run);
  return InitCommon();
}


ErrCode_t 
TaAnalysisManager::Init (string runfile)
{
  // Initialization routine for replay, data from a given file

  fRun = new TaRun(runfile);
  return InitCommon();
}


ErrCode_t
TaAnalysisManager::Process()
{
  // Process all data

  return fAnalysis->ProcessRun();
}

ErrCode_t
TaAnalysisManager::End()
{
  // Cleanup for overall analysis

  fAnalysis->RunFini ();
  fAnalysis->Finish(); // take care of end-of-analysis tasks
  fRun->Finish(); // compute and print/store run results
  fRootFile->Write();
  fRootFile->Close();
  // Move the generic root file to 'pan_%d.root' where %d is the run number.
  char syscommand[200];
  string anatype;
  anatype = fRun->GetDataBase().GetAnaType();
  char *path;
  path = getenv("ROOT_OUTPUT");
  if (path == NULL) {
    sprintf(syscommand,"mv pan.root pan_%d.root",fRun->GetRunNumber());
  } else {
    sprintf(syscommand,"mv %s/pan.root %s/pan_%d.root",path,path,fRun->GetRunNumber());
  }
  system(syscommand);

  delete fAnalysis;
  delete fRun;
  delete fRootFile;
  return fgTAAM_OK; // for now always return OK
}


// Private member functions

ErrCode_t
TaAnalysisManager::InitCommon()
{
  // Common setup for overall management of analysis

  // Make the ROOT output file, generic at first since we don't know
  // run number yet.

  char rootfile[50];
  char *path;
  path = getenv("ROOT_OUTPUT");
  if (path == NULL) {
    sprintf(rootfile,"pan.root");
  } else {
    sprintf(rootfile,"%s/pan.root",path);
  }
  fRootFile = new TFile(rootfile,"RECREATE");
  fRootFile->SetCompressionLevel(0);

  // Initialize the run
  int status = fRun->Init();
  if (status != 0)
    return status;

  // Check the database.  If there is a problem, you cannot continue.

  clog << "checking database ..."<<endl;
  if ( !fRun->GetDataBase().SelfCheck() ) {
    cerr << "TaAnalysisManager::Init ERROR: Invalid database.  Quitting."<<endl;
    return fgTAAM_ERROR;
  }

  // Make the desired kind of analysis
  TaString theAnaType = fRun->GetDataBase().GetAnaType();

  clog << "TaAnalysisManager::Init Analysis type is " 
       << theAnaType << endl;

  if (theAnaType.CmpNoCase("beam") == 0)
    fAnalysis = new TaBeamAna;
  else if (theAnaType.CmpNoCase("adcped") == 0)
    fAnalysis = new TaADCCalib("adcped");
  else if (theAnaType.CmpNoCase("adcdac") == 0)
    fAnalysis = new TaADCCalib("adcdac");
  else
    {
      cerr << "TaAnalysisManager::Init ERROR: Invalid analysis type = "
	   << theAnaType << endl;
      return fgTAAM_ERROR;
    }
  
  fAnalysis->Init();
  return fAnalysis->RunIni (*fRun);
}

