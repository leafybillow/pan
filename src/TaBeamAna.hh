//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaBeamAna.hh  (header file)
//           ^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Beam quality analysis.
//
//////////////////////////////////////////////////////////////////////////

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
  ClassDef(TaBeamAna, 0)
#endif
};

#endif
