//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//       VaAnalysis.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// Abstract base class of analysis. Derived classes include TaADCCalib
// (for computation of pedestals and DAC noise slopes) and TaBeamAna
// (for analysis of beam characteristics).  Future derived classes may
// include TaAsymAna (for analysis of physics asymmetries),
// TaModulaAna (for computation of beam modulation coefficients), and
// TaCorrecAna (for computation of corrections due to
// helicity-correlated beam differences).  Each of these is
// responsible for some treatment of TaEvents from a TaRun.  The type
// of analysis to be done is specified in the database, and the
// TaAnalysisManager instantiates the appropriate analysis class
// accordingly.
//
// VaAnalysis has initialization and termination routines for both the
// overall analysis and the analysis of a particular run.  At present
// Pan is designed to analyze only a single run, but these routines
// provide for a possible future version that will handle multiple
// runs.
//
// The main event loop is inside the ProcessRun method.  The three
// main methods called from here are PreProcessEvt, ProcessEvt, and
// ProcessPair.  The first of these places the most recently read
// event into a delay queue until the delayed helicity information for
// that event becomes available.  Cut conditions are checked for here.
// Once the helicity information is added the event is pushed onto a
// second delay queue, while the events are used to construct pairs
// which are pushed onto a third delay queue.  These two delay queues
// are used to hold events and pairs until we can tell whether they
// fall within a cut interval caused by a cut condition arising later.
// Events and pairs which emerge from the ends of these queues are
// analyzed in ProcessEvt and ProcessPair, respectively.  Analysis
// results are added to the events and pairs themselves.
//
////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define CHECKOUT
//#define ASYMCHECK
#define FDBK1

#include "TaCutList.hh"
#include "TaEvent.hh"
#include "TaLabelledQuantity.hh"
#include "TaPairFromPair.hh"
#include "TaRun.hh"
#include "VaAnalysis.hh"
#include "VaDataBase.hh"
#include "TaAsciiDB.hh"
#include "VaPair.hh"
#include <iostream>

#ifdef DICT
ClassImp(VaAnalysis)
#endif

const UInt_t VaAnalysis::fgNO_STATS       = 0x1;
const UInt_t VaAnalysis::fgNO_BEAM_NO_ASY = 0x2;
const UInt_t VaAnalysis::fgCOPY           = 0x4;
const UInt_t VaAnalysis::fgDIFF           = 0x8;
const UInt_t VaAnalysis::fgASY            = 0x10;

#ifdef LEAKCHECK
UInt_t VaAnalysis::fLeakNewEvt(0);
UInt_t VaAnalysis::fLeakDelEvt(0);
UInt_t VaAnalysis::fLeakNewPair(0);
UInt_t VaAnalysis::fLeakDelPair(0);
#endif

// Constructors/destructors/operators

VaAnalysis::VaAnalysis(): 
  fRun(0),
  fMaxNumEv(10000000),
  fPreEvt(0), 
  fPrePair(0), 
  fPair(0), 
  fEHelDequeMax(0),
  fEDequeMax(0),
  fPDequeMax(0),
  fPairTree(0),
  fTreeREvNum(0),
  fTreeLEvNum(0),
  fTreeMEvNum(0),
  fTreeOKCond(0),
  fTreeOKCut(0),
  fTreeSpace(0),
  fPairType(FromPair)
{ 
  fEHelDeque.clear(); 
  fEDeque.clear(); 
  fPDeque.clear();
  fTreeList.clear();
  fEvt = new TaEvent();
#ifdef LEAKCHECK
  ++fLeakNewEvt;
#endif
}


VaAnalysis::~VaAnalysis() 
{ 
  delete fPreEvt;  
  delete fPrePair;  
#ifdef LEAKCHECK
  ++fLeakDelEvt;
  ++fLeakDelPair;
#endif
  for (deque<VaPair*>::const_iterator ip = fPDeque.begin();
       ip != fPDeque.end();
       ++ip)
    {
      delete *ip;
    }
  delete fEvt;
  delete fPair;
  delete fPairTree;
  delete[] fTreeSpace;
#ifdef LEAKCHECK
  ++fLeakDelEvt;
  ++fLeakDelPair;
#endif
}


// Major functions

void 
VaAnalysis::Init() 
{ 
  // To be called at the start of each analysis.
  fEvtProc = 0;
  fPairProc = 0;
}


