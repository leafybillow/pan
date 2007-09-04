
void showCuts()
{
  TPaveText *pt = new TPaveText(0.05,0.05,0.95,0.95,"brNDC");
//   pt->AddText("Trying this");
  TTree *rawtree=NULL, *pairtree=NULL;
  rawtree= (TTree*)gROOT->FindObject("R");
  pairtree= (TTree*)gROOT->FindObject("P");
  if(rawtree==NULL || pairtree==NULL) {
    pt->AddText("NO STATISTICS");
    pt->Draw();
    return;
  }
  rawtree->Refresh();
  pairtree->Refresh();
  Long64_t total_ev = rawtree->GetEntries();
  Long64_t total = pairtree->GetEntries();
  if(total==0) {
    pt->AddText("NO pairs");
    pt->Draw();
    return;
  }
  Long64_t n_ok_cut = pairtree->Draw("ok_cut","ok_cut","goff");
  Long64_t n_detsat_cut = pairtree->Draw("ok_cut",
				  "cut_det_saturate[1]||cut_det_saturate[0]",
				  "goff");
  // Monitors will saturate when beam is ramping.. 
  //  remove the beam ramping from that tally
  TString moncut = "(cut_mon_saturate[1]||cut_mon_saturate[0])";
  moncut += "&&(cut_low_beam[1]==0||cut_low_beam[1]==0)";
  moncut += "&&(cut_beam_burp[1]==0||cut_beam_burp[0]==0)";
  Long64_t n_monsat_cut = 
    (Double_t)pairtree->Draw("ok_cut",
			     moncut,
			     "goff");
  Long64_t n_helicity_cut = (Double_t)pairtree->Draw("ok_cut",
				  "cut_pair_seq[1]||cut_pair_seq[0]",
				  "goff");
  pt->AddText(Form("Total Events (no cuts) = %d",total_ev));
  pt->AddText(Form("Total Pairs (no cuts) = %d",total));
  pt->AddText(Form("***** OK pairs = %d *****",n_ok_cut));
  pt->AddText("-------- Pair Cut Info --------");
  pt->AddText(Form("Detector saturated = %.0f (%.1f\%)",(double)n_detsat_cut,
		   100.0*(double)n_detsat_cut/(double)total));
  pt->AddText(Form("BPM saturated = %.0f (%.1f\%)",(double)n_monsat_cut,
		   100.0*(double)n_monsat_cut/(double)total));
  pt->AddText(Form("Helicity Errors = %.0f (%.1f\%)",(double)n_helicity_cut,
		   100.0*(double)n_helicity_cut/(double)total));
  pt->AddText("");
  pt->Draw();


}
