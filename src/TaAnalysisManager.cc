//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaAnalysisManager.cc   (implementation)
//           ^^^^^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    This is the class that drives the entire analysis.  
//
//
//////////////////////////////////////////////////////////////////////////

#include <strstream>
#include "TROOT.h"
#include "TFile.h"
#include "TaAnalysisManager.hh" 
#include "TaRun.hh"
#include "VaDataBase.hh"
#include "VaAnalysis.hh"
#include "TaBeamAna.hh"
#include "TaFdbkAna.hh"
#include "TaADCCalib.hh"

#ifdef DICT
ClassImp(TaAnalysisManager)
#endif

// This non-member string comparison routine probably should be somewhere else!
int cmp_nocase (const string& s, const string& s2)
{
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end())
    {
      if (toupper(*p) != toupper(*p2))
	return (toupper(*p) < toupper(*p2)) ? -1 : 1;
      ++p;
      ++p2;
    }

  return (s2.size() == s.size()) ? 0 : (s.size() < s2.size()) ? -1 : 1;
}


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

// Init for use with online data
void TaAnalysisManager::Init ()
{
#ifdef ONLINE
  fRun = new TaRun();
  InitCommon();
#else
  cerr << "TaAnalysisManager::Init ERROR: Not compiled with ONLINE, cannot analyze online data" << endl;
  exit (1);
#endif
}


// Init for replay, data from file derived from run number
void TaAnalysisManager::Init (RunNumber_t run)
{
  fRun = new TaRun(run);
  InitCommon();
}


// Init for replay, data from a given file
void TaAnalysisManager::Init (string runfile)
{
  fRun = new TaRun(runfile);
  InitCommon();
}


void
TaAnalysisManager::Process()
{
  fAnalysis->ProcessRun();
}

void
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
  anatype = fRun->GetDataBase()->GetAnaType();
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
  return;
}


// Private member functions

void
TaAnalysisManager::InitCommon()
{

  // Common setup for overall management of analysis

  // Make the ROOT output file, generic at first since we don't know run number yet.

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
  fRun->Init();

  // Check the database.  If there is a problem, you cannot continue.
  // (Rich, I'm not sure where you want to put this.  -Bob)
  cout << "checking database ..."<<endl;
  if ( !fRun->GetDataBase()->SelfCheck() ) {
    cout << "TaAnalysisManager::Init ERROR: Invalid database.  Quitting."<<endl;
    exit (1);
  }

  // Make the desired kind of analysis
  string theAnaType = fRun->GetDataBase()->GetAnaType();

  cout << "TaAnalysisManager::Init Analysis type is " 
       << theAnaType << endl;

  if (cmp_nocase (theAnaType, "beam") == 0)
    fAnalysis = new TaBeamAna;
  else if(cmp_nocase (theAnaType, "fdbk") == 0)
    fAnalysis = new TaFdbkAna;
  else if (cmp_nocase (theAnaType, "adcped") == 0)
    fAnalysis = new TaADCCalib("adcped");
  else if (cmp_nocase (theAnaType, "adcdac") == 0)
    fAnalysis = new TaADCCalib("adcdac");
  else
    {
      cout << "TaAnalysisManager::Init ERROR: Invalid analysis type = "
	   << theAnaType << endl;
      exit (1);
    }
  
  fAnalysis->Init();
  fAnalysis->RunIni (*fRun);
}

