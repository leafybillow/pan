//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaPair.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Base class for pairs of events of opposite helicities.  Contains
// (when full) two events, as well as the results of analysis of that
// pair. Different derived classes correspond to different beam
// helicity structures, i.e., different methods of getting a pair from
// an event sequence. Methods are provided to compute differences and
// asymmetries for a given device.
//
////////////////////////////////////////////////////////////////////////


#include "TaCutList.hh"
#include "TaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "VaPair.hh" 
#include "TaRun.hh"

#ifdef DICT
ClassImp(VaPair)
#endif

// Static members
const ErrCode_t VaPair::fgVAP_OK = 0;
const ErrCode_t VaPair::fgVAP_ERROR = -1;
deque< TaEvent > 
VaPair::fgEventQueue;


VaPair::VaPair()
{
}


VaPair::VaPair(const VaPair& copy)
{
  fResults = copy.fResults;
  fEvLeft = copy.fEvLeft;
  fEvRight = copy.fEvRight;
}


VaPair &
VaPair::operator=(const VaPair &assign)
{
  if (this != &assign)
    { 
      fResults = assign.fResults;
      fEvLeft = assign.fEvLeft;
      fEvRight = assign.fEvRight;
    } 
  return *this;
}


VaPair::~VaPair()
{
}


ErrCode_t
VaPair::RunInit(const TaRun& run)
{
  // Initialization at start of run -- clear event queue (so we don't
  // pair events from two different runs, for instance)
  fgEventQueue.clear();
  return fgVAP_OK;
}


const TaEvent& 
VaPair::GetRight() const
{
  return fEvRight;
}


const TaEvent& 
VaPair::GetLeft() const
{
  return fEvLeft;
}


void 
VaPair::QueuePrint() const
{
  // Print stored events, for diagnostic purposes

  if ( fgEventQueue.size() !=0 )
    {
      cout<<" ----- PAIR event queue print out -------- "<<endl;
      for ( deque< TaEvent >::iterator fEvIdx  = fgEventQueue.begin() ; 
            fEvIdx != fgEventQueue.end() ; 
            fEvIdx++ )
        cout<<"event number "<< (*fEvIdx).GetEvNumber() << endl;
      cout<<" ----------------------------------------- " <<endl;
    }
  else cout<< " Pair Event ring empty, no print out "<<endl; 
}


void 
VaPair::AddResult (const TaLabelledQuantity& lq)
{
  // Insert pair analysis results into pair.
  fResults.push_back(lq);
}


Double_t 
VaPair::GetDiff (Int_t key)
{
  // Get difference in quantity indexed by key for this pair.
  return GetRight().GetData(key) - GetLeft().GetData(key);
}


Double_t 
VaPair::GetAsy (Int_t key)
{
  // Get asymmetry in quantity indexed by key for this pair.
  Double_t denom = GetRight().GetData(key) + GetLeft().GetData(key);
  if ( denom == 0 )
    {
      cerr << "VaPair::GetAsy ERROR: Denominator is zero, key = " 
           << key << endl;
      return 0;
    }
  return (GetRight().GetData(key) - GetLeft().GetData(key)) / denom;
}

Bool_t 
VaPair::PassedCuts()
{
// True if neither event has cut condition

  return ! ( GetLeft().CutStatus() || GetRight().CutStatus() );
}

Bool_t 
VaPair::PassedCutsInt(const TaCutList& cl)
{
// True if neither event is in cut interval

  return ( cl.OK(GetLeft()) && cl.OK(GetRight()) );
}

const vector<TaLabelledQuantity>& 
VaPair::GetResults() const
{
  return fResults;
}
