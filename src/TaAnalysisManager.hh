#ifndef __TaAnalysisManager__
#define __TaAnalysisManager__
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//        TaAnalysisManager.hh   (header file)
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

  Int_t Init ();              // Initialization with online data
  Int_t Init (RunNumber_t);   // Initialization with run number
  Int_t Init (string);        // Initialization with file path
  Int_t Process();            // Process all data
  Int_t End();                // End all analysis
  
 private:

  // Private member functions
  Int_t InitCommon ();        // Code common to all Init routines
  TaAnalysisManager (const TaAnalysisManager&) {}  // Do not use
  TaAnalysisManager& operator= (const TaAnalysisManager&) { return *this; } // Do not use

  // Static constants
  static const Int_t fgTAAM_ERROR = -1;
  static const Int_t fgTAAM_OK = 0;

  // Data members
  TaRun* fRun;              // Requested run
  VaAnalysis* fAnalysis;    // Requested analysis
  TFile* fRootFile;         // Root file for analysis results

#ifdef DICT
  ClassDef (TaAnalysisManager, 0) // Drives the entire analysis.  
#endif
};

#endif