ErrCode_t
VaAnalysis::RunIni(TaRun& run) 
{ 
  // To be called at the start of each run.  (Present structure of Pan
  // is that each analysis handles only one run, so this is somewhat
  // redundant with Init.)  "Prime the pump" by getting a new empty
  // pair and making sure fPair is null; set maximum fEDeque and
  // fPDeque length, which depends on trigger rate, and maximum
  // fEHelDeque length, which depends on delay value and oversampling
  // factor.  Find out pair type and set fPairType accordingly.

  fRun = &run;
  fPreEvt = new TaEvent();
#ifdef LEAKCHECK
  ++fLeakNewEvt;
#endif
  NewPrePair();
  fPrePair->RunInit();
  fPair = 0;

// Define CHECKOUT if you want a printout of the database.  This might be worth
// leaving in permanently, but should at least be checked sometimes.
#ifdef CHECKOUT
  fRun->GetDataBase().Checkout();
#endif  
// ----------- end of database checkout (ifdef)

  // Get maximum events to analyze from the database
  EventNumber_t mx = 0;
  mx = fRun->GetDataBase().GetMaxEvents();
  if (mx > 0) {
    fMaxNumEv = mx;
    clog << "\nLimiting analysis to " << fMaxNumEv << "  events "<<endl;
  }

  // maximum events in fEHelDeque set equal to helicity delay times
  // oversample plus one. 

  fEHelDequeMax = fRun->GetDataBase().GetDelay() * 
    fRun->GetDataBase().GetOverSamp() + 1;

  // maximum events in fEDeque set equal to twice number of
  // events per second.  Half as many pairs in fPDeque.
  fPDequeMax = fRun->GetRate(); 
  fEDequeMax = fPDequeMax + fPDequeMax;
  if (fPDequeMax == 0)
    {
      cerr << "VaAnalysis::RunIni ERROR: Trigger rate is zero, cannot analyze"
           << endl;
      return fgVAANA_ERROR;
    }
  
  fPairTree = 0;
  fPairTree = new TTree("P","Pair data DST");
  InitChanLists();
  InitTree();
  fQSwitch = kFALSE;
  fZSwitch = kFALSE;
  fQNpair=0; fQfeedNum=0;
  fZNpair=0; fZfeedNum=0;
  fQsum.clear();
  for (Int_t i=0;i<2;i++){
    fZpair[i]=0;
    fZsum4B[i].clear();
  }  
  if ( fRun->GetDataBase().GetFdbkSwitch("AQ") == "on") fQSwitch = kTRUE;
  if ( fQSwitch) 
    {
      fQTimeScale = fRun->GetDataBase().GetFdbkTimeScale("AQ");
      clog<< " feedback timescale "<<fQTimeScale<<endl;
    }
  if ( fRun->GetDataBase().GetFdbkSwitch("PZT") == "on") fZSwitch = kTRUE;
  if ( fZSwitch) 
    {
      fZTimeScale = fRun->GetDataBase().GetFdbkTimeScale("PZT");
      clog<< " feedback timescale "<<fZTimeScale<<endl;
    }

  // Set pair type

  string type = fRun->GetDataBase().GetPairType();
  
  // Remove this ifdef when TaPairFromQuad class exists and FromQuad
  // added to enum EPairType.
#ifdef PAIRFROMQUAD
  if (type == "quad")
    fPairType = FromQuad;
  else 
#endif
    {
      if (type != "pair")
        {
          cerr << "VaAnalysis::NewPrePair WARNING: "
               << "Invalid pair type: " << type << endl;
          cerr << "Pair pairing chosen" << endl;
        }
      fPairType = FromPair;
    }
  return fgVAANA_OK;
}


ErrCode_t
VaAnalysis::ProcessRun()
{
  // Main analysis routine -- this is the event loop.  Get an event,
  // preprocess it; if the event deque is full, pop off an event,
  // analyze it, and pass it to the run to be accumulated; likewise
  // for the pair deque.

  while ( fRun->NextEvent() )
    {
      if (PreProcessEvt() != 0)
	return fgVAANA_ERROR;
      if ( fEDeque.size() == fEDequeMax )
        {
          if (ProcessEvt() != 0)
	    return fgVAANA_ERROR;
          fRun->AccumEvent (*fEvt);
        }
      if ( fPDeque.size() == fPDequeMax )
        {
          if (ProcessPair() != 0)
	    return fgVAANA_ERROR;
          fRun->AccumPair (*fPair);
        }
      if (fRun->GetEvent().GetEvNumber() % 1000 == 0)
        {
          clog << "VaAnalysis::ProcessRun: Event " 
               << fRun->GetEvent().GetEvNumber() 
               << " read -- processed " << fEvtProc << " (" << fPairProc
               << ") events (pairs)" << endl;
#ifdef LEAKCHECK
          //LeakCheck();
#endif
        }
      if (fRun->GetEvent().GetEvNumber() >= fMaxNumEv)
        break;
    }
  
  // Cleanup loops -- Keep calling ProcessEvt (ProcessPair) to analyze
  // an event (pair) from the event (pair) delay queue until that
  // queue is empty, and use results to update averages etc.  [NOTE
  // that we can't process the leftover events in the helicity delay
  // queue because we can't find out their helicity!]
  
  while ( fEDeque.size() > 0 )
    {
      if (ProcessEvt() != 0)
	return fgVAANA_ERROR;
      fRun->AccumEvent (*fEvt);
    }
  while ( fPDeque.size() > 0 )
    {
      if (ProcessPair() != 0)
	return fgVAANA_ERROR;
      fRun->AccumPair (*fPair);
    }

  return fgVAANA_OK;
}

void 
VaAnalysis::RunFini()
{
  // To be called at the end of each run.  (Present structure of Pan
  // is that each analysis handles only one run, so this is somewhat
  // redundant with Finish.)  Clear out the queues, get rid of all
  // pairs, and get rid of the pair tree.  But write it out first...


  clog << "VaAnalysis::RunFini: Run " << fRun->GetRunNumber()
       << " analysis terminated at event " 
       << fPreEvt->GetEvNumber() << endl;
  clog << "VaAnalysis::RunFini: Processed " << fEvtProc 
       << " (" << fPairProc << ") events (pairs)" << endl;

  if (fQSwitch) QasyEndFeedback();
  if (fZSwitch) PZTEndFeedback();

  fPairTree->Write();
  delete fPreEvt;
  fPreEvt = 0;
  delete fPrePair;  
  fPrePair = 0;
#ifdef LEAKCHECK
  ++fLeakDelEvt;
  ++fLeakDelPair;
#endif
  for (deque<VaPair*>::const_iterator ip = fPDeque.begin();
       ip != fPDeque.end();
       ++ip)
    {
      delete *ip;
#ifdef LEAKCHECK
      ++fLeakDelPair;
#endif
    }
  fEHelDeque.clear();
  fEDeque.clear();
  fPDeque.clear();
  delete fPair;
  fPair = 0;
  delete fPairTree;
  fPairTree = 0;
#ifdef LEAKCHECK
  ++fLeakDelPair;
#endif
}


void 
VaAnalysis::Finish() 
{ 
  // To be called at the end of each analysis.
#ifdef LEAKCHECK
  LeakCheck();
#endif
}


// Protected member functions

// We should not need to copy or assign an analysis, so copy
// constructor and operator= are protected.

