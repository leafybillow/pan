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

//#define NOISY

#include "TaCutList.hh"
#include <iostream>
#include <iomanip>
#include "TaCutInterval.hh"

#ifdef DICT
ClassImp(TaCutList)
#endif

// Constructors/destructors/operators

TaCutList::TaCutList(RunNumber_t run): fRunNumber(run)
{
}

TaCutList::TaCutList()
{
}

TaCutList::TaCutList (const TaCutList& copy)
{
  fRunNumber = copy.fRunNumber;
  fIntervals = copy.fIntervals;
  fOpenIntIndices = copy.fOpenIntIndices;
  fLowExtension = copy.fLowExtension;
  fHighExtension = copy.fHighExtension;
  fTally = copy.fTally;
  fCutNames = copy.fCutNames;
}


TaCutList& 
TaCutList::operator= (const TaCutList& assign)
{
  if (this != &assign)
    {
      fRunNumber = assign.fRunNumber;
      fIntervals = assign.fIntervals;
      fOpenIntIndices = assign.fOpenIntIndices;
      fLowExtension = assign.fLowExtension;
      fHighExtension = assign.fHighExtension;
      fTally = assign.fTally;
      fCutNames = assign.fCutNames;
    }
  return *this;
}


// Major functions

void
TaCutList::Init(const VaDataBase& db)
{
  clog << "TaCutList::Init Cut list Initialization for run " 
       << fRunNumber << endl;
  
  vector<Int_t> temp;

  fLowExtension.resize (MaxCuts, 0);
  fHighExtension.resize (MaxCuts, 0);
  fTally.resize (MaxCuts, 0);
  fCutNames.resize (MaxCuts, "");

  // Cut extensions 

  // These are specified in terms of helicity windows in the database
  // but stored in terms of events -- i.e. we multiply by the
  // oversample factor.

  SlotNumber_t os = db.GetOverSamp();
  temp = db.GetEvLo();
  for (size_t i = 0; i < temp.size() && i < MaxCuts; ++i)
    fLowExtension[i] = temp[i] * os;

  temp = db.GetEvHi();
  for (size_t i = 0; i < temp.size() && i < MaxCuts; ++i)
    fHighExtension[i] = temp[i] * os;

  // Cut values
  for (size_t i = 0; i < size_t (db.GetNumBadEv()); ++i)
    {
      temp = db.GetCutValues()[i];
      if (temp[2] >= 0 && temp[2] < MaxCuts)
	{
	  ECutType ct = (ECutType) temp[2];
	  EventNumber_t elo = temp[0];
	  EventNumber_t ehi = temp[1];
	  fIntervals.push_back(TaCutInterval (ct, temp[3], elo, ehi));
	}
      else
	cerr << "TaCutList::Init WARNING: Unknown cut type = " << temp[2]
	     << " found in database -- ignoring" << endl;
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
      ++fTally[cut];
      if ( oi != fIntervals.size() )
	{
	  // Found open cut interval for this cut
	  if ( fIntervals[oi].GetVal() != val )
	    {
	      fIntervals[oi].SetEnd(ev);
	      fOpenIntIndices.erase(oiit);
	      fIntervals.push_back(TaCutInterval(cut, val, ev, fgMaxEvent));
	      fOpenIntIndices.push_back(fIntervals.size()-1);
#ifdef NOISY
	      clog << *this;
#endif
	    }
	}
      else
	{
	  // No open cut interval for this cut
	  fIntervals.push_back(TaCutInterval(cut, val, ev, fgMaxEvent));
	  fOpenIntIndices.push_back(fIntervals.size()-1);
#ifdef NOISY
	      clog << *this;
#endif
	}
    }
  else
    {
      if ( oi != fIntervals.size() )
	{
	  // Found open cut interval for this cut
	  fIntervals[oi].SetEnd(ev);
	  fOpenIntIndices.erase(oiit);
#ifdef NOISY
	      clog << *this;
#endif
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

void 
TaCutList::AddName (const ECutType cut, const string& s)
  // Add name to list
{
  fCutNames[cut] = s;
}

void
TaCutList::printInt (ostream& s) const
{
  // Print intervals
  for (size_t i = 0;
       i < fIntervals.size();
       ++i)
    {
      s << fIntervals[i];
      list<size_t>::const_iterator j;
      for (j = fOpenIntIndices.begin();
	   (j != fOpenIntIndices.end()) && (*j != i);
	   ++j)
	{}
      if (j != fOpenIntIndices.end())
	s << "*";
      s << endl;
    }
}

void
TaCutList::printExt (ostream& s) const
{
  // Print extensions
  s << "Low extensions:" << endl;
  for (vector<UInt_t>::const_iterator k = fLowExtension.begin();
       k != fLowExtension.end();
       ++k)
    s << " " << *k;
  s << endl;
  s << "High extensions:" << endl;
  for (vector<UInt_t>::const_iterator k = fHighExtension.begin();
       k != fHighExtension.end();
       ++k)
    s << " " << *k;
  s << endl;
}

void
TaCutList::printTally (ostream& s) const
{
  // Print tally of events failing cuts
  s << "Events failing cut conditions:" << endl;
  for (size_t k = 0;
       k != fTally.size();
       ++k)
    {
      s << k << "  ";
      clog.setf(ios::left,ios::adjustfield);
      // N.B. seem to need to convert to C-style string for setw to work.
      s << setw(15) << fCutNames[k].c_str() << " " ;
      clog.setf(ios::right,ios::adjustfield);
      s << setw(6) << fTally[k] << endl;
    }
}

// Private member functions

// Friend functions

ostream& 
operator<< (ostream& s, const TaCutList q)
{
  s << "Run " << q.fRunNumber << " cuts:" << endl;
  q.printInt(s);
  q.printExt(s);
  q.printTally(s);
  return s;
}
