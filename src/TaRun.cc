//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRun.cc  (implementation)
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
// This class treats the data of one run. The Init method initializes
// the event TTree, attaches the Coda file or online data, and gets
// the database.  It initializes the storage of devices and cuts.
//
// In the event loop, the NextEvent method is called to get and decode
// an event from the data stream.  AddCuts is called after
// preprocessing each event, to update the list of cut intervals.
// AccumEvent and AccumPair accumulate statistics for results of event
// and pair analysis, respectively.  PrintSlice and PrintRun write
// statistics summaries to STDOUT.
//
// When analysis is complete, Finish is called to write and close the
// ROOT file.
//
////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define CHECKOUT 

#include "TaRun.hh"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include "TFile.h"
#include "THaCodaData.h"
#include "THaCodaFile.h"
#include "TaDataBase.hh"
#include "TaCutList.hh"
#include "TaEvent.hh"
#include "TaFileName.hh"
#include "TaDevice.hh"
#include "TaLabelledQuantity.hh"
#include "TaStatistics.hh"
#include "TaString.hh"
#include "VaAnalysis.hh"
#include "VaPair.hh"
#ifdef ONLINE
#include "THaEtClient.h"
#endif

#ifndef NODICT
ClassImp(TaRun)
#endif

// Flags
const ErrCode_t TaRun::fgTARUN_ERROR = -1;  // returned on error
const ErrCode_t TaRun::fgTARUN_OK = 0;      // returned on success
const ErrCode_t TaRun::fgTARUN_VERBOSE = 1; // verbose(1) or not(0) warnings

EventNumber_t TaRun::fNLastSlice = 0; // event number at last slice reset

// Constructors/destructors/operators