VaAnalysis::VaAnalysis( const VaAnalysis& copy)
{
}


VaAnalysis& VaAnalysis::operator=(const VaAnalysis& assign)
{
  return *this;
}


ErrCode_t
VaAnalysis::PreProcessEvt()
{
  // Preprocess event, checking for cut conditions, and putting event
  // in helicity delay queue.  If that fills up, take an event off the
  // end, insert delayed helicity, and put it on the event delay
  // queue.  Package these events into pairs.  If a pair is completed,
  // push it onto pair delay queue.

#ifdef NOISY
  clog << "Entering PreProcessEvt, fEHelDeque.size() : " <<fEHelDeque.size()<< endl;
#endif
  fRun->GetEvent().CheckEvent(*fRun);

  fEHelDeque.push_back(fRun->GetEvent());

  if (fEHelDeque.size() == fEHelDequeMax)
      {
        *fPreEvt = fEHelDeque.front();
        fEHelDeque.pop_front();
        fPreEvt->SetDelHelicity(fRun->GetEvent().GetHelicity());

        fEDeque.push_back (*fPreEvt);
        if (fPrePair->Fill (*fPreEvt, *fRun))
        {
          fPDeque.push_back(fPrePair);
          NewPrePair();
        }
      }
#ifdef NOISY
  clog << "Leaving PreProcessEvt, queue sizes " << fEHelDeque.size()
       << " (" << fEHelDequeMax
       << ") " << fEDeque.size()
       << " (" << fEDequeMax
       << ") " << fPDeque.size()
       << " (" << fPDequeMax << ")" << endl;
#endif
  return fgVAANA_OK;
}


ErrCode_t
VaAnalysis::ProcessEvt()
{
  // Handle analysis of an event that has been through the event delay
  // queue, putting results into the event.

#ifdef NOISY
  clog << "Entering ProcessEvt" << endl;
#endif
  if ( fEDeque.size() )
    {
      *fEvt = fEDeque.front();
      fEDeque.pop_front();
      
      EventAnalysis();
      ++fEvtProc;
    }
  else
    cerr << "VaAnalysis::ProcessEvt WARNING: "
         << "no event on deque to analyze" << endl;
#ifdef NOISY
  clog << "Leaving ProcessEvt, queue sizes " << fEHelDeque.size()
       << " (" << fEHelDequeMax
       << ") " << fEDeque.size()
       << " (" << fEDequeMax
       << ") " << fPDeque.size()
       << " (" << fPDequeMax << ")" << endl;
#endif
  return fgVAANA_OK;
}


ErrCode_t
VaAnalysis::ProcessPair()
{
  // Handle analysis of a pair that has been through the pair delay
  // queue, putting results into the pair.

#ifdef NOISY
  clog << "Entering ProcessPair" << endl;
#endif
  if ( fPDeque.size() )
    {
      if (fPair)
        { 
          delete fPair;
#ifdef LEAKCHECK
          ++fLeakDelPair;
#endif
        }
      fPair = fPDeque.front();
      fPDeque.pop_front();

#ifdef ASYMCHECK   
      clog << " paired event "<<fPair->GetLeft().GetEvNumber()<<" with "<<fPair->GetRight().GetEvNumber()<<endl;
      clog << " BCM1(L, R) "<<fPair->GetLeft().GetData(IBCM1)<<" "<<fPair->GetRight().GetData(IBCM1)<<endl;
      clog << " BCM2(L, R) "<<fPair->GetLeft().GetData(IBCM2)<<" "<<fPair->GetRight().GetData(IBCM2)<<endl;
      clog << " BCM asy " <<fPair->GetAsy(IBCM1)*1E6 <<" "<<fPair->GetAsy(IBCM2)*1E6<<endl;
      clog << " x diff " <<endl;       
#endif
      PairAnalysis();
      fPairTree->Fill();      
      ++fPairProc;
    }
  else
    {
      cerr << "VaAnalysis::ProcessPair ERROR: "
           << "no pair on deque to analyze" << endl;
      return fgVAANA_ERROR;
    }
#ifdef NOISY
  clog << "Leaving ProcessPair, queue sizes " << fEHelDeque.size()
       << " (" << fEHelDequeMax
       << ") " << fEDeque.size()
       << " (" << fEDequeMax
       << ") " << fPDeque.size()
       << " (" << fPDequeMax << ")" << endl;
#endif
  return fgVAANA_OK;
}


ErrCode_t
VaAnalysis::NewPrePair()
{ 
  // Look at the pairing type to create a new pair object of the
  // correct type

  if (fPairType == FromPair)
    {
      fPrePair = new TaPairFromPair; 
#ifdef LEAKCHECK
      ++fLeakNewPair;
#endif
    }
  // Remove this ifdef when TaPairFromQuad class exists
#ifdef PAIRFROMQUAD
  else if (fPairType == FromQuad)
    {
      fPrePair = new TaPairFromQuad; 
#ifdef LEAKCHECK
      ++fLeakNewPair;
#endif
    }
#endif
  else
    {
      cerr << "VaAnalysis::NewPrePair ERROR "
           << "-- Invalid pair type " << endl;
      return fgVAANA_ERROR;
    }
  return fgVAANA_OK;
}


void
VaAnalysis::InitChanLists ()
{
  // Initialize the lists used by InitTree and AutoPairAna.  Default
  // implementation is empty.  Derived classes may override (or not).
}


