//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRun.cc   (implementation)
//           ^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//    A run of data (where 'run' is defined by CODA).  A run is 
//    typically a 1 hour period where the setup parameters are fixed.
//
//////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define CHECKOUT 

#include "TaRun.hh"
#include <iostream>
#include <stdlib.h>
#include "THaCodaData.h"
#include "THaCodaFile.h"
#include "TaAsciiDB.hh"
#include "TaCutList.hh"
#include "TaEvent.hh"
#include "TaDevice.hh"
#include "TaLabelledQuantity.hh"
#include "TaStatistics.hh"
#include "VaDataBase.hh"
#include "VaPair.hh"
#ifdef ONLINE
#include "THaEtClient.h"
#endif

#ifdef DICT
ClassImp(TaRun)
#endif

// Constructors/destructors/operators
TaRun::TaRun():
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fSliceLimit(fgSLICELENGTH)
{
   fCodaFileName = "online";
#ifdef ONLINE
   int mymode    = 1;
   fComputer     = "adaqcp";  // DAQ computer
   fSession      = "par1";    // CODA $SESSION
   fCoda         = new THaEtClient();
#else
   cerr << "TaRun:: ERROR: Default c'tor implies online data."<<endl;
   cerr << "You must compile with ET library in this case."<<endl;
#endif 
};

TaRun::TaRun(const Int_t& run) : 
  fRunNumber(run),
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fSliceLimit(fgSLICELENGTH)
{
  char *prefix, *suffix, *crun;
  prefix = getenv("CODA_FILE_PREFIX");
  suffix = getenv("CODA_FILE_SUFFIX");
  if (prefix == NULL || suffix == NULL) {
    cerr << "TaRun:: ERROR: You must define env. variables to obtain filename"<<endl;
    cerr << "Example:  CODA_FILE_PREFIX = /adaql2/data2/parity01"<<endl;
    cerr << "Example:  CODA_FILE_SUFFIX = dat"<<endl;
    fCodaFileName = "nothing";
    return;
  }
  char *fname = new char[strlen(prefix)+strlen(suffix)+20];
  strcpy(fname,prefix);
  crun   = new char[10];
  sprintf(crun,"_%d.",fRunNumber);
  strcat(fname,crun);
  strcat(fname,suffix);
  fCodaFileName = fname;
} 

TaRun::TaRun(const string& filename):
  fRunNumber(0),
  fCodaFileName(filename),
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fSliceLimit(fgSLICELENGTH)
{
};

void
TaRun::Init()
{
  clog << "TaRun::Init Initialization for run, analyzing " 
       << fCodaFileName << endl;

  evtree = new TTree("R","Event data DST");
  evtree->Branch ("ev_num", &fEventNumber, "ev_num/I", 5000); // event number

  if (fCodaFileName == "online") { 
#ifdef ONLINE
     fCoda = new THaEtClient();
     if ( fCoda->codaOpen(fComputer, fSession, mymode) != 0) {
        cerr << "TaRun:: InitERROR: Cannot open ET connection"<<endl;
        cerr << " to  computer "<<fComputer;
        cerr << " and session "<<fSession<<endl;
        return TARUN_ERROR;
     }
#else
     cerr << "TaRun:: Init ERROR: Undefined online input."<<endl;
     return;
#endif
  } else { 
     TString tfile(fCodaFileName.c_str()); // hopefully temp. prefer <string>
     fCoda = new THaCodaFile(tfile);
  }

  // Get first event
  // For a data file this will normally be a PRESTART event.

  fEvent = new TaEvent();
  if (GetBuffer() != 0)
    {
      cerr << "TaRun::Init ERROR: No data from " << fCodaFileName << endl;
      exit (1);
    }
  fRunNumber = FindRunNumber();
  if (fRunNumber == 0)
    {
      cerr << "TaRun::Init WARNING: Run number zero."<<endl;
    }
  else
    {
      cerr << "TaRun::Init Run number is " << fRunNumber << endl;
    }
    
  fDataBase = new TaAsciiDB();
  fDataBase->Load(fRunNumber);
  fDevices = new TaDevice();
  fDevices->Init(*fDataBase);
  fCutList = new TaCutList(fRunNumber);
  fCutList->Init(*fDataBase);
  InitDevices();
  fOversamp = fDataBase->GetOverSamp();
  if (fOversamp == 0)
    {
      cerr << "TaRun::Init ERROR: Oversample factor is zero, cannot analyze"
	   << endl;
      exit (1);
    }

}
 

