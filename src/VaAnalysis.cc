//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           VaAnalysis.cc   (implementation)
//           ^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    Analysis base class.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define CHECKOUT
//#define ASYMCHECK

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
  fTreePairOK(0),
  fTreeSpace(0),
  fPairType(FromPair)
{ 
  fEHelDeque.clear(); 
  fEDeque.clear(); 
  fPDeque.clear();
  fCopyList.clear();
  fDiffList.clear();
  fAsymList.clear();
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


void 
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
  fPair = 0;

// Define CHECKOUT if you want a printout of the database.  This might be worth
// leaving in permanently, but should at least be checked sometimes.
#ifdef CHECKOUT
  cout << "\n\nCHECKOUT of DATABASE for Run "<<fRun->GetRunNumber()<<endl;
  cout << "Run type  = "<<fRun->GetDataBase()->GetRunType()<<endl;
  cout << "Max events = "<<fRun->GetDataBase()->GetMaxEvents()<<endl;
  cout << "lobeam  cut = "<<fRun->GetDataBase()->GetCutValue("LOBEAM")<<endl;
  cout << "burpcut  cut = "<<fRun->GetDataBase()->GetCutValue("BURPCUT")<<endl;
  cout << "window delay = "<<fRun->GetDataBase()->GetDelay()<<endl;
  cout << "oversampling factor = "<<fRun->GetDataBase()->GetOverSamp()<<endl;
  cout << "pair type (i.e. pair or quad) =  "<<fRun->GetDataBase()->GetPairType()<<endl;
  cout << "\n\nPedestal and Dac noise parameters by ADC# and channel# : "<<endl;
  for (int adc = 0; adc < 10 ; adc++) {
    cout << "\n\n-----  For ADC "<<adc<<endl;
    for (int chan = 0; chan < 4; chan++) {
      cout << "\n  channel "<<chan;
      cout << "   ped = "<<fRun->GetDataBase()->GetPedestal(adc,chan);
      cout << "   dac slope = "<<fRun->GetDataBase()->GetDacNoise(adc,chan,"slope");
      cout << "   dac int = "<<fRun->GetDataBase()->GetDacNoise(adc,chan,"int");
    }
  }  
  cout << "\n\nNumber of cuts "<<fRun->GetDataBase()->GetNumCuts()<<endl;
  vector<Int_t> evlo = fRun->GetDataBase()->GetEvLo();
  vector<Int_t> evhi = fRun->GetDataBase()->GetEvHi();
  for (int i = 0; i<fRun->GetDataBase()->GetNumCuts(); i++) { 
    if (i >= (long)evhi.size()) cout << "evlo and evhi mismatched size"<<endl;
    cout << "Cut "<<i<<"   Evlo  = "<<evlo[i]<<"  Evhi = "<<evhi[i]<<endl;
  }  
  cout << "\n\nNum cut event intervals "<<fRun->GetDataBase()->GetNumBadEv()<<endl;
  map<Int_t, vector<Int_t> > evint = fRun->GetDataBase()->GetCutValues();
  Int_t k = 0;
  for (map<Int_t, vector<Int_t> >::iterator icut = evint.begin();
     icut != evint.end(); icut++) {
     vector<Int_t> cutint = icut->second;
     cout << "Cut interval "<<dec<< k++;
     cout << "  from event "<< cutint[0] << " to "<<cutint[1];
     cout << "  mask "<<cutint[2]<<"   veto "<<cutint[3]<<endl;
  }
#endif  
// ----------- end of database checkout (ifdef)

  // Get maximum events to analyze from the database
  EventNumber_t mx = 0;
  mx = fRun->GetDataBase()->GetMaxEvents();
#ifdef THIS_IS_OLD_CODE
  // As a way to limit events analyzed without the above mechanism in
  // place, check an environment variable.
  char* mxev;
  mxev = getenv ("CODA_MAXEVENTS");
  if (mxev != 0)
    {
      mx = atoi (mxev);
      if (mx > 0)
	clog << "Limiting analysis to " << mxev << " events "
	     << "(from env. var. CODA_MAXEVENTS)" << endl;
    }
#endif
  if (mx > 0) {
    fMaxNumEv = mx;
    cout << "\nLimiting analysis to " << fMaxNumEv << "  events "<<endl;
  }

  // maximum events in fEHelDeque set equal to helicity delay times
  // oversample plus one. 

  fEHelDequeMax = fRun->GetDataBase()->GetDelay() * 
    fRun->GetDataBase()->GetOverSamp() + 1;

  // maximum events in fEDeque set equal to twice number of
  // events per second.  Half as many pairs in fPDeque.
  fPDequeMax = fRun->GetRate(); 
  fEDequeMax = fPDequeMax + fPDequeMax;
  if (fPDequeMax == 0)
    {
      cerr << "VaAnalysis::RunIni ERROR: Trigger rate is zero, cannot analyze"
	   << endl;
      exit (1);
    }
  
  fPairTree = 0;
  fPairTree = new TTree("P","Pair data DST");
  InitChanLists();
  InitTree();

  // Set pair type

  string type = fRun->GetDataBase()->GetPairType();
  
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
}