void
VaAnalysis::InitTree ()
{
  // Initialize the pair tree with standard entries plus entries based
  // on the tree list.

  Int_t bufsize = 5000;

  //#define TREEPRINT
#ifdef TREEPRINT
  clog << "Adding to pair tree:" << endl;
  clog << "r_ev_num" << endl;
  clog << "l_ev_num" << endl;
  clog << "m_ev_num" << endl;
  clog << "ok_cond"  << endl;
  clog << "ok_cut"   << endl;
#endif

  size_t treeListSize = 0;
  for (vector<AnaList* >::const_iterator i = fTreeList.begin();
       i != fTreeList.end();
       ++i )
    {
      AnaList* alist = *i;
      if (alist->fFlagInt & fgCOPY)
	treeListSize += 2;
      if (alist->fFlagInt & fgDIFF)
	++treeListSize;
      if (alist->fFlagInt & fgASY)
	++treeListSize;
    }

  fTreeSpace = new Double_t[5+treeListSize];
  Double_t* tsptr = fTreeSpace;
  fPairTree->Branch ("r_ev_num", &fTreeREvNum, "r_ev_num/I", bufsize); 
  fPairTree->Branch ("l_ev_num", &fTreeLEvNum, "l_ev_num/I", bufsize); 
  fPairTree->Branch ("m_ev_num", &fTreeMEvNum, "m_ev_num/D", bufsize); 
  fPairTree->Branch ("ok_cond",  &fTreeOKCond, "ok_cond/I",  bufsize); 
  fPairTree->Branch ("ok_cut",   &fTreeOKCut,  "ok_cut/I",   bufsize); 

  // Add branches corresponding to channels in the channel lists
  string suff ("/D");
  
  for (vector<AnaList* >::const_iterator i = fTreeList.begin();
       i != fTreeList.end();
       ++i )
    {
      AnaList* alist = *i;
      if (alist->fFlagInt & fgCOPY)
	{
	  // Channels for which to copy right and left values to tree
	  string prefix_r ("r_");
	  string prefix_l ("l_");
	  fPairTree->Branch ((prefix_r+alist->fVarStr).c_str(), 
			     tsptr++, 
			     (prefix_r+alist->fVarStr+suff).c_str(), 
			     bufsize); 
	  fPairTree->Branch ((prefix_l+alist->fVarStr).c_str(), 
			     tsptr++, 
			     (prefix_l+alist->fVarStr+suff).c_str(), 
			     bufsize); 
#ifdef TREEPRINT
	  clog << (prefix_l+alist->fVarStr) << endl;
	  clog << (prefix_r+alist->fVarStr) << endl;
#endif
	}

      if (alist->fFlagInt & fgDIFF)
	{
	  // Channels for which to put difference in tree
	  string prefix ("diff_");
	  fPairTree->Branch ((prefix+alist->fVarStr).c_str(), 
			     tsptr++, 
			     (prefix+alist->fVarStr+suff).c_str(), 
			     bufsize); 
#ifdef TREEPRINT
	  clog << (prefix+alist->fVarStr) << endl;
#endif
	}

      if (alist->fFlagInt & fgASY)
	{
	  // Channels for which to put asymmetry in tree
	  string prefix = "asym_";
	  fPairTree->Branch ((prefix+alist->fVarStr).c_str(), 
			     tsptr++, 
			     (prefix+alist->fVarStr+suff).c_str(), 
			     bufsize); 
#ifdef TREEPRINT
	  clog << (prefix+alist->fVarStr) << endl;
#endif
	}
    }
}


vector<AnaList* >
VaAnalysis::ChanList (const string& devtype, const string& channel, const string& other, const UInt_t flags)
{
  // Create a channel list.
  //
  // Go through all devices in data map.  For each device of type
  // <devtype>, push onto the returned vector an AnaList consisting of
  // (<channel'>, <key>, <other>, <flags>) where <channel'> is
  // <channel> with "~" replaced by the device name and <key> is the
  // corresponding key.  This constitutes a list of data channels to
  // be processed (with the third element typically the units for the
  // analyzed quantity, and the fourth flags controlling how it's
  // handled).
  //
  // Example: if the data map contains two BPMs, bpm8 and bpm12, then
  // ChanList ("bpm", "~x", "um") will return: < <"bpm8x", key8x,
  // "um">, <"bpm12x", key12x, "um"> >.

  vector<AnaList* > chanlist;
  chanlist.clear();
  string chancopy1, chancopy2;
  chancopy1 = channel;

  fRun->GetDataBase().DataMapReStart();  // Re-initialize the datamap iterator
  while ( fRun->GetDataBase().NextDataMap() )
    {
      string devicetype (fRun->GetDataBase().GetDataMapType());
      if ( devicetype == devtype )
        {
          string devicename (fRun->GetDataBase().GetDataMapName());
          size_t i = channel.find("~");
          string channelp; 
          if (i < channel.length()) {
             chancopy2 = chancopy1;
             channelp  = chancopy1.replace (i, 1, devicename);
             chancopy1 = chancopy2;
          } else {
             channelp = channel;
          }
          chanlist.push_back ( new AnaList(channelp, fRun->GetKey(channelp), other, flags) );
        }
    }

  return chanlist;
}