Int_t TaRun::FindRunNumber() {
   if (fCodaFileName == "online") {
      ifstream runfile(getenv("RUNNUMBER_FILE"));
      if ( !runfile ) return 0;
      string sinput;
      getline(runfile,sinput);
      return atoi(sinput.c_str());
   } 
// 1st event is always prestart for an unfiltered CODA file.
// If (somehow) not, I suppose we could try to find one, but
// for now we return the presumed fRunNumber.
   if ( fEvent->IsPrestartEvent() ) {
      UInt_t runno = fEvent->GetRawData(3);
      if (fRunNumber != 0 && runno != fRunNumber) {
        cerr<<"TaRun:: WARNING: Run number in file not what you thought."<<endl;
	cerr << "Expected " << fRunNumber << ", found " << runno << endl;
      }  
      return runno;
   } else {
      return fRunNumber;
   }
};
      


Bool_t 
TaRun::NextEvent()
{
  // Get the next physics event out of the coda file.
  // If end of file or error return false.
  Bool_t gotPhys = false;
  while (!gotPhys)
    {
      Int_t status = GetBuffer();
      if (status == -1)
	{
	  clog << "TaRun::NextEvent Normal end of CODA data" << endl;
	  return false;
	}
      else if (status != 0)
	{
	  cerr << "TaRun::NextEvent Abnormal CODA status" << endl;
	  return false;
	}
      gotPhys = fEvent->IsPhysicsEvent();
    }
  Decode();     // Seems like a good place to put this.
  return true;
}


void 
TaRun::Decode()
{
   fEvent->Decode(*fDevices);
   fEventNumber = fEvent->GetEvNumber();
   evtree->Fill();
// Use this to make detailed checks of decoding:
#ifdef CHECKOUT
   fEvent->RawDump();
   fEvent->DeviceDump();
#endif
}


void 
TaRun::AccumEvent(const TaEvent& ev) 
{ 
  vector<TaLabelledQuantity> lqres = ev.GetResults();
  if (fESliceStats == 0 && lqres.size() > 0)
    {
      // Set up statistics
#ifdef NOISY
      clog << "Setting up event stats - " << lqres.size() << endl;
#endif
      fESliceStats = new TaStatistics (lqres.size(), false);
      fERunStats = new TaStatistics (lqres.size(), false);
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  fEStatsNames.push_back (i->GetName());
	  fEStatsUnits.push_back (i->GetUnits());
	}
    }

  if ( fCutList->OK(ev) )
    {
      // Event passes cuts...
      // Update statistics
      vector<Double_t> vres;
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  vres.push_back (i->GetVal());
	}
      fESliceStats->Update (vres);
    }
#ifdef NOISY
  else
    clog << "Event " << ev.GetEvNumber() << " is in cut interval" << endl;
#endif
}


void 
TaRun::AccumPair(const VaPair& pr) 
{ 
  vector<TaLabelledQuantity> lqres = pr.GetResults();
  if (fPSliceStats == 0 && lqres.size() > 0)
    {
      // Set up statistics
#ifdef NOISY
      clog << "Setting up pair stats - " << lqres.size() << endl;
#endif
      fPSliceStats = new TaStatistics (lqres.size(), false);
      fPRunStats = new TaStatistics (lqres.size(), false);
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  fPStatsNames.push_back (i->GetName());
	  fPStatsUnits.push_back (i->GetUnits());
	}
    }

  if ( fCutList->OK(pr.GetRight()) && fCutList->OK(pr.GetLeft()) )
    {
      // Both events pass cuts...
      // Update statistics
      vector<Double_t> vres;
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  vres.push_back (i->GetVal());
	}
      fPSliceStats->Update (vres);
    }
#ifdef NOISY
  else
    clog << "Pair " << pr.GetRight().GetEvNumber() 
	 << "/" <<  pr.GetLeft().GetEvNumber() 
	 << " is in cut interval" << endl;
#endif

  // Print a slice and reset its statistics
  EventNumber_t evr = pr.GetRight().GetEvNumber();
  EventNumber_t evl = pr.GetLeft().GetEvNumber();
  if (evr >= fSliceLimit || evl >= fSliceLimit)
    {
      fSliceLimit += fgSLICELENGTH;
      clog << "TaRun::AccumEvent(): At pair " << evr << "/" << evl
	   << " of run " << fRunNumber;
      if (fESliceStats != 0 || fPSliceStats != 0)
	clog << " -- Stats for last " << fgSLICELENGTH
	     << " events:";
      clog << endl;

      if (fESliceStats != 0)
	{
	  PrintStats (*fESliceStats, fEStatsNames, fEStatsUnits);
	  *fERunStats += *fESliceStats;
	  fESliceStats->Zero();
	}
      if (fPSliceStats != 0)
	{
	  PrintStats (*fPSliceStats, fPStatsNames, fPStatsUnits);
	  *fPRunStats += *fPSliceStats;
	  fPSliceStats->Zero();
	}
      clog << endl;
    }
}

