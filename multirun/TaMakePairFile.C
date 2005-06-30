#define TaMakePairFile_cxx
//////////////////////////////////////////////////////////////////////
//
// TaMakePairFile.C
//
//  Class that compiles together many runs into a single rootfile
//
//    Usage: 
//       TaMakePairFile *mpf = new TaMakePairFile("summary.root",
//                                                "chooser.txt");
//       mpf->SetRunList("runlist.txt");
//       mpf->RunLoop();
//       mpf->Finish();
//
//////////////////////////////////////////////////////////////////////

#include <multirun/TaMakePairFile.h>
#include <fstream>

TaMakePairFile::TaMakePairFile(TString rootfilename, 
			       TString chooserfilename):
  fRootFile(0),
  fTree(0),
  pairSelect(0),
  regSelect(0),
  fChooser(0),
  fDitFilename(),
  fRunFilename(),
  doubleVars(0),
  intVars(0),
  doubleRegVars(0),
  intRegVars(0),
  ditBPMindex(0),
  DETindex(0)
{
  // Constructor.  Create the output rootfile and tree.  
  //  Use the info in the TaVarChooser to make tree branches .
  fRootFile = new TFile(rootfilename,"RECREATE");
  fTree     = new TTree("S","Pair Summary File");
  fChooser  = new TaVarChooser(chooserfilename);

  MakeVarList();
  MakeBranches();

}

void TaMakePairFile::RunLoop() 
{
  // Loops through each run in the Runlist.

  if(!isConfigured()) return;

  // Loop over each run
  for(UInt_t irun=0; irun<runlist.size(); irun++) {
    runnumber = runlist[irun].first;
    slug      = runlist[irun].second;

    // Need to write up this routine... uses TaRootRep
    //     slowsign  = GetRunSign();

    TaFileName panFN(runnumber,"standard","root");
    TFile panFile(panFN.Tstring());
    TTree *asy = (TTree*)panFile.Get("P");
    if(!panFile.IsOpen()) {
      delete asy;
      cerr << "*** Warning: unable to open PAN rootfile for run " 
	   << runnumber << endl;
      continue;
    }

    TaFileName regFN(runnumber,"regress","root");
    TFile regFile(regFN.Tstring());
    TTree *reg = (TTree*)regFile.Get("reg");
    if(!regFile.IsOpen()) {
      delete reg;
      cerr << "*** Warning: unable to open Regression rootfile for run " 
	   << runnumber << endl;
      continue;
    }
    // Should find a crafty way to check the trees too..
    //  ... like valid keys...

    Long64_t nevents = asy->GetEntries();
    
    pairSelect = new TaPairSelector(asy,doubleVars,intVars,*fChooser); 
    regSelect = new TaPairSelector(reg,doubleRegVars,intRegVars,*fChooser);

    EventLoop(nevents);

    cout << "close panFile" << endl;
    panFile.Close();
    cout << "close regFile" << endl;
    regFile.Close();

    cout << "delete pairSelect" << endl;
    delete pairSelect;
    cout << "delete regSelect" << endl;
    delete regSelect;

//     cout << "delete asy" << endl;
//     if(asy) delete asy;
//     cout << "delete reg" << endl;
//     if(reg) delete reg;

  }
}