TaRun::TaRun():
  fDataBase(0),
  fCutList(0),
  fCoda(0),
  fCodaFileName ("online"),
  fEvent(0),
  fAccumEvent(0),
  fDevices(0),
  fRootFile(0),
  fEvtree(0),
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fFirstPass(true)
{
#ifdef ONLINE
   mymode        = 1;
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
  fDataBase(0),
  fCutList(0),
  fCoda(0),
  fEvent(0),
  fAccumEvent(0),
  fDevices(0),
  fEvtree(0),
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fFirstPass(true)
{
  TaFileName::Setup (fRunNumber, "");
  fCodaFileName = TaFileName ("coda").String();
} 

TaRun::TaRun(const string& filename):
  fRunNumber(0),
  fDataBase(0),
  fCutList(0),
  fCoda(0),
  fCodaFileName(filename),
  fEvent(0),
  fAccumEvent(0),
  fDevices(0),
  fEvtree(0),
  fESliceStats(0),
  fPSliceStats(0),
  fERunStats(0),
  fPRunStats(0),
  fFirstPass(true)
{
};

ErrCode_t
TaRun::Init(const vector<string>& dbcommand)
{
  // Run initialization: Create event tree, attach data source and
  // database, initialize static variables from these.

  if (fCodaFileName == "")
    {
      cerr << "TaRun::Init ERROR Empty filename" << endl;
      return fgTARUN_ERROR;
    }
      
  if (fCodaFileName == "online") { 
#ifdef ONLINE
    fCoda = new THaEtClient();
    if ( fCoda->codaOpen(TString(fComputer.c_str()), TString(fSession.c_str()), mymode) != 0) {
      cerr << "TaRun:: Init ERROR: Cannot open ET connection"<<endl;
      cerr << " to  computer "<<fComputer;
      cerr << " and session "<<fSession<<endl;
      return fgTARUN_ERROR;
    }
#else
    cerr << "TaRun:: Init ERROR: Undefined online input."<<endl;
    return fgTARUN_ERROR;
#endif
  } else { 
    TString tfile(fCodaFileName.c_str()); // hopefully temp. prefer <string>
    fCoda = new THaCodaFile(tfile);
    if (fCoda->status() != 0)
      {
	cerr << "TaRun::Init ERROR: Cannot open data file " 
	     << fCodaFileName << endl;
	return fgTARUN_ERROR;
      }
  }
   
  clog << "TaRun::Init Initialization for run, analyzing " 
       << fCodaFileName << endl;
  
  // Get first event
  // For a data file this will normally be a PRESTART event.

  fEvent = new TaEvent();
  if (GetBuffer() != 0)
    {
      cerr << "TaRun::Init ERROR: No data from " << fCodaFileName << endl;
      return fgTARUN_ERROR;
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
  TaFileName::Setup (fRunNumber, "");

  fDataBase = new TaDataBase();
  fDataBase->Read(fRunNumber,dbcommand);
  fDevices = new TaDevice();
  fDevices->Init(*fDataBase);
  fCutList = new TaCutList(fRunNumber);
  fCutList->Init(*fDataBase);

  fOversamp = fDataBase->GetOverSamp();
  if (fOversamp == 0)
    {
      cerr << "TaRun::Init ERROR: Oversample factor is zero, cannot analyze"
	   << endl;
      return fgTARUN_ERROR;
    }

  if (TaEvent::RunInit(*this) != 0)
    return fgTARUN_ERROR;

  // Delete and recreate fEvent, now that we have done RunInit,
  // so we are able to set up cut array therein;
  // also create fAccumEvent now.

  delete fEvent;
  fEvent = new TaEvent();
  fAccumEvent = new TaEvent();

  return fgTARUN_OK;

}
 

ErrCode_t
TaRun::ReInit()
{
  // Run reinitialization for second pass: reattach data source.

  fFirstPass = false;
  if (fCodaFileName == "")
    {
      cerr << "TaRun::ReInit ERROR Empty filename" << endl;
      return fgTARUN_ERROR;
    }
      
  if (fCodaFileName == "online") 
    {
      cerr << "TaRun::ReInit ERROR: Cannot reinitialize online data" << endl;
    } 
  else 
    { 
      TString tfile(fCodaFileName.c_str()); // hopefully temp. prefer <string>
      delete fCoda;
      fCoda = new THaCodaFile(tfile);
      if (fCoda->status() != 0)
	{
	  cerr << "TaRun::ReInit ERROR: Cannot open file" << endl;
	  return fgTARUN_ERROR;
	}
    }

  delete fCutList;
  fCutList = new TaCutList(fRunNumber);
  fCutList->Init(*fDataBase);
//    fCutList->AddName(LowBeamCut, "Low_beam");
//    fCutList->AddName(BeamBurpCut, "Beam_burp");
//    fCutList->AddName(OversampleCut, "Oversample");
//    fCutList->AddName(SequenceCut, "Sequence");
  if (TaEvent::RunInit(*this) != 0)
    return fgTARUN_ERROR;

  if (fERunStats != 0)
    fERunStats->SetFirstPass (false);
  if (fPRunStats != 0)
    fPRunStats->SetFirstPass (false);

  return fgTARUN_OK;

}
 

Int_t 
TaRun::FindRunNumber() {
  // Get run number, from run number file if online or from data
  // stream if not.

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

  static EHelicity h = UnkHeli;
  static EHelicity ph = UnkHeli;
  if (fEvent->GetEvNumber() > 0 && fEvent->GetTimeSlot() == 1)
    {    
      ph = h;
      h = fEvent->GetHelicity();
    }
  fEvent->SetPrevHelicity (ph);

  return true;
}


void 
TaRun::Decode()
{
  // Decode raw data, store event number

   fEvent->Decode(*fDevices);
   fEventNumber = fEvent->GetEvNumber();
// Use this to make detailed checks of decoding:
#ifdef CHECKOUT
   fEvent->RawDump();
   fEvent->DeviceDump();
#endif
}


void 
TaRun::AccumEvent(const TaEvent& ev, const Bool_t doSlice, const Bool_t doRun) 
{ 
  // Update event statistics with the results in this event, if it
  // passes cuts.

  fAccumEvent->CopyInPlace(ev);
  fAccumEventNumber = fAccumEvent->GetEvNumber();
  vector<TaLabelledQuantity> lqres = ev.GetResults();
  if ((doSlice || doRun) && fESliceStats == 0 && lqres.size() > 0)
    {
      // Set up statistics
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  if (!i->TestFlags(VaAnalysis::fgNO_STATS))
	    {
	      fEStatsNames.push_back (i->GetName());
	      fEStatsUnits.push_back (i->GetUnits());
	    }
	}
#ifdef NOISY
      clog << "Setting up event stats - " << fEStatsNames.size() << endl;
#endif
      if (doSlice)
	fESliceStats = new TaStatistics (fEStatsNames.size(), false);
      if (doRun)
	fERunStats = new TaStatistics (fEStatsNames.size(), false);
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
	  if (!i->TestFlags(VaAnalysis::fgNO_STATS))
	    vres.push_back (i->GetVal());
	}
      if (doSlice)
	fESliceStats->Update (vres);
      if (doRun)
	fERunStats->Update (vres);
    }
#ifdef NOISY
  else
    clog << "Event " << ev.GetEvNumber() << " is in cut interval" << endl;
#endif
  if (fFirstPass && fEvtree != 0)
    fEvtree->Fill();
}


