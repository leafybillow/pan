//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaCutList.hh  (interface)
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

#ifndef PAN_TaCutList
#define PAN_TaCutList


#include "TObject.h"
#include "PanTypes.hh"
#include <vector>
#include <list>
#include "TaCutInterval.hh"
#include "VaDataBase.hh"

class TaCutList
{
public:

  // Constructors/destructors/operators
  TaCutList(RunNumber_t run);
  TaCutList();
  virtual ~TaCutList() {}
  TaCutList (const TaCutList& copy);
  TaCutList& operator= (const TaCutList& assign);

  // Major functions
  void Init(const VaDataBase&);
  Bool_t OK (const TaEvent&) const;  // True if event not in any cut interval
  vector<pair<ECutType,Int_t> > CutsFailed (const TaEvent&) const; // Cuts failed by event
  void UpdateCutInterval (const ECutType, const Int_t, const EventNumber_t);  // Update interval for this cut type
  void AddExtension (const ECutType, const UInt_t, const UInt_t);  // Add extensions to list
  void AddName (const ECutType, const string&);  // Add name to list
  void PrintInt (ostream&) const;  // Print intervals
  void PrintExt (ostream&) const;  // Print extensions
  void PrintTally (ostream&) const;  // Print tally of events failing cuts

  friend ostream& operator<< (ostream& s, const TaCutList q);

private:

  // Data members
  RunNumber_t fRunNumber;            // Run number associated with this list
  vector<TaCutInterval> fIntervals;  // List of cut intervals
  list<size_t> fOpenIntIndices;      // Indices of open interval (if any) for each cut type
  vector<UInt_t> fLowExtension;      // Low-end extension for each cut type
  vector<UInt_t> fHighExtension;     // High-end extension for each cut type
  vector<UInt_t> fTally;             // tally of cut condition failures
  vector<string> fCutNames;          // names of cuts
  static const size_t fgMaxEvent = 1000000; // Probably should be available from TaEvent, really

#ifdef DICT
  ClassDef(TaCutList, 0)   // List of cuts
#endif

};

#endif
