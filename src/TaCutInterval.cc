//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaCutInterval.cc  (implementation)
//           ^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Cut intervals.
//    A cut interval consists of a cut type, cut value, and two event
//    numbers, corresponding to the start and end of the cut condition.
//
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "TaCutInterval.hh"

#ifdef DICT
ClassImp(TaCutInterval)
#endif

// Constructors/destructors/operators

TaCutInterval::TaCutInterval ( const ECutType cu, 
			       const Int_t val, 
			       const EventNumber_t e0, 
			       const EventNumber_t e1 ):
  fCut(cu), 
  fVal(val), 
  fBegin(e0), 
  fEnd(e1) {}


TaCutInterval::TaCutInterval() {}


TaCutInterval::~TaCutInterval() {}


  // Major functions

Bool_t 
TaCutInterval::Inside (const TaEvent& ev, 
		       const UInt_t lex, 
		       const UInt_t hex) const 
{
  UInt_t f0 = (fBegin > lex) ? fBegin-lex : 0;
  return ( ( ev.GetEvNumber() >= f0 ) && 
	   ( ev.GetEvNumber() < fEnd+hex ) );
}


  // Access functions

void 
TaCutInterval::SetEnd( const EventNumber_t e1 ) 
{ fEnd = e1; }


const ECutType
TaCutInterval::GetCut() const 
{ return fCut; }   // Returns the cut


const Int_t 
TaCutInterval::GetVal() const 
{ return fVal; }

// Non member functions

ostream& 
operator<< (ostream& s, const TaCutInterval q)
{
  s << int (q.fCut) << " " << q.fVal << " " << q.fBegin << " - " << q.fEnd;
  return s;
}

