//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT parity analyzer  Pan           
//
//              VaPair.cc           Author : A. Vacheret, R. S. Holmes    
//
//    base class of pairing process in analysis of runs     
//
//    Round 1 Sep 2001  AV
//    Extensively modified Dec 2001 RSH
//
//////////////////////////////////////////////////////////////////////////


#include "TaCutList.hh"
#include "TaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "VaPair.hh" 

#ifdef DICT
ClassImp(VaPair)
#endif

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


void 
VaPair::RunInit()
{
  fgEventQueue.clear();
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
  fResults.push_back(lq);
}


Double_t 
VaPair::GetDiff (Int_t key)
{
  return GetRight().GetData(key) - GetLeft().GetData(key);
}


Double_t 
VaPair::GetAsy (Int_t key)
{
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
