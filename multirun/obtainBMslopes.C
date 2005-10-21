///////////////////////////////////////////////////////////////////
//
// obtainBMslopes.C
//   Bryan Moffit - Nov, 2004
//
//  Get the Beam Modulation slopes or Regression coefficients from the
//  postpan analysis ROOT files, and store them to a new ROOTfile
//  <type>_helium.root
//
//
// Run this macro with the redana libraries loaded...
//  (using libRedAna.so or the redana executable)
//
//
#define MAXRUNS 1000
//#define NOISY

int obtainBMslopes(TString type="dither",
		   TString runlistStr="runlist.txt",
		   TString exp="helium")
{
  if(exp=="helium") 
    {
      const UInt_t nDet=11;
      TString sDet[nDet] = {"det1","det3",
			    "det_lo",
			    "blumi1","blumi2","blumi3","blumi4",
			    "blumi5","blumi6","blumi7","blumi8"};
      return getslopes(type,runlistStr,exp,nDet,sDet);
    }
  if(exp=="hydrogen")
    {
      const UInt_t nDet=17;
      TString sDet[nDet] = {"det1","det2","det3","det4",
			    "det_l","det_r","det_lo","det_hi","det_all",
			    "blumi1","blumi2","blumi3","blumi4",
			    "blumi5","blumi6","blumi7","blumi8"};
      return getslopes(type,runlistStr,exp,nDet,sDet);
    }
  
  return 1;
}