void
VaAnalysis::AutoPairAna()
{
  // Routine a derived class can call to do some of the analysis
  // automatically.
  //
  // Place into the tree space copies, difference, and/or asymmetries
  // for channels listed in fTreeList, depending on flags.  Also put
  // these into the pair as labelled quantities.
  //
  // These lists are created in InitChanLists; different analyses can
  // produce different lists, then just call AutoPairAna to handle
  // some if not all of the pair analysis.
  //
  // At the end, call feedback routines if enabled.

#ifdef NOISY  
 clog<<" Entering AutoPairAna()"<<endl;
#endif

  Double_t* tsptr = fTreeSpace;

  // First store values not associated with a channel
  fTreeREvNum = fPair->GetRight().GetEvNumber();
  fTreeLEvNum = fPair->GetLeft().GetEvNumber();
  fTreeMEvNum = (fPair->GetRight().GetEvNumber()+
                fPair->GetLeft().GetEvNumber())*0.5;
  fTreeOKCond = (fPair->PassedCuts() ? 1 : 0);
  fTreeOKCut  = (fPair->PassedCutsInt(fRun->GetCutList()) ? 1 : 0);
#ifdef ASYMCHECK
  clog<<" mean ev pair "<<(fPair->GetRight().GetEvNumber()+fPair->GetLeft().GetEvNumber())*0.5<<" passed Cuts :"<<fPair->PassedCuts()<<endl;
#endif
  Double_t val;

  for (vector<AnaList* >::const_iterator i = fTreeList.begin();
       i != fTreeList.end();
       ++i )
    {
      AnaList* alist = *i;
      
      if (alist->fFlagInt & fgCOPY)
	{
	  // Channels for which to copy right and left values to tree
	  string prefix_r ("Right ");
	  string prefix_l ("Left  ");
	  
	  val = fPair->GetRight().GetData(alist->fVarInt);
	  *(tsptr++) = val;
	  fPair->AddResult (TaLabelledQuantity (prefix_r+(alist->fVarStr), 
						val, 
						alist->fUniStr,
						alist->fFlagInt));
	  val = fPair->GetLeft().GetData(alist->fVarInt);
	  *(tsptr++) = val;
	  fPair->AddResult (TaLabelledQuantity (prefix_l+(alist->fVarStr), 
						val, 
						alist->fUniStr,
						alist->fFlagInt));
	}

      if (alist->fFlagInt & fgDIFF)
	{
	  // Channels for which to put difference in tree
	  string prefix ("Diff ");

	  val = fPair->GetDiff(alist->fVarInt) * 1E3;
	  *(tsptr++) = val;
	  fPair->AddResult (TaLabelledQuantity (prefix+(alist->fVarStr), 
						val, 
						alist->fUniStr,
						alist->fFlagInt));
	}
      
      if (alist->fFlagInt & fgASY)
	{
	  // Channels for which to put asymmetry in tree
	  string prefix = "Asym ";

	  if ((alist->fFlagInt & fgNO_BEAM_NO_ASY) &&
	      (fPair->GetRight().BeamCut() ||
	       fPair->GetLeft().BeamCut()))
	    val = 0.0;
	  else
	    val = fPair->GetAsy(alist->fVarInt) * 1E6;
	  *(tsptr++) = val;
	  fPair->AddResult (TaLabelledQuantity (prefix+(alist->fVarStr), 
						val, 
						alist->fUniStr,
						alist->fFlagInt));
	}
    }

  if (fQSwitch) QasyRunFeedback();
  if (fZSwitch) PZTRunFeedback();
}

void 
VaAnalysis::QasyRunFeedback(){
  // Intensity feedback routine.

  if ( fQSwitch ){ 
    if ( fPair->PassedCuts() && fPair->PassedCutsInt(fRun->GetCutList()) ){
      fQNpair++;
      fQsum.push_back(fPair->GetAsy(IBCM1)*1E6);
      fQmean1+=fPair->GetAsy(IBCM1)*1E6;

#ifdef FDBK
      clog<<"\n\nVaAnalysis::QasyRunFeedback() compute IBCM1ASY : "<<fPair->GetAsy(IBCM1)*1E6
          <<" Npair "<<fQNpair<<" sum "<<fQmean1<<" current mean "<<fQmean1/fQNpair<<endl;
#endif
    }// end fPair->PassedCuts()
    if (fQNpair >= fQTimeScale*900){
      fQfeedNum++; 
      fQStopPair = fPair->GetLeft().GetEvNumber();
      
#ifdef FDBK1
      clog<<"\n";      
      clog<<"      |------ VaAnalysis::QasyRunFeedback()---------------\n";
      clog<<"      |\n";      
      clog<<"      | IBCM1ASY FEEDBACK #"<<fQfeedNum<<" at event "<<fQStopPair<<" Npair "<<fQNpair<<" \n";
      clog<<"      |---------------------------------------------------\n";
#endif

      fQmean1= fQmean1/fQNpair;
      vector< Double_t>::iterator qi;
      for ( qi = fQsum.begin(); qi != fQsum.end(); qi++){
        fQRMS+=(*qi - fQmean1)*(*qi -fQmean1);
      }
    fQRMS = sqrt(fQRMS/fQNpair);
    fQasy = fQmean1;
    fQasyEr = fQRMS/sqrt(fQNpair);

#ifdef FDBK1
    clog<<"      |\n";      
    clog<<"      | 1st pass <Aq> :  "<<fQasy<<" +- "<<fQasyEr<<" RMS "<<fQRMS<<"\n";
    clog<<"      |\n";
#endif
    fQNpair=0;
      for ( qi = fQsum.begin(); qi != fQsum.end() ; qi++){
        // if asymmetry value is not too far from the mean value 
        // (filter very large values )    
        // let's say < 6 sigma away from the calculated mean
        if ( abs(Int_t(*qi) - Int_t(fQasy)) < 6*fQRMS ) { 
             fQNpair++;
             fQmean2 += *qi;             
            }
          else
            {
             fQsum.erase(qi); 
        }
      } 

#ifdef FDBK1
    clog<<"      | Filtering processed, new Npair :"<<fQNpair<<"\n"; 
#endif
    fQmean2 = fQmean2/fQNpair;
    fQRMS=0;
    for (qi = fQsum.begin() ; qi != fQsum.end() ; qi++){
      fQRMS += (*qi - fQmean2)*(*qi - fQmean2);      
    }     
    fQRMS = sqrt(fQRMS/fQNpair); //  Qasym RMS
    fQasy = fQmean2;
    fQasyEr = fQRMS/sqrt(fQNpair); //  error on Qasym Mean
#ifdef FDBK1

    clog<<"      |\n";      
    clog<<"      | 2nd pass <Aq> : "<<fQasy<<" +- "<<fQasyEr<<" RMS "<<fQRMS<<" \n"; 
    clog<<"      |\n";
    clog<<"      |---------------------------------------------------\n";
    clog<<"\n\n";      

#endif
    fQNpair = 0;
    fQsum.clear();
    QasySendEPICS();
  
    }// end fQNpair >= fQTimeScale*900
 
  }// end fQSwitch        
}