void TaMakePairFile::EventLoop(Long64_t nevents)
{
  // For the current run... loop through each event (up to nevents).
  //  grabbing the selected data from the pan/regression rootfile.

  // need to work this out...
//   Int_t minL = getHRunEventEdge(0,"L",runnumber);
//   Int_t maxL = getHRunEventEdge(1,"L",runnumber);
//   Int_t minR = getHRunEventEdge(0,"R",runnumber);
//   Int_t maxR = getHRunEventEdge(1,"R",runnumber);

  for(UInt_t ient=0; ient<nevents; ient++){
    
    // Load up the events
    pairSelect->ProcessLoad(ient);
    // Check for an ok_cut, or bpm saturation cut
    if(!pairSelect->ProcessCut()) continue; // Nope... move along..
    ok_Left = 1;
    ok_Right = 1;
    
    // Check for specific event cuts...
//     Int_t m_ev_num = pairSelect->doubleData[0]; // always the first doubleVar.
    //	cout << "Mean event number = " << m_ev_num << endl;
    // need the getRunEventEdge thing...
//     if (m_ev_num<minL || m_ev_num>maxL) ok_Left = 0;
//     if (m_ev_num<minR || m_ev_num>maxR) ok_Right = 0;
    
    if (ok_Left==1 && ok_Right==1) ok_Both=1;
    if (!ok_Left && !ok_Right) continue;

    // Load up the regression events.
    regSelect->ProcessLoad(ient);

    // Copy data into the local leafs
    for(UInt_t i=0; i<intVars.size(); i++) {
      intData[i] = pairSelect->intData[i];
      cout << intData[i] << endl;
    }
    for(UInt_t i=0; i<doubleVars.size(); i++) {
      doubleData[i] = pairSelect->doubleData[i];
    }

    for(UInt_t i=0; i<intRegVars.size(); i++) {
      intRegData[i] = regSelect->intData[i];
    }
    for(UInt_t i=0; i<doubleRegVars.size(); i++) {
      doubleRegData[i] = regSelect->doubleData[i];
    }
    
    // Need to do dither corrections here...

    fTree->Fill();
  }

}

void TaMakePairFile::Finish() 
{
  // All done.  Write and close the output.

  cout << "Finished" << endl;

  if(!isConfigured()) return;


  fRootFile->cd();
  fTree->Write();
  fRootFile->Close();

//   if(fChooser) delete fChooser;
//   if(fTree) delete fTree;
//   if(fRootFile) delete fRootFile;
}

void TaMakePairFile::MakeVarList() 
{
  // Using information from the TaVarChooser, create a list of variables
  //  to be read out from the rootfiles (as well as defining the branches
  //  of the output tree).

  doubleVars.push_back("m_ev_num"); // always first doubleVar... dont change it

  PushToDoubleList(fChooser->GetBatteries(),"diff_batt");

  PushToDoubleList(fChooser->GetDitBPMs(),"avg_bpm","dit");
  PushToDoubleList(fChooser->GetDitBPMs(),"diff_bpm","dit");

  PushToDoubleList(fChooser->GetBPMs(),"avg_bpm");
  PushToDoubleList(fChooser->GetBPMs(),"diff_bpm");

  PushToDoubleList(fChooser->GetBCMs(),"avg_bcm");
  PushToDoubleList(fChooser->GetBCMs(),"asym_bcm");
 
  vector <TString> flumi;
  if(fChooser->GetFLumi()) 
    for(UInt_t i=0; i<2; i++) 
      flumi.push_back(Form("%d",i+1));
  PushToDoubleList(flumi,"avg_n_flumi");
  PushToDoubleList(flumi,"asym_n_flumi");
  PushToDoubleList(flumi,"asym_n_flumi","reg");

  vector <TString> blumi(0);
  if(fChooser->GetBLumi()) 
    for(UInt_t i=0; i<8; i++) 
      blumi.push_back(Form("%d",i+1));
  PushToDoubleList(blumi,"avg_n_blumi");
  PushToDoubleList(blumi,"asym_n_blumi");
  PushToDoubleList(blumi,"asym_n_blumi","reg");

  vector <TString> dets(0);
  if(fChooser->GetHe4Detectors()) {
    dets.push_back("1"); dets.push_back("3");
  } else if (fChooser->GetLH2Detectors()) {
    dets.push_back("1");   dets.push_back("2");
    dets.push_back("3");   dets.push_back("4");
    dets.push_back("_l");  dets.push_back("_r");
    dets.push_back("_lo"); dets.push_back("_hi");
    dets.push_back("_all");
  }
  PushToDoubleList(dets,"avg_n_det");
  PushToDoubleList(dets,"asym_n_det");
  PushToDoubleList(dets,"asym_n_det","reg");
  
}

