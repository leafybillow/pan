//////////////////////////////////////////////////////////////////////
//
// Integrated.C
//   Bryan Moffit - June 2005
//
//  Provides a means to determine the convergence (or lack of) of
//   a given parameter (branch of a tree) vs time.
//
//  *** This macro assumes that there is a perfect correlation between
//      the branch entry number and m_ev_num/2. 
//      (Always the case??)
//
//  Requires:
//     PAN Libraries, m_ev_num in the input Tree.
//  Output:
//     TGraphErrors object
//  Usage:
//     TGraphErrors *gr = 
//        Integrated("TreeName","Parameter",TCut usercut,UInt_t nbins);
//

TGraphErrors *Integrated(TTree *tree, 
			 TString parameter,
			 TCut usercut=1,
			 UInt_t nbins=20) {

  // Check to make sure input arguments are okay.
  if(!tree) {
    cout << "Integrated: " << endl
	 << "\t Input tree does not exist." << endl;
    return 0;
  }
  if(!tree->FindBranch(parameter)) {
    cout << "Integrated: " << endl
	 << "\t Input parameter (" << parameter 
	 << ") does not exist in tree." << endl;
    return 0;
  }
  
  // Check to make sure all other needed branches exist
  if( (!tree->FindBranch("ok_cut")) ||
      (!tree->FindBranch("m_ev_num")) ) {
    cout << "Integrated: " << endl
	 << "\t Tree does not contain crucial branches." << endl;
    return 0;
  }

  // Make sure nbins!=0
  if(nbins==0) return 0;

  // Grab the parameter entries from the tree
  
  if(tree->Draw(parameter,usercut+"ok_cut","goff")==-1) {
    cout << "Integrated: " << endl
	 << "\t Could not process.  Check variables within provided cut:" 
	 << endl
	 << "\t\t" << usercut.GetTitle() << endl;
    return 0;
  }

  // Determine the number of entries per bin (ent_per_bin)
  Double_t ent_per_bin = tree->GetSelectedRows()/nbins;

  TGraphErrors *graph = new TGraphErrors(nbins);

  // Go through each bin (accumulating statistics), 
  // and add it to the TGraphErrors
  TaStatistics  *stat = new TaStatistics(1,kFALSE);
  stat->Zero();
  UInt_t current_bin = 0;
  UInt_t entries_in_bin = 0;
  for(UInt_t ient = 0; ient<tree->GetSelectedRows(); ient++) {
    if(entries_in_bin==ent_per_bin) {
      // Set current bin in TGraph, and move on to next bin
      graph->SetPoint(current_bin,(current_bin+1)*(1./15.)*ent_per_bin/60.,stat->Mean(0));
      graph->SetPointError(current_bin,0,stat->MeanErr(0));
      current_bin++; entries_in_bin=0;
    }
    Double_t value = tree->GetV1()[ient];
    stat->Update(value);
    entries_in_bin++;
  }
    
  graph->SetTitle(parameter+" Integrated Convergence;time (minutes);"+parameter);

  return graph;

}