void VaAnalysis::QasyEndFeedback(){
  // Last feedback at the end of the run. To have a chance to send 
  // a feedback, this function needs enough pairs.
  // It checks the QAsym vector size, computes an error and 
  // tests the value and sends it.
 if ( fQSwitch ){ 
  if (fQsum.size() > fQTimeScale ) {     
    fQNpair = fQsum.size();
    fQfeedNum = -1; // arbitrary end feedback number is -1
    fQStopPair = 0; // give 0 for end feedback 
    vector<Double_t>::iterator qi;
    for (qi = fQsum.begin() ; qi != fQsum.end() ; qi++){
      fQmean1+=*qi;
    }
#ifdef FDBK1
      clog<<"\n";      
      clog<<"      |------ VaAnalysis::QasyEndFeedback()---------------\n";
      clog<<"      |\n";      
      clog<<"      | IBCM1ASY END FEEDBACK queue size "<<fQsum.size()<<" \n";
      clog<<"      |---------------------------------------------------\n";
#endif
    
    for (qi = fQsum.begin() ; qi != fQsum.end() ; qi++){
      fQRMS += (*qi - fQmean1)*(*qi - fQmean1);      
    }     
    fQRMS = sqrt(fQRMS/fQNpair); //  Qasym RMS
    fQasy = fQmean1;
    fQasyEr = fQRMS/sqrt(fQNpair); //  error on Qasym Mean
    
#ifdef FDBK1
    clog<<"      |\n";      
    clog<<"      | 1st pass <Aq> :  "<<fQasy<<" +- "<<fQasyEr<<" RMS "<<fQRMS<<"\n";
    clog<<"      |\n";
#endif

    fQNpair = 0;    
    for ( qi = fQsum.begin(); qi != fQsum.end() ; qi++){
        // if asymmetry value is not too far from the mean value 
        // (filter very large values )    
        //  < 6 sigma away from the calculated mean
        if ( abs(Int_t(*qi) - Int_t(fQmean1)) < 6*fQRMS ) { 
             fQNpair++;
             fQmean2 += *qi;             
            }
          else
            {
             fQsum.erase(qi); 
        }
     } 
#ifdef FDBK1
    clog<<"      | Filtering processed, new Npair :"<<fQNpair<<"\n"; 
#endif
    fQmean2 = fQmean2/fQNpair;
    for (qi = fQsum.begin() ; qi != fQsum.end() ; qi++){
      fQRMS += (*qi - fQmean2)*(*qi - fQmean2);      
    }     
    fQRMS = sqrt(fQRMS/fQNpair); //  Qasym RMS
    fQasy = fQmean2;
    fQasyEr = fQRMS/sqrt(fQNpair); //  error on Qasym Mean
#ifdef FDBK1
    clog<<"      |\n";      
    clog<<"      | 2nd pass <Aq> : "<<fQasy<<" +- "<<fQasyEr<<" RMS "<<fQRMS<<" \n"; 
    clog<<"      |\n";
    clog<<"      |---------------------------------------------------\n";
    clog<<"\n\n";      
#endif
    fQNpair = 0;
    fQsum.clear();
    QasySendEPICS();
    }
  else{
    clog<<"*** NO END FEEDBACK proceeded, NOT ENOUGH PAIRS "<<endl;
  }  
 }
}

void VaAnalysis::SendVoltagePC(){
  // Send Pockels cell voltage
    if ( abs(Int_t(fQasy)) < 25 || abs(Int_t(fQasy)) > 25 && 
         abs(Int_t(fQasy)/(Int_t(fQasyEr))) > 2 ) {
      // CRITERIA are OK, compute PC low voltage value.  
#ifdef ONLINE      
      // will write the function when hardware defined ....
#endif
    }
}

void VaAnalysis::QasySendEPICS(){
  // Send intensity feedback value to EPICS
    if ( abs(Int_t(fQasy)) < 25 || abs(Int_t(fQasy)) > 25 && 
         abs(Int_t(fQasy)/(Int_t(fQasyEr))) > 2 ) {
        // CRITERIA are OK.  
      //      clog<<" fQasy and fQasy/fQasyEr :"<<fQasy<<" "<< abs(Int_t(fQasy)/(Int_t(fQasyEr)))<<endl;
      clog<<" \n             *** Pan is sending EPICS values for PC *** "<<endl;
      char* thevar[2];   
      thevar[0]= "HA:Q_ASY"; 
      thevar[1]= "HA:DQ_ASY";   
      char* thevalue[2];   
      char* command[2];
      for (Int_t i=0; i<2; i++){ 
      thevalue[i] = new char[100];
      command[i] = new char[100];
      }        
      sprintf(thevalue[0],"%6.0f",fQasy);
      sprintf(thevalue[1],"%6.0f",fQasyEr);
       for (Int_t i=0; i<2; i++){ 
        sprintf(command[i],"/pathtothescript/PITAfeedback_script ");
         strcat( command[i],thevar[i]);
         strcat(command[i]," ");
         strcat(command[i],thevalue[i]);
         
         clog << "   SHELL command = "<<command[i]<<endl;
#ifdef ONLINE
         //system(command[i]);
#endif            
         // delete thevar[i]; 
       delete thevalue[i];  
      delete command[i];           
      } 
   }
}