void TaMakePairFile::PushToDoubleList(vector <TString> thisvar,
				      TString prefix,
				      TString type) 
{
  // Given a vector of variables... push them onto the variable list,
  //  adding the prefix and a suffix where needed.

  // Regressing pushes...
  if(type=="reg") {
    if(thisvar.size()) {
      for(UInt_t i=0; i<thisvar.size();i++)
	doubleRegVars.push_back("reg_"+prefix+thisvar[i]);
    }
  } else {
    // Regular Pan pushes...
    if(thisvar.size()) {
      for(UInt_t i=0; i<thisvar.size();i++)
	if(thisvar[i].Contains("bpm") && type!="dit") {
	  // Push for BPMs... dithering BPMs already have x and y
	  doubleVars.push_back(prefix+thisvar[i]+"x");
	  doubleVars.push_back(prefix+thisvar[i]+"y");
	} else {
	  doubleVars.push_back(prefix+thisvar[i]);
	  // Keep track of dithering indices here..
	  if(type=="dit" && prefix.Contains("diff_bpm")) {
	    ditBPMindex.push_back(doubleVars.size());
	  }
	  if(prefix.Contains("asym_n_det") ||
	     prefix.Contains("asym_n_flumi") ||
	     prefix.Contains("asym_n_blumi")) {
	    DETindex.push_back(doubleVars.size());
	  }
	}
    }
  }
}

void TaMakePairFile::MakeBranches() 
{
  // Make the branches for the output tree, according to those variables
  //  choosen in the TaVarChooser.

  // Some default branches that are somewhat specific to each run.
  fTree->Branch("run",&runnumber,"run/I");
  fTree->Branch("sign",&slowsign,"sign/I");
  fTree->Branch("slug",&slug,"slug/I");
  fTree->Branch("ok_cutB",&ok_Both,"ok_cutB/I");
  fTree->Branch("ok_cutL",&ok_Left,"ok_cutL/I");
  fTree->Branch("ok_cutR",&ok_Right,"ok_cutR/I");  
  fTree->Branch("m_ev_num_off",&m_ev_num_off,"m_ev_num_off/I");

  cout << "MakePairFile" << endl;
  for(UInt_t i=0; i<doubleVars.size(); i++) {
    fTree->Branch(doubleVars[i],&doubleData[i],doubleVars[i]+"/D");
    cout << "\t" << i << "\t" << doubleVars[i] << endl;
  }
  for(UInt_t i=0; i<intVars.size(); i++) {
    fTree->Branch(intVars[i],&intData[i],intVars[i]+"/I");
  }
  for(UInt_t i=0; i<doubleRegVars.size(); i++) {
    fTree->Branch(doubleRegVars[i],&doubleRegData[i],doubleRegVars[i]+"/D");
  }
  for(UInt_t i=0; i<intRegVars.size(); i++) {
    fTree->Branch(intRegVars[i],&intRegData[i],intRegVars[i]+"/I");
  }

}

Bool_t TaMakePairFile::isConfigured() 
{
  // Check to make sure that we are all configured, before looping
  //  through the runs.

//   if(fDitFilename.IsNull() || runlist.empty()) {
  if(runlist.empty()) {
    cout << "Need to configure TaMakePairFile with:" << endl;
//     if(fDitFilename.IsNull()) 
//       cout << "\t o Dithering Slope source.." << endl
// 	   << "\t\t TaMakePairFile::SetDitSlopeFile(filename)" << endl;
    if(runlist.empty())
      cout << "\t o Run/Slug list source.." << endl
	   << "\t\t TaMakePairFile::SetRunList(filename)" << endl;
    return kFALSE;
  }
  return kTRUE;
}

Bool_t TaMakePairFile::SetRunList(TString listfilename) 
{
  // Given a filename, create the runlist.
  //  File name is in this format
  //    run  slug
  // e.g.
  //    1028  0
  //    1029  1

  runlist.clear();
  
  ifstream slugfile(listfilename.Data());
  UInt_t found=0;
  while(1) {
    UInt_t run=0, slug=0;
    slugfile >> run >> slug;
    if(!slugfile.good()) break;
    runlist.push_back(make_pair(run,slug));
    found++;
  }
  if(found>0) {
    cout << "\n Found " << found << " runs!" << endl << endl;
    return kTRUE;
  } else {
    cout << "\nNo runs found... check filename: " << listfilename 
	 << endl << endl;
    return kFALSE;
  }

}

Bool_t TaMakePairFile::SetRunList(vector <pair <UInt_t,UInt_t> > list)
{
  // Set the runlist by providing the same structure that 
  //  TaMakePairFile uses.

  if(list.empty()) {
    cout << "WARNING: Input runlist is empty." << endl;
    return kFALSE;
  }

  runlist = list;
  return kTRUE;

}