void 
TaRun::AccumPair(const VaPair& pr, const Bool_t doSlice, const Bool_t doRun) 
{ 
  // Update pair statistics with the results in this pair, if its
  // events pass cuts.  Periodically print incremental statistics and
  // cumulative cut tally.

  vector<TaLabelledQuantity> lqres = pr.GetResults();
  if ((doSlice || doRun) && fPSliceStats == 0 && lqres.size() > 0)
    {
      // Set up statistics
      for (vector<TaLabelledQuantity>::const_iterator i = lqres.begin();
	   i != lqres.end();
	   ++i )
	{
	  if (!i->TestFlags(VaAnalysis::fgNO_STATS))
	    {
	      fPStatsNames.push_back (i->GetName());
	      fPStatsUnits.push_back (i->GetUnits());
	    }
	}
#ifdef NOISY
      clog << "Setting up pair stats - " << fPStatsNames.size() << endl;
#endif
      if (doSlice)
	fPSliceStats = new TaStatistics (fPStatsNames.size(), false);
      if (doRun)
	fPRunStats = new TaStatistics (fPStatsNames.size(), false);
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
	  if (!i->TestFlags(VaAnalysis::fgNO_STATS))
	    vres.push_back (i->GetVal());
	}
      if (doSlice)
	fPSliceStats->Update (vres);
      if (doRun)
	fPRunStats->Update (vres);
    }
#ifdef NOISY
  else
    clog << "Pair " << pr.GetRight().GetEvNumber() 
	 << "/" <<  pr.GetLeft().GetEvNumber() 
	 << " is in cut interval" << endl;
#endif

}


void
TaRun::PrintSlice (EventNumber_t n)
{
  // Print a slice and reset its statistics

  size_t nSlice = n - fNLastSlice;
  if (nSlice > 5)
    {
      if (fESliceStats != 0 || fPSliceStats != 0)
	cout << "Stats for last " << (n - fNLastSlice) << " events:";
      cout << endl;
      if (fESliceStats != 0)
	PrintStats (*fESliceStats, fEStatsNames, fEStatsUnits);
      if (fPSliceStats != 0)
	PrintStats (*fPSliceStats, fPStatsNames, fPStatsUnits);
      
      cout << endl;
      fCutList->PrintTally(cout);
      cout << endl;
    }
      
  if (fESliceStats != 0)
    fESliceStats->Zero();
  if (fPSliceStats != 0)
    fPSliceStats->Zero();
  fNLastSlice = n;
}