void
VaAnalysis::ProcessRun()
{

  // Main analysis routine -- this is the event loop

  while ( fRun->NextEvent() )
    {
      PreProcessEvt();
      fRun->AddCuts(); // revise cut intervals
      if ( fEDeque.size() == fEDequeMax )
	{
	  ProcessEvt();
	  fRun->AccumEvent (*fEvt);
	}
      if ( fPDeque.size() == fPDequeMax )
	{
	  ProcessPair();
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
      ProcessEvt();
      fRun->AccumEvent (*fEvt);
    }
  while ( fPDeque.size() > 0 )
    {
      ProcessPair();
      fRun->AccumPair (*fPair);
    }

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


void
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
        if ( fPrePair->Fill (*fPreEvt) )
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
}


void
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
}


void
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
      clog << " BCM1(L, R) "<<fPair->GetLeft().GetData("bcm1")<<" "<<fPair->GetRight().GetData("bcm1")<<endl;
      clog << " BCM2(L, R) "<<fPair->GetLeft().GetData("bcm2")<<" "<<fPair->GetRight().GetData("bcm2")<<endl;
      clog << " BCM asy " <<fPair->GetAsy("bcm1")*1E6 <<" "<<fPair->GetAsy("bcm2")*1E6<<endl;
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
      exit (1);
    }
#ifdef NOISY
  clog << "Leaving ProcessPair, queue sizes " << fEHelDeque.size()
       << " (" << fEHelDequeMax
       << ") " << fEDeque.size()
       << " (" << fEDequeMax
       << ") " << fPDeque.size()
       << " (" << fPDequeMax << ")" << endl;
#endif
}


void
VaAnalysis::NewPrePair()
{ 
  // look at the pairing type to create a new pair object of the correct type  

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
      exit (1);
    }
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
  // Initialize the pair tree 
  Int_t bufsize = 5000;

  fTreeSpace = new Double_t[4+
			   fCopyList.size()*2+
			   fDiffList.size()+
			   fAsymList.size()];
#define TREEPRINT
#ifdef TREEPRINT
  clog << "Adding to pair tree:" << endl;
  clog << "r_ev_num" << endl;
  clog << "l_ev_num" << endl;
  clog << "m_ev_num" << endl;
  clog << "pair_ok"  << endl;
#endif

  Double_t* tsptr = fTreeSpace;
  fPairTree->Branch ("r_ev_num", &fTreeREvNum, "r_ev_num/I", bufsize); 
  fPairTree->Branch ("l_ev_num", &fTreeLEvNum, "l_ev_num/I", bufsize); 
  fPairTree->Branch ("m_ev_num", &fTreeMEvNum, "m_ev_num/D", bufsize); 
  fPairTree->Branch ("pair_ok",  &fTreePairOK, "pair_ok/I",  bufsize); 

  // Add branches corresponding to channels in the channel lists
  string suff ("/D");
  
  // Channels for which to copy right and left values to tree
  string prefix_r ("r_");
  string prefix_l ("l_");
  for (vector<pair<string,string> >::const_iterator i = fCopyList.begin();
       i != fCopyList.end();
       ++i )
    {
      fPairTree->Branch ((prefix_r+i->first).c_str(), 
			 tsptr++, 
			 (prefix_r+i->first+suff).c_str(), 
			 bufsize); 
#ifdef TREEPRINT
      clog << (prefix_r+i->first) << endl;
#endif

      fPairTree->Branch ((prefix_l+i->first).c_str(), 
			 tsptr++, 
			 (prefix_l+i->first+suff).c_str(), 
			 bufsize); 
#ifdef TREEPRINT
      clog << (prefix_l+i->first) << endl;
#endif

    }

  // Channels for which to put difference in tree
  string prefix ("diff_");
  for (vector<pair<string,string> >::const_iterator i = fDiffList.begin();
       i != fDiffList.end();
       ++i )
    {
      fPairTree->Branch ((prefix+i->first).c_str(), 
			 tsptr++, 
			 (prefix+i->first+suff).c_str(), 
			 bufsize); 
#ifdef TREEPRINT
      clog << (prefix+i->first) << endl;
#endif

    }

  // Channels for which to put asymmetry in tree
  prefix = "asym_";
  for (vector<pair<string,string> >::const_iterator i = fAsymList.begin();
       i != fAsymList.end();
       ++i )
    {
      fPairTree->Branch ((prefix+i->first).c_str(), 
			 tsptr++, 
			 (prefix+i->first+suff).c_str(), 
			 bufsize); 
#ifdef TREEPRINT
      clog << (prefix+i->first) << endl;
#endif

    }
}


