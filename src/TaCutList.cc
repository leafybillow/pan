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
}

TaCutList::TaCutList()
{
}

// Major functions

void
TaCutList::Init(const VaDataBase& db)
{
  clog << "TaCutList::Init Cut list Initialization for run " 
       << fRunNumber << endl;

  vector<Int_t> temp;

  // Cut extensions 

  // These can be replaced by simple assignments if types of GetEvLo,
  // GetEvHi are changed to vector<UInt_t>.  However, we should then
  // still do the resize to guarantee the correct size vector.

  fLowExtension.resize (MaxCuts, 0);
  fHighExtension.resize (MaxCuts, 0);

  temp = db.GetEvLo();
  for (size_t i = 0; i < temp.size() && i < MaxCuts; ++i)
    fLowExtension[i] = temp[i];

  temp = db.GetEvHi();
  for (size_t i = 0; i < temp.size() && i < MaxCuts; ++i)
    fHighExtension[i] = temp[i];

  // Cut values
  for (size_t i = 0; i < db.GetNumBadEv(); ++i)
    {
      temp = db.GetCutValues()[i];
      ECutType ct = temp[2];
      EventNumber_t elo = temp[0];
      EventNumber_t ehi = temp[1];
      fIntervals.push_back(TaCutInterval (ct, temp[3], elo, ehi));
    }
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

ostream& 
operator<< (ostream& s, const TaCutList q)
{
  s << endl;
  s << "Run " << q.fRunNumber << endl;
  for (size_t i = 0;
       i < q.fIntervals.size();
       ++i)
    {
      s << q.fIntervals[i];
      list<size_t>::const_iterator j;
      for (j = q.fOpenIntIndices.begin();
	   (j != q.fOpenIntIndices.end()) && (*j != i);
	   ++j)
	{}
      if (j != q.fOpenIntIndices.end())
	s << "*";
      s << endl;
    }
  s << "Low extensions:" << endl;
  for (vector<UInt_t>::const_iterator k = q.fLowExtension.begin();
       k != q.fLowExtension.end();
       ++k)
    s << " " << *k;
  s << endl;
  s << "High extensions:" << endl;
  for (vector<UInt_t>::const_iterator k = q.fHighExtension.begin();
       k != q.fHighExtension.end();
       ++k)
    s << " " << *k;
  s << endl;
}
