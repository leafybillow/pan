//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaCutList.hh  (header file)
//           ^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    List of cuts applied to data.
//
//////////////////////////////////////////////////////////////////////////


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

  // Major functions
  void Init(const VaDataBase&);
  Bool_t OK (const TaEvent&) const;  // True if event not in any cut interval
  vector<pair<ECutType,Int_t> > CutsFailed (const TaEvent&) const; // Cuts failed by event
  void UpdateCutInterval (const ECutType, const Int_t, const EventNumber_t);  // Update interval for this cut type
  void AddExtension (const ECutType, const UInt_t, const UInt_t);  // Add extensions to list
  void AddName (const ECutType, const string&);  // Add name to list
  void printInt (ostream&) const;  // Print intervals
  void printExt (ostream&) const;  // Print extensions
  void printTally (ostream&) const;  // Print tally of events failing cuts

  friend ostream& operator<< (ostream& s, const TaCutList q);

private:

  // Data members
  RunNumber_t fRunNumber;
  vector<TaCutInterval> fIntervals;
  // Indices of open interval (if any) for each cut type:
  list<size_t> fOpenIntIndices;
  vector<UInt_t> fLowExtension;
  vector<UInt_t> fHighExtension;
  vector<UInt_t> fTally;       // tally of cut condition failures
  vector<string> fCutNames;    // names of cuts
  static const size_t fgMaxEvent = 1000000; // Probably should be available from TaEvent, really

#ifdef DICT
  ClassDef(TaCutList, 0)
#endif

};

#endif
