//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaCutList.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Container of cut intervals for a given run. The cut list for a
// given run identifies all the intervals during which a cut condition
// existed.  It also contains extensions for each cut type, telling
// how many events to extend each interval before and after the stored
// event numbers; a tally of events failing each cut type; and labels
// for the cut types.  It provides functions to add cut intervals to
// the list and to determine with intervals, if any, a given event is
// in. The cut list is initialized from the database and updated after
// each event is preprocessed.  
//
////////////////////////////////////////////////////////////////////////

//#define NOISY

#include "TaCutList.hh"
#include <iostream>
#include <iomanip>
#include "TaCutInterval.hh"

#ifdef DICT
ClassImp(TaCutList)
#endif

// Define a ++ for ECutType

ECutType& operator++ (ECutType& c)
{
  return c = (c == MaxCuts) ? LowBeamCut: ECutType (c+1);
}

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
  // Initialization of a cut list.  The database is queried for
  // a predefined list of cut intervals.

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
  temp = db.GetExtLo();
  for (size_t i = 0; i < temp.size() && i < MaxCuts; ++i)
    fLowExtension[i] = temp[i] * os;

  temp = db.GetExtHi();
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
TaCutList::OK (const TaEvent& ev) const   
{
  // Return true if event not in any cut interval of this list.

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
TaCutList::CutsFailed (const TaEvent& ev) const 
{
  // Return a vector of cuts (pairs of cut type and value) for which
  // the given event is inside an interval.

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
{
  // Update the cut list.
  // If val is nonzero and an open interval for this cut type exists, but
  // value is different, close that interval and open a new one.
  // If val is nonzero and no open interval for this cut type exists, make one.
  // If val is zero and an open interval for this cut type exists, close it.

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
{
  // For the given cut type, add a pair of extensions to the extensions lists.

  fLowExtension[cut] = lex;
  fHighExtension[cut] = hex;
}

void 
TaCutList::AddName (const ECutType cut, const string& s)
{
  // For the given cut type, add a label to the list of cut type names.

  fCutNames[cut] = s;
}

const string& 
TaCutList::GetName (const ECutType cut) const
{
  // Get name from list

  return fCutNames[(unsigned int) cut];
}

void
TaCutList::PrintInt (ostream& s) const
{
  // Print the intervals in the list to the given output stream.
  // Open intervals are tagged with an asterisk.

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
TaCutList::PrintExt (ostream& s) const
{
  // Print the lists of extensions.

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
TaCutList::PrintTally (ostream& s) const
{
  // Print tally of number of events failing each cut type.
  // (Note that this is not the same as the number of events actually
  // cut, since cut intervals may be extended.)

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
  // Print full information on this cut list -- intervals, extensions,
  // and tallies.

  s << "Run " << q.fRunNumber << " cuts:" << endl;
  q.PrintInt(s);
  q.PrintExt(s);
  q.PrintTally(s);
  return s;
}