void VaAnalysis::PZTRunFeedback(){
  // Position feedback routine.
  // do it on IBPM4BX,Y but perhaps it is preferable for the future to 
  // do feedback at bpms at the injector in the 5 MeV region....if it is the case 
  // just replace the name of the bpm.  

  if (fZSwitch){ 
    if ( fPair->PassedCuts() && fPair->PassedCutsInt(fRun->GetCutList())){     
       fZNpair++;
       //       clog<<"good pair for PZT"<<endl;
       fZsum4B[0].push_back(fPair->GetDiff(IBPM4BX)*1E6);
       fZsum4B[1].push_back(fPair->GetDiff(IBPM4BY)*1E6);
       fZ4Bmean1[0]+=fPair->GetDiff(IBPM4BX)*1E6;
       fZ4Bmean1[1]+=fPair->GetDiff(IBPM4BY)*1E6;
       //       clog<<" data :"<<" "<<fZ4Bmean1[0]<<" "<<fZ4Bmean1[1]<<endl;
    }
   // start feedback after timescale*900
  if (fZNpair > fZTimeScale*900){
    fZfeedNum++;
    fZStopPair = fPair->GetLeft().GetEvNumber();
 
#ifdef FDBK1
    clog<<"\n      -------------VaAnalysis::QPZTRunFeedback()--------\n";
    clog<<"      |\n";
    clog<<"      | PZT mirror feedback #"<<fZfeedNum<<" at event "<<fZStopPair<<endl; 
    clog<<"      |\n";      
    clog<<"      |---------------------------------------------------\n";
    clog<<"      |\n";  
    clog<<"      | Matrix elements      (     |     )    \n";  
    clog<<"      |                      (-----|-----)    \n";      
    clog<<"      |                      (     |     )    \n";      
    clog<<"      |\n";      
    clog<<"      |---------------------------------------------------\n";  
#endif 

    for (Int_t i=0;i<2;i++){ 
       fZ4Bmean1[i]= fZ4Bmean1[i]/fZNpair;
       vector< Double_t>::iterator qi;
       for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end(); qi++){
           fZ4BRMS[i]+=(*qi - fZ4Bmean1[i])*(*qi -fZ4Bmean1[i]);
        }
       fZ4BRMS[i] = sqrt(fZ4BRMS[i]/fZNpair);
       fZ4Bdiff[i] = fZ4Bmean1[i];
       fZ4BdiffEr[i] = fZ4BRMS[i]/sqrt(fZNpair); 
    }
#ifdef FDBK1
    clog<<"      |\n";      
    clog<<"      | 1st pass "<<fZNpair<<" pairs \n";
    clog<<"      |\n";      
    clog<<"      |  <4BXdiff> = "<<fZ4Bdiff[0]<<"+-"<<fZ4BdiffEr[0]<<"\n";      
    clog<<"      |\n";      
    clog<<"      |  <4BYdiff> = "<<fZ4Bdiff[1]<<"+-"<<fZ4BdiffEr[1]<<"\n";      
#endif 
    for (Int_t i=0;i<2;i++){ 
        fZpair[i]=0;
       vector< Double_t>::iterator qi;
        for ( qi = fZsum4B[i].begin(); 
             qi != fZsum4B[i].end() ; qi++){
           // if diff value is not too far from the mean value
           // (filter very large values )
           // let's say < 6 sigma away from the calculated mean
        if ( abs(Int_t(*qi) - Int_t(fZ4Bdiff[i])) < 6*fZ4BRMS[i] ) {
             fZpair[i]++; 
             fZ4Bmean2[i] += *qi;
            }
          else
            {
             fZsum4B[i].erase(qi);
        } 
      }
    fZ4Bmean2[i]= fZ4Bmean2[i]/fZpair[i];
    for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end(); qi++){
        fZ4BRMS[i]+=(*qi - fZ4Bmean2[i])*(*qi -fZ4Bmean2[i]);
      }
    fZ4BRMS[i] = sqrt(fZ4BRMS[i]/fZpair[i]);
    fZ4Bdiff[i] = fZ4Bmean2[i];
    fZ4BdiffEr[i] = fZ4BRMS[i]/sqrt(fZpair[i]);
    } 
#ifdef FDBK1
    clog<<"      |\n";      
    clog<<"      | 2nd pass X "<<fZpair[0]<<" pairs, Y "<<fZpair[1]<<" pairs      \n";
    clog<<"      |\n";      
    clog<<"      |  <4BXdiff> = "<<fZ4Bdiff[0]<<"+-"<<fZ4BdiffEr[0]<<"\n";      
    clog<<"      |\n";      
    clog<<"      |  <4BYdiff> = "<<fZ4Bdiff[1]<<"+-"<<fZ4BdiffEr[1]<<"           \n";
    clog<<"      |\n";      
    clog<<"      ----------------------------------------------------\n";
    clog<<"\n\n";      
#endif 
    fZNpair=0;  
    for (Int_t i=0;i<2;i++){ 
      fZpair[i]=0;
      fZsum4B[i].clear();  
    }
   PZTSendEPICS();  
  }

 }
}


