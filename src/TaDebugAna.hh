//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaDebugAna.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Debug analysis.  This class derives from VaAnalysis.  No ROOT
//    file is created.  It simply prints some basic information for
//    each event, and prints statistics on differences and asymmetries
//    of beam monitors and detectors periodically.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaDebugAna
#define PAN_TaDebugAna

#include <TObject.h>
#include <TTree.h> 
#include "VaAnalysis.hh"

//class TaStatistics;
class TaRun;
class VaEvent;

class TaDebugAna: public VaAnalysis {
  
public:
  
  // Constructors/destructors/operators
  TaDebugAna();
  ~TaDebugAna();
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  
  // Major functions
  
  // Data access functions
  
private:
  
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  TaDebugAna(const TaDebugAna& copy);
  TaDebugAna& operator=( const TaDebugAna& assign);
  
  // Member functions
  void EventAnalysis ();
  void PairAnalysis ();
  void InitChanLists ();
  
  // Data members
  
#ifndef NODICT
  ClassDef(TaDebugAna, 0)  // Prompt Data Analysis
#endif
};

#endif
