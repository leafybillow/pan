//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaCutList.cc  (implementation)
//           ^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    List of cuts applied to data.
//
//////////////////////////////////////////////////////////////////////////


#include "TaCutList.hh"
#include <iostream>
#include "TaCutInterval.hh"

#ifdef DICT
ClassImp(TaCutList)
#endif

// Constructors/destructors/operators

// For now we just set up empty cut lists and extension lists.  To be
// done: Initialize cut list from database.

TaCutList::TaCutList(RunNumber_t run): fRunNumber(run)
{
  fLowExtension.resize (MaxCuts, 0);
  fHighExtension.resize (MaxCuts, 0);
}

TaCutList::TaCutList()
{
  fLowExtension.resize (MaxCuts, 0);
  fHighExtension.resize (MaxCuts, 0);
}

// Major functions

void
TaCutList::Init()
{
  cout << "TaCutList::Init Cut list Initialization for run " 
       << fRunNumber << endl;
}


Bool_t 
TaCutList::OK (const TaEvent& ev) const   // True if event not in any cut interval
{
  Bool_t oksofar = true;
  for (vector<TaCutInterval>::const_iterator c = fIntervals.begin();
       oksofar && (c != fIntervals.end()); 
       ++c )
    {
      if (c->Inside(ev, fLowExtension[c->GetCut()], fHighExtension[c->GetCut()]))
	oksofar = false;
    }
  return oksofar;
}

vector<pair<ECutType,Int_t> >
TaCutList::CutsFailed (const TaEvent& ev) const // Cuts failed by event
{
  vector<pair<ECutType,Int_t> > cf;
  for (vector<TaCutInterval>::const_iterator c = fIntervals.begin();
       c != fIntervals.end(); 
       ++c )
    {
      if (c->Inside(ev, fLowExtension[c->GetCut()], fHighExtension[c->GetCut()]))
	cf.push_back(make_pair(c->GetCut(),c->GetVal()));
    }
  return cf;
}


void 
TaCutList::UpdateCutInterval (const ECutType cut, const Int_t val, const EventNumber_t ev)
  // If val is nonzero and an open interval for this cut type exists, but
  // value is different, close that interval and open a new one.
  // If val is nonzero and no open interval for this cut type exists, make one.
  // If val is zero and an open interval for this cut type exists, close it.
{
  // Look for an open interval for this cut
  list<size_t>::iterator oiit;
  for ( oiit = fOpenIntIndices.begin();
	oiit != fOpenIntIndices.end() && fIntervals[*oiit].GetCut() != cut;
	++oiit )
    { } // empty loop body
  size_t oi = ( oiit == fOpenIntIndices.end() ) ? fIntervals.size() : *oiit;

  if ( val )
    {
      if ( oi != fIntervals.size() )
	{
	  // Found open cut interval for this cut
	  if ( fIntervals[oi].GetVal() != val )
	    {
	      fIntervals[oi].SetEnd(ev);
	      fOpenIntIndices.erase(oiit);
	      fIntervals.push_back(TaCutInterval(cut, val, ev, fgMaxEvent));
	      fOpenIntIndices.push_back(fIntervals.size()-1);
	    }
	}
      else
	{
	  // No open cut interval for this cut
	  fIntervals.push_back(TaCutInterval(cut, val, ev, fgMaxEvent));
	  fOpenIntIndices.push_back(fIntervals.size()-1);
	}
    }
  else
    {
      if ( oi != fIntervals.size() )
	{
	  // Found open cut interval for this cut
	  fIntervals[oi].SetEnd(ev);
	  fOpenIntIndices.erase(oiit);
	}
    }
}

void 
TaCutList::AddExtension (const ECutType cut, const UInt_t lex, const UInt_t hex)
  // Add extensions to list
{
  fLowExtension[cut] = lex;
  fHighExtension[cut] = hex;
}

// Private member functions