void VaAnalysis::PZTEndFeedback(){
  // Position feedback at end of run
  if (fZSwitch){
     if ((fZsum4B[0].size() > fQTimeScale*900) && 
         (fZsum4B[1].size() > fQTimeScale*900)){     
         fZfeedNum = -1; // arbitrary end feedback number is -1
         fZStopPair = 0; // give 0 for end feedback 
         vector<Double_t>::iterator qi;
       for ( Int_t i = 0;i<2;i++){
         fZpair[i] = fZsum4B[i].size();
         for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end(); qi++){
           fZ4Bmean1[i]+= *qi;
         }
       } 
#ifdef FDBK1

       clog<<"\n-------------VaAnalysis::QPZTEndFeedback()---------\n";
       clog<<"|                                                   |\n";
       clog<<"| PZT mirror feedback at END of RUN "<<fRunNum<<"       | \n"; 
       clog<<"|                                                   |\n";      
       clog<<"|---------------------------------------------------|\n";
       clog<<"| Matrix elements      (      |     )               |\n";  
       clog<<"|                      (------------)               |\n";      
       clog<<"|                      (      |     )               |\n";      
       clog<<"|---------------------------------------------------|\n";

#endif 
    for (Int_t i=0;i<2;i++){ 
       fZ4Bmean1[i]= fZ4Bmean1[i]/fZpair[i];
       for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end(); qi++){
           fZ4BRMS[i]+=(*qi - fZ4Bmean1[i])*(*qi -fZ4Bmean1[i]);
        }
       fZ4BRMS[i] = sqrt(fZ4BRMS[i]/fZpair[i]);
       fZ4Bdiff[i] = fZ4Bmean1[i];
       fZ4BdiffEr[i] = fZ4BRMS[i]/sqrt(fZpair[i]); 
    }
#ifdef FDBK1
    clog<<"|                                                   |\n";      
    clog<<"| 1st pass X "<<fZpair[0]<<" pairs,  Y "<<fZpair[1]<<" pairs    |\n";
    clog<<"|                                                   |\n";      
    clog<<"|  <4BXdiff> = "<<fZ4Bdiff[0]<<"+-"<<fZ4BdiffEr[0]<<"           |\n";      
    clog<<"|                                                   |\n";      
    clog<<"|  <4BYdiff> = "<<fZ4Bdiff[1]<<"+-"<<fZ4BdiffEr[1]<<"           |\n";      
#endif 
    for (Int_t i=0;i<2;i++){ 
        fZpair[i]=0;
        for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end() ; qi++){
           // if diff value is not too far from the mean value
           // (filter very large values )
           // let's say < 6 sigma away from the calculated mean
           if ( abs(Int_t(*qi) - Int_t(fZ4Bdiff[i])) < 6*fZ4BRMS[i] ) {
               fZpair[i]++; 
               fZ4Bmean2[i] += *qi;
             }
           else
             {
             fZsum4B[i].erase(qi);
          }
        }  
        fZ4Bmean2[i]= fZ4Bmean2[i]/fZpair[i];
        for ( qi = fZsum4B[i].begin(); qi != fZsum4B[i].end(); qi++){
          fZ4BRMS[i]+=(*qi - fZ4Bmean2[i])*(*qi -fZ4Bmean2[i]);
        }
        fZ4BRMS[i] = sqrt(fZ4BRMS[i]/fZpair[i]);
        fZ4Bdiff[i] = fZ4Bmean2[i];
        fZ4BdiffEr[i] = fZ4BRMS[i]/sqrt(fZpair[i]);
    } 
#ifdef FDBK1
    clog<<"|                                                   |\n";      
    clog<<"| 2st pass X "<<fZpair[0]<<" pairs, Y "<<fZpair[1]<<" pairs     |\n";
    clog<<"|                                                   |\n";      
    clog<<"|  <4BXdiff> = "<<fZ4Bdiff[0]<<"+-"<<fZ4BdiffEr[0]<<"           |\n";      
    clog<<"|                                                   |\n";      
    clog<<"|  <4BYdiff> = "<<fZ4Bdiff[1]<<"+-"<<fZ4BdiffEr[1]<<"           |\n";
    clog<<"|                                                   |\n";      
    clog<<"-----------------------------------------------------\n";
    clog<<"\n\n";      
#endif 
    for (Int_t i=0;i<2;i++){ 
      fZpair[i]=0;
      fZsum4B[i].clear();
    }
    PZTSendEPICS();  
     }
  }
}

void VaAnalysis::PZTSendEPICS(){
  // Send position feedback value to EPICS

  // arbitrary values for now
    if ( (abs(Int_t(fZ4Bdiff[0])) < 1000 || abs(Int_t(fZ4Bdiff[0])) > 1000 && 
         abs(Int_t(fZ4Bdiff[0])/(Int_t(fZ4BdiffEr[0]))) > 2) &&
         (abs(Int_t(fZ4Bdiff[1])) < 1000 || abs(Int_t(fZ4Bdiff[1])) > 1000 && 
         abs(Int_t(fZ4Bdiff[1])/(Int_t(fZ4BdiffEr[1]))) > 2)) {

      clog<<" \n             *** Pan is sending EPICS values for PZT *** "<<endl;
      char* thevar[4];   
      thevar[0]= "HA:PZT_4BX"; 
      thevar[1]= "HA:PZT_D4BX";   
      thevar[2]= "HA:PZT_4BY"; 
      thevar[3]= "HA:PZT_D4BY";   
      char* thevalue[4];   
      char* command[4];
      for (Int_t i=0; i<4; i++){ 
      thevalue[i] = new char[100];
      command[i] = new char[100];
      }        
      sprintf(thevalue[0],"%6.0f",fZ4Bdiff[0]);
      sprintf(thevalue[1],"%6.0f",fZ4BdiffEr[0]);      
      sprintf(thevalue[2],"%6.0f",fZ4Bdiff[1]);
      sprintf(thevalue[3],"%6.0f",fZ4BdiffEr[1]);
       for (Int_t i=0; i<4; i++){ 
        sprintf(command[i],"/pathtothescript/PZTfeedback_script ");
         strcat( command[i],thevar[i]);
         strcat(command[i]," ");
         strcat(command[i],thevalue[i]);
         
         clog << "   SHELL command = "<<command[i]<<endl;
#ifdef ONLINE
         //system(command[i]);
#endif            
         // delete thevar[i]; 
       delete thevalue[i];  
      delete command[i];            
       }
    }
}


void VaAnalysis::SendVoltagePZT(){
  // Send position feedback value
  // will write the function when hardware defined ....
}

#ifdef LEAKCHECK
void VaAnalysis::LeakCheck()
{
  // Check for memory leaks in VaAnalysis
  cerr << "Memory leak test for VaAnalysis:" << endl;
  cerr << "Events new " << fLeakNewEvt
       << " del " << fLeakDelEvt
       << " diff " <<fLeakNewEvt-fLeakDelEvt << endl;
  cerr << "Pairs  new " << fLeakNewPair
       << " del " << fLeakDelPair
       << " diff " <<fLeakNewPair-fLeakDelPair << endl;
}
#endif