int getslopes(TString type,
	      TString runlistStr,
	      TString exp,
	      const UInt_t nDet,
	      TString *sDet) 
{

  typedef UInt_t Long64_t;
  gROOT->LoadMacro("multirun/TaRunlist.C+");

  const UInt_t nMon = 5;
  TString sMon[nMon] = {"bpm4bx", "bpm4by", 
		     "bpm4ax", "bpm4ay",
		     "bpm12x"};

  // Array containing run numbers and slug identifiers
  UInt_t runlist[MAXRUNS];
  UInt_t sluglist[MAXRUNS];

  TaRunlist dblist(runlistStr.Data());
  if(!dblist.GoodRunlist()) {
    cout << "Bad runlist" << endl;
    return;
  }
  
  // Fill the runlist with the runs containing dithering
  Int_t nruns = dblist.GetNumberOfRuns();
  
  for (Int_t ir = 0; ir<nruns; ir++) {
    runlist[ir] = dblist.GetRunByIndex(ir);
    sluglist[ir] = dblist.GetSlug(runlist[ir]);
#ifdef NOISY
    cout << runlist[ir] << "  " << sluglist[ir] << endl;
#endif
  }
  cout << "largest run number = " << runlist[nruns-1] << endl;

  // Create the output rootfile and tree.
  TFile slope_summary_file(type+"_"+exp+".root","RECREATE");
  TTree slopes("slopes",type+" slopes for "+exp);
  slopes.SetMarkerStyle(22);

  // Create the tree Branches
  Int_t runnumber;
  Int_t supercycle,minirun;
  Int_t slugnum;
  Int_t firstev, lastev;
  Double_t runmod;  // runmod = run + supercycle/(max supercycles in run)
  Int_t ok_slopes=1;
  Int_t ok_slopesL=1;
  Int_t ok_slopesR=1;
  Double_t sc_slopes[nDet][nMon]; // Slopes from current supercycle

  cout << "Making branches" << endl;
  slopes.Branch("run",&runnumber,"run/I");
  if(type=="dither") {
    slopes.Branch("supercycle",&supercycle,"supercycle/I");
    slopes.Branch("firstev",&firstev,"firstev/I");
    slopes.Branch("lastev",&lastev,"lastev/I");
  } else if(type=="regress") {
    slopes.Branch("minirun",&minirun,"minirun/I");
  }
  slopes.Branch("runmod",&runmod,"runmod/D");
  slopes.Branch("slug",&slugnum,"slug/I");
  slopes.Branch("ok_slopes",&ok_slopes,"ok_slopes/I");
  slopes.Branch("ok_slopesL",&ok_slopesL,"ok_slopesL/I");
  slopes.Branch("ok_slopesR",&ok_slopesR,"ok_slopesR/I");
  for(UInt_t iDet=0; iDet<nDet; iDet++) {
    for(UInt_t iMon=0; iMon<nMon; iMon++) {
      TString bName = sDet[iDet] + "_" + sMon[iMon];
      TString bNameType = bName + "/D";
      slopes.Branch(bName,&sc_slopes[iDet][iMon],bNameType);
    }
  }

  UInt_t Badruns[MAXRUNS]; UInt_t nBadruns=0;
  // Loop over each run... obtain the slopes

  TCanvas *c1; // This is for the root bug
  for(UInt_t irun=0; irun<nruns; irun++) {

    TaFileName::Setup(runlist[irun],type.Data());
    TString file = (TaFileName ("root")).Tstring();
    TFile anaFile(file,"READ");
    if(anaFile.IsZombie() || !anaFile.IsOpen()) {
      // Something wrong with this file... skip it.
      Badruns[nBadruns]=runlist[irun]; nBadruns++;
      anaFile.Close();
      cout << "   ... Skipping run " << runlist[irun] << endl;
    } else {

      Bool_t skip=kFALSE;
      // Update the Monitor and Detector indices
      TaRootDitDef *ditdef; TaRootRegDef *regdef;
      if(type=="dither") {
	ditdef = (TaRootDitDef*)anaFile.Get("TaRootDitDef;1");
	if(ditdef==0) {
	  cout << "run " << runlist[irun] << " has no TaRootDitDef.. "
	       << " SKIPPED." << endl;
	  Badruns[nBadruns]=runlist[irun]; nBadruns++;
	  skip=kTRUE;
	  anaFile.Close();
	}
      }
      if(type=="regress") {
	regdef = (TaRootRegDef*)anaFile.Get("TaRootRegDef;1");
	if(regdef==0) {
	  cout << "run " << runlist[irun] << " has no TaRootRegDef.. "
	       << " SKIPPED." << endl;
	  Badruns[nBadruns]=runlist[irun]; nBadruns++;
	  skip=kTRUE;
	  anaFile.Close();
	}
      }
      Int_t badIndex=-1, badType=0;
      Int_t index=-1;
      // Indices to be obtained from TaRootDitDef
      int monIndex[nMon];
      int detIndex[nDet];

      if(!skip) {
	for(UInt_t imon=0; imon<nMon; imon++) {
	  if(type=="dither") index = ditdef->GetMonitorIndex(sMon[imon]);
	  if(type=="regress") index = regdef->GetIVIndex("diff_"+sMon[imon]);
	  if(index==-1) {
	    badIndex=imon;
	    badType=1;
	  }
	  monIndex[imon] = index;
	}
	index=-1;
	for(UInt_t idet=0; idet<nDet; idet++) {
	  if(type=="dither") index = ditdef->GetDetectorIndex(sDet[idet]);
	  if(type=="regress") index = regdef->GetDVIndex("asym_n_"+sDet[idet]);
	  if(index==-1) {
	    badIndex=idet;
	    badType=2;
	  }
	  detIndex[idet] = index;
	}
      }

      // Skip this run if there's trouble with the indices
      if(badIndex!=-1 || skip) {
	if(!skip) {
	  cout << "run " << runlist[irun] 
	       << " Bad index for ";
	  if(badType==1) cout << sMon[badIndex];
	  if(badType==2) cout << sDet[badIndex];
	  cout << " - SKIPPED" << endl;
	  Badruns[nBadruns]=runlist[irun]; nBadruns++;
	}
	anaFile.Close();
      } else {
	
	// Get the slopes/coeffs tree..
	TTree *slpsTree;
	if(type=="dither") 
	  slpsTree = (TTree*)anaFile.Get("ditslps");
	else if (type=="regress")
	  slpsTree = (TTree*)anaFile.Get("regcoeffs");
	UInt_t nSC = (UInt_t)slpsTree->GetEntries(); // # of supercycles in this run
	if(nSC==0) {
	  // No supercycles... head to the next run
	  cout << "run " << runlist[irun] 
	       << " No slopes - SKIPPED" << endl;
	  // Probably not a bad run, if looking for dithering
	  //  (maybe no dithering cycles were performed)
	  if(type=="regress") {Badruns[nBadruns]=runlist[irun]; nBadruns++;}
	  anaFile.Close();
	}else{	
	  
	  Double_t allslopes[nDet][nMon][100]; 
	  UInt_t slope_sc[100];
	  Double_t max_sc;
	  UInt_t sc_firstev[100], sc_lastev[100];
	  // initialize the arrays to some default values
	  for(UInt_t idet=0; idet<nDet; idet++) {
	    for(UInt_t imon=0; imon<nMon; imon++) {
	      for(UInt_t isc=0;isc<100;isc++) {
		allslopes[idet][imon][isc] = -100000;
		slope_sc[isc] = 0;
		sc_firstev[isc] = 0; sc_lastev[isc]=0;
	      }
	    }
	  }
	  
	  // Obtain all of the slopes, for all of the supercycles..
	  for(UInt_t idet=0; idet<nDet; idet++) {
	    for(UInt_t imon=0; imon<nMon; imon++) {
	      TString drawcom;
	      if(type=="dither") drawcom = "slpDetVsMon[";
	      if(type=="regress") drawcom = "coeff[";
	      drawcom += detIndex[idet]; drawcom += "][";
	      drawcom += monIndex[imon]; 
	      if(type=="dither") {
		drawcom += "]:supercyc";
		drawcom += ":firstev:lastev";
	      }
	      if(type=="regress") drawcom += "]:minirun";
	      
	      if(c1==NULL) c1 = new TCanvas("c1","ROOT BUG");
	      slpsTree->Draw(drawcom,"","goff");
	      Long64_t nrows = slpsTree->GetSelectedRows();
	      if(nSC!=nrows) {
		// I dont anticipate this as a problem... 
		// ... but just in case... here's a BIG warning.
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
		     << endl;
		cout << "!!! Number of Entries != Number of Supercycles !!!" 
		     << endl;
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
		     << endl;
	      }
	      for(UInt_t isc=0;isc<nrows;isc++) {
		// isc doesn't necessarily equal the current supercycle #
		slope_sc[isc] = TMath::Nint(slpsTree->GetV2()[isc]);
		allslopes[idet][imon][isc] = slpsTree->GetV1()[isc];
		if(type=="dither") {
		  sc_firstev[isc] = TMath::Nint(slpsTree->GetV3()[isc]);
		  sc_lastev[isc] = TMath::Nint(slpsTree->GetV4()[isc]);
		}
	      }
	      // we can assume that the last isc contains the largest observed SC
	      Int_t mxrow = (UInt_t) nrows;
	      max_sc = slope_sc[mxrow-1] + 1;
	    }
	  }
	  
	  // Fill the branch variables with their values for this run.
	  runnumber = runlist[irun];
	  slugnum = sluglist[irun];
	  for(UInt_t isc=0; isc<nSC; isc++) {
	    ok_slopes=1; ok_slopesL=1; ok_slopesR=1;
	    for(UInt_t idet=0; idet<nDet; idet++) {
	      // check here, to see if the slopes are within
	      // good detector event ranges.
	      Int_t minL = dblist.GetLeftDetLo(runnumber);
	      Int_t maxL = dblist.GetLeftDetHi(runnumber);
	      Int_t minR = dblist.GetRightDetLo(runnumber);
	      Int_t maxR = dblist.GetRightDetHi(runnumber);
	      // Remove the slopes if one or both detetectors are off
	      if((minL==-1)&&(maxL==-1))
		ok_slopesL=0;
	      if((minR==-1)&&(maxR==-1))
		ok_slopesR=0;
	      if(type=="dither") {
		if(minL>sc_firstev[isc] || maxL<sc_lastev[isc])
		  ok_slopesL=0;
		if(minR>sc_firstev[isc] || maxR<sc_lastev[isc])
		  ok_slopesR=0;
	      }
	      if(minL==0 && maxL==0) 
		ok_slopesL=1;
	      if(minR==0 && maxR==0)
		ok_slopesR=1;
	      
	      
	      for(UInt_t imon=0; imon<nMon; imon++) {
		// check for invalid slopes (-100000 is bad)
		if(allslopes[idet][imon][isc]==-100000) {
		  ok_slopes=0;
		}
		sc_slopes[idet][imon] = allslopes[idet][imon][isc];
	      }
	      ok_slopesL *= ok_slopes;
	      ok_slopesR *= ok_slopes;
	    }
#ifdef NOISY
	    cout << "run " << runnumber;
#endif
	    if(type=="dither") {
	      supercycle = slope_sc[isc];
	      firstev = sc_firstev[isc];
	      lastev = sc_lastev[isc];
 	      runmod = (Double_t)runlist[irun] 
 		+ Double_t(supercycle)/Double_t(max_sc);
#ifdef NOISY
	      cout << " supercycle " << supercycle << endl;
#endif
	    }
	    if(type=="regress") {
	      minirun = slope_sc[isc];
	      runmod = (Double_t)runlist[irun] 
		+ Double_t(minirun)/Double_t(nSC);
#ifdef NOISY
	      cout << " minirun " << minirun << endl;
#endif
	    }
	    
	    // Make the summary file the current file.
	    slope_summary_file.cd();
	    // Fill the summary tree with the variables from this run.
	    slopes.Fill();
	  }
	}
	// Close the current run ROOTfile.
	anaFile.Close();
      }
    }
  }

  // make sure the summary file is the current file, and write to it.
  slope_summary_file.cd();
  // The line below is commented out because it makes a second copy
  // of the slopes tree.. and makes the rootfile larger.
  // I guess I dont need it.
  //  slopes.Write();
  slope_summary_file.Write();

  if(nBadruns>0) {
    cout << nBadruns << " Bad or suspect runs: ";
    for(UInt_t irun=0; irun<nBadruns; irun++) {
      if(irun%8==0) cout << endl;
      cout  << "\t" << Badruns[irun];
    }
    cout << endl;
  }

  if(c1) delete c1;
    
  return 1;


}

