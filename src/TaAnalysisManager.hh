#ifndef __TaAnalysisManager__
#define __TaAnalysisManager__
//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaAnalysisManager.hh   (header file)
//        ^^^^^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    This is the class that drives the entire analysis.  
//
//
//////////////////////////////////////////////////////////////////////////

// Decomment this when compiling with Root.
// #include <TObject.h>
#include <string>
#include "Rtypes.h"
#include "PanTypes.hh"

class TaRun;
class VaAnalysis;
class TROOT;
class TFile;

class TaAnalysisManager
{

 public:

  // Constructors/destructors/operators
  TaAnalysisManager ();

  // There should be no need to copy or assign a TaAnalysisManager, so
  // the copy constructor and assignment operator are private and
  // null.
  
  virtual ~TaAnalysisManager();
  
  // Major functions
  // Init does initialization.
  // We can initialize with:
  //  * no argument (for online analysis),
  //  * a run number (for analysis from a file whose name and path are
  //    presumed to be standard),
  //  * or a file name (for analysis from a file with arbitrary name and
  //    path).
  // Process does the actual loop over events, and End handles end of
  // analysis tasks.  Typically a calling function will just call
  // Init, Process, and End once each.
  void Init ();
  void Init (RunNumber_t);
  void Init (string);
  void Process();
  void End();
  
 private:
  // Private member functions
  void InitCommon ();
  TaAnalysisManager (const TaAnalysisManager&) {}
  TaAnalysisManager& operator= (const TaAnalysisManager&) { return *this; }

  // Data members
  TaRun* fRun;    
  VaAnalysis* fAnalysis;
  TFile* fRootFile;

#ifdef DICT
  ClassDef (TaAnalysisManager, 0)
#endif
};

#endif