void
TaRun::PrintRun ()
{
  // Print run statistics

  if (fERunStats != 0 || fPRunStats != 0)
    cout << "\nCumulative stats for " << fEventNumber 
	 << " events: "
	 << endl;

  if (fERunStats != 0)
    PrintStats (*fERunStats, fEStatsNames, fEStatsUnits);
  if (fPRunStats != 0)
    PrintStats (*fPRunStats, fPStatsNames, fPStatsUnits);

  cout << endl;
  fCutList->PrintTally(cout);
  cout << endl;

// Database Print() must be done BEFORE we Put() data into database
// as result of this analysis. 

  if (fDataBase) fDataBase->Print();

}


void 
TaRun::UpdateCutList (const Cut_t cut, const Int_t val, EventNumber_t evno) 
{ 
  // Update this run's cut list with the given cut type, value, and
  // event number.

  fCutList->UpdateCutInterval ( cut, val, evno );
}

void 
TaRun::Finish() 
{ 
  // End of run.  Write out root file and close it.

  cout << "\nTaRun::Finish End of run " << fRunNumber << endl;

  if (fRootFile != 0)
    {
      fRootFile->Write();
      fRootFile->Close();
      delete fRootFile;
    }
}


Double_t 
TaRun::GetDBValue(string key) const 
{ 
  // Query database for a value.

  return fDataBase->GetData(key); 
}


// Private member functions

void 
TaRun::Uncreate()
{
  // Utility function for destructor.

  fCoda->codaClose();
  // If fEvtree is deleted, some later delete will hang.  I don't know why.
  //  delete fEvtree;
  delete fCoda;
  delete fEvent;
  delete fAccumEvent;
  delete fDataBase;
  delete fDevices;
  delete fCutList;
  delete fESliceStats;
  delete fERunStats;
  delete fPSliceStats;
  delete fPRunStats;
  fCutList = 0;
  fCoda = 0;
  fEvent = 0;
  fAccumEvent = 0;
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

Int_t 
TaRun::GetKey(string keystr) const {
  // return the integer key that corresponds to a string.  
  return fDevices->GetKey(keystr);
};

string 
TaRun::GetKey(Int_t key) const {
  // return the string key that corresponds to the integer key.
  return fDevices->GetKey(key);
};


void 
TaRun::InitRoot() 
{
  // Set up the root file and event tree

  // Open the Root file
  TaFileName::Setup (fRunNumber, TaString (fDataBase->GetAnaType()).ToLower());
  TaFileName rootFileName ("root");
  fRootFile = new TFile(rootFileName.String().c_str(),"RECREATE");
  fRootFile->SetCompressionLevel(0);
  fEvtree = new TTree("R","Event data DST");
  fEvtree->Branch ("ev_num", &fAccumEventNumber, "ev_num/I", 5000); // event number

  if ( !fDevices ) {
    cout << "TaRun::InitTree:: ERROR:  You must create and initialize";
    cout << " fDevices before adding to tree"<<endl;
    return;
  }
  if ( !fCutList ) {
    cout << "TaRun::InitTree:: ERROR:  You must create and initialize";
    cout << " fCutList before adding to tree"<<endl;
    return;
  }
  fAccumEvent->AddToTree(*fDevices, *fCutList, *fEvtree);
};


void 
TaRun::PrintStats (const TaStatistics& s, const vector<string>& n, const vector<string>& u) const
{
  // Print statistics, with given labels and units.
  
  for (size_t j = 0; j < s.Size(); ++j)
    {
      cout.setf(ios::left,ios::adjustfield);
      cout << setw(15) << n[j].c_str();
      if (s.Neff(j) > 5)
	{
	  // N.B. seem to need to convert to C-style string for setw to work.
	  cout << " mean " << setw(10) << s.Mean(j)
	       << " +- " << setw(10) << s.MeanErr(j)
	       << " RMS " << setw(8) << s.DataRMS(j) 
	       << " " << setw(6) << u[j].c_str()
	       << endl;
	  // Comment above line and uncomment this to get 
	  // print of effective number of events:
	  //   << s.Neff(j) << endl;
	}
      else
	{
	  cout << " insufficient data" << endl;
	}
    }
  //  cout << *fCutList << endl;
}