vector<pair<string,string> >
VaAnalysis::ChanList (const string& devtype, const string& channel, const string& other)
{
  // Create a channel list.

  // Go through all devices in data map.  For each device of type
  // <devtype>, push onto the returned vector the pair (<channel'>,
  // <other>) where <channel'> is <channel> with "~" replaced by the
  // device name.  This constitutes a list of data channels to be
  // processed (with the second element typically the units for the
  // analyzed quantity).
  //
  // Example: if the data map contains two BPMs, bpm8 and bpm12, then
  // ChanList ("bpm", "~x", "um") will return: < <"bpm8x","um">,
  // <"bpm12x", "um"> >.

  vector<pair<string,string> > chanlist;
  string chancopy1, chancopy2;
  chancopy1 = channel;

  fRun->GetDataBase()->DataMapReStart();  // Re-initialize the datamap iterator
  while ( fRun->GetDataBase()->NextDataMap() )
    {
      string devicetype (fRun->GetDataBase()->GetDataMapType());
      if ( devicetype == devtype )
	{
	  string devicename (fRun->GetDataBase()->GetDataMapName());
          size_t i = channel.find("~");
          string channelp; 
	  if (i < channel.length()) {
             chancopy2 = chancopy1;
             channelp  = chancopy1.replace (i, 1, devicename);
             chancopy1 = chancopy2;
	  } else {
 	     channelp = channel;
	  }
	  chanlist.push_back (make_pair (channelp, other));
	}
    }

  return chanlist;
}


void
VaAnalysis::AutoPairAna()
{
#ifdef NOISY  
 clog<<" Entering AutoPairAna()"<<endl;
#endif
  // Routine a derived class can call to do some of the analysis
  // automatically.

  // Place into the tree space copies, difference, and asymmetries for
  // channels listed in fCopyList, fDiffList, and fAsymList.  Also
  // put these into the pair as labelled quantities.

  // These lists are created in InitChanLists; different analyses can
  // produce different lists, then just call AutoPairAna to handle
  // some if not all of the pair analysis.


   

  Double_t* tsptr = fTreeSpace;

  // First store values not associated with a channel
  fTreeREvNum = fPair->GetRight().GetEvNumber();
  fTreeLEvNum = fPair->GetLeft().GetEvNumber();
  fTreeMEvNum = (fPair->GetRight().GetEvNumber()+
		fPair->GetLeft().GetEvNumber())*0.5;
  fTreePairOK = (fPair->PassedCuts() ? 1 : 0);
#ifdef ASYMCHECK
  //  cout<<" mean ev pair "<<(fPair->GetRight().GetEvNumber()+fPair->GetLeft().GetEvNumber())*0.5<<" passed Cuts :"<<fPair->PassedCuts()<<endl;
#endif
  Double_t val;

  // Channels for which to copy right and left values to tree
  string prefix_r ("Right ");
  string prefix_l ("Left  ");


#ifdef NOISY
  clog << " --------- List of analysis ------------" << endl;
  clog << "    COPY         " << endl;
#endif


  for (vector<pair<string,string> >::const_iterator i = fCopyList.begin();
       i != fCopyList.end();
       ++i )
    {
      val = fPair->GetRight().GetData(i->first);
      *(tsptr++) = val;
      fPair->AddResult ( TaLabelledQuantity ( prefix_r+(i->first), 
					     val, i->second ) );
      val = fPair->GetLeft().GetData(i->first);
      *(tsptr++) = val;
      fPair->AddResult ( TaLabelledQuantity ( prefix_l+(i->first), 
					     val, i->second ) );
#ifdef NOISY
  clog <<val<<endl;
#endif
    }

#ifdef NOISY
  clog << "    DIFF         " << endl;
  clog <<val<<endl;
#endif
  // Channels for which to put difference in tree
  string prefix ("Diff ");
  for (vector<pair<string,string> >::const_iterator i = fDiffList.begin();
       i != fDiffList.end();
       ++i )
    {
      val = fPair->GetDiff(i->first) * 1E3;
      *(tsptr++) = val;
      fPair->AddResult ( TaLabelledQuantity ( prefix+(i->first), 
					     val, i->second ) );
#ifdef NOISY
  clog <<val<<endl;
#endif
    }

#ifdef NOISY
  clog << "    ASYM         " << endl;
#endif
  // Channels for which to put asymmetry in tree
  prefix = "Asym ";
  for (vector<pair<string,string> >::const_iterator i = fAsymList.begin();
       i != fAsymList.end();
       ++i )
    {
      val = fPair->GetAsy(i->first) * 1E6;
      *(tsptr++) = val;
      fPair->AddResult ( TaLabelledQuantity ( prefix+(i->first), 
					     val, i->second ) );
#ifdef NOISY
   clog<<i->first<<endl; 
   clog <<val<<endl;
#endif
    }

}

#ifdef LEAKCHECK
void VaAnalysis::LeakCheck()
{
  cerr << "Memory leak test for VaAnalysis:" << endl;
  cerr << "Events new " << fLeakNewEvt
       << " del " << fLeakDelEvt
       << " diff " <<fLeakNewEvt-fLeakDelEvt << endl;
  cerr << "Pairs  new " << fLeakNewPair
       << " del " << fLeakDelPair
       << " diff " <<fLeakNewPair-fLeakDelPair << endl;
}
#endif


