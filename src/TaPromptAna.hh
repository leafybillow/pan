//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPromptAna.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Prompt data analysis.  This class derives from VaAnalysis.  It
//    simply puts differences and asymmetries of beam monitors and
//    detectors into the output root file using the AutoPairAna lists,
//    and prints statistics on these quantities periodically.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaPromptAna
#define PAN_TaPromptAna

#include <TObject.h>
#include <TTree.h> 
#include "VaAnalysis.hh"

//class TaStatistics;
class TaRun;
class TaEvent;

class TaPromptAna: public VaAnalysis {
  
public:
  
  // Constructors/destructors/operators
  TaPromptAna();
  ~TaPromptAna();
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  
  // Major functions
  
  // Data access functions
  
private:
  
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  TaPromptAna(const TaPromptAna& copy);
  TaPromptAna& operator=( const TaPromptAna& assign);
  
  // Member functions
  void EventAnalysis ();
  void PairAnalysis ();
  void InitChanLists ();
  
  // Data members
  
#ifndef NODICT
  ClassDef(TaPromptAna, 0)  // Prompt Data Analysis
#endif
};

#endif