void 
TaRun::AddCutToEvent(const ECutType cut, const Int_t val)
{
  fEvent->AddCut (cut, val);
}

void 
TaRun::AddCuts() 
{ 
  for ( vector< pair<ECutType,Int_t> >::const_iterator i = fEvent->GetCuts().begin();
	i != fEvent->GetCuts().end();
	++i )
    fCutList->UpdateCutInterval ( i->first, i->second, fEvent->GetEvNumber() );
  for ( vector< pair<ECutType,Int_t> >::const_iterator i = fEvent->GetCutsPassed().begin();
	i != fEvent->GetCutsPassed().end();
	++i )
    fCutList->UpdateCutInterval ( i->first, i->second, fEvent->GetEvNumber() );
}

void TaRun::SendEPICSInfo( pair< char* , Double_t> value)
{
  //#ifdef ONLINE 
  char* thevar = value.first;
  char* thevalue = new char[100];
  char* command = new char[100];
  sprintf(thevalue,"%6.0f",value.second);
  sprintf(command,"/pathtothescript/feedback_script ");
  strcat( command,thevar);
  strcat(command," ");
  strcat(command,thevalue);
  clog << "SHELL command = "<<command<<endl<<flush; 
  system(command);
  delete [] thevar;delete []thevalue; delete [] command;
  //#endif
}


void 
TaRun::Finish() 
{ 
  clog << "\nTaRun::Finish End of run " << fRunNumber;
  size_t nSlice = fEventNumber - (fSliceLimit - fgSLICELENGTH);
  if ((fESliceStats != 0 || fPSliceStats != 0) && nSlice > 5)
    clog<< " -- Stats for last " << nSlice << " events:";
  clog << endl;

  if (fESliceStats != 0 && nSlice > 5)
    PrintStats (*fESliceStats, fEStatsNames, fEStatsUnits);
  if (fPSliceStats != 0 && nSlice > 5)
    PrintStats (*fPSliceStats, fPStatsNames, fPStatsUnits);

  if (fESliceStats != 0)
    *fERunStats += *fESliceStats;
  if (fPSliceStats != 0)
    *fPRunStats += *fPSliceStats;

  if (fERunStats != 0 || fPRunStats != 0)
    clog << "\nCumulative stats: "
	 << endl;

  if (fERunStats != 0)
    PrintStats (*fERunStats, fEStatsNames, fEStatsUnits);
  if (fPRunStats != 0)
    PrintStats (*fPRunStats, fPStatsNames, fPStatsUnits);
}


Double_t 
TaRun::GetDBValue(string key) const 
{ 
  return fDataBase->GetData(key); 
}


// Private member functions

void 
TaRun::Uncreate()
{
  // Utility function for destructor.

  fCoda->codaClose();
  delete fCutList;
  delete fCoda;
  delete fEvent;
  fCutList = 0;
  fCoda = 0;
  fEvent = 0;
}

Int_t 
TaRun::GetBuffer()
{
  // Pull in a CODA buffer
  int status = fCoda->codaRead();
  if (status != 0) 
    {
      return status;
    } 
  fEvent->Load( fCoda->getEvBuffer() );
  return 0;
}

void TaRun::InitDevices() {
// Initialize the devices and add their data to the output tree.
  if ( !fDevices ) {
    cout << "TaRun::InitDevices:: ERROR:  You must create and initialize";
    cout << " fDevices before adding to tree"<<endl;
    return;
  }
  if ( !evtree ) {
    cout << "TaRun::InitDevices:: ERROR:  You must create the root tree";
    cout << " before attempting to add to it."<<endl;
    return;
  }
  fEvent->AddToTree(*fDevices, *evtree);
};

Int_t TaRun::GetKey(string keystr) const {
// return the integer key that corresponds to a string.  
  return fDevices->GetKey(keystr);
};

string TaRun::GetKey(Int_t key) const {
// return the string key that corresponds to the integer key.
  return fDevices->GetKey(key);
};


void 
TaRun::PrintStats (TaStatistics s, vector<string> n, vector<string> u) const
{
  for (size_t j = 0; j < s.Size(); ++j)
    {
      // Some quantities it doesn't make sense to do statistics on.
      // For now we accumulate but don't do averages etc. on anything
      // whose name starts with "Left" or "Right".  There's probably a
      // better way but this will do at least for now.

      size_t i = n[j].find(' ');
      string firstword = string (n[j], 0, i);
      if (firstword != "Right" && firstword != "Left")
	{
	  clog << n[j];
	  if (s.Neff(j) > 5)
	    {
	      clog << " mean " << s.Mean(j)
		   << " +- " << s.MeanErr(j)
		   << " RMS " << s.DataRMS(j) 
		   << " " << u[j] << endl;
	    }
	  else
	    {
	      clog << " insufficient data" << endl;
	    }
	}
    }
}
