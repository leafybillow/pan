//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBeamAna.hh  (interface)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    Beam quality analysis.  This class derives from VaAnalysis.
//    It simply puts differences and asymmetries of beam monitors
//    into the output root file using the AutoPairAna lists, and prints
//    statistics on these quantities periodically.
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_TaBeamAna
#define PAN_TaBeamAna

#include <TObject.h>
#include <TTree.h> 
#include "VaAnalysis.hh"

//class TaStatistics;
class TaRun;

class TaBeamAna: public VaAnalysis {
  
public:
  
  // Constructors/destructors/operators
  TaBeamAna();
  ~TaBeamAna();
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  
  // Major functions
  
  // Data access functions
  
private:
  
  // We should not need to copy or assign an analysis, so copy
  // constructor and operator= are private.
  TaBeamAna(const TaBeamAna& copy);
  TaBeamAna& operator=( const TaBeamAna& assign);
  
  // Member functions
  void EventAnalysis ();
  void PairAnalysis ();
  void InitChanLists ();
  
  // Data members
  
#ifdef DICT
  ClassDef(TaBeamAna, 0)  // Analysis of Beam Data
#endif
};

#endif
