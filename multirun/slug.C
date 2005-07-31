//////////////////////////////////////////////////////////////////////
//
// slug.C
//   Bryan Moffit - July 2005
//
//   Handle the compilation of the ok data for SLUG summaries.
//

#include <vector>
#include <iostream>
#include "multirun/TaVarChooser.h"
#include "multirun/TaMakePairFile.h"
#include "multirun/TaRunlist.h"
#include "src/TaFileName.hh"
#include "TString.h"
#include "macro/ParamSave.macro"

void runSlug(Int_t slugnumber) {

  TaRunlist dblist("runlist.txt");
  if(!dblist.GoodRunlist()) {
    cout << "Bad runlist" << endl;
  }
  
  vector <Int_t> templist = dblist.GetListOfRuns(slugnumber);

  vector <pair <UInt_t,UInt_t> > runlist;
  for(UInt_t i=0; i<templist.size(); i++) {
    runlist.push_back(make_pair(templist[i],0));
  }

  cout << "Runs entered (" << runlist.size() << ")" << endl;
  for(UInt_t irun=0; irun<runlist.size(); irun++) {
    cout << "\t" << runlist[irun].first;
    if((irun+1)%5==0) cout << endl;
  }
  cout << endl;

  TaFileName::Setup(slugnumber,"slug");
  TString filename = (TaFileName ("root")).Tstring();

  TaMakePairFile *mpf = new TaMakePairFile(filename,"multirun/chooser.txt");
  mpf->SetRunList(runlist);
  mpf->SetDBRunlist("runlist.txt");
  mpf->RunLoop();
  mpf->Finish();

  TFile slugfn(filename);
  TTree *s = (TTree*)slugfn.Get("S");

  ParamSave *ps = new ParamSave(slugnumber,"slug");
  ps->PutComment(Form("Slug = %d",slugnumber));

  ps->PutCut("ok_cutL", *s);
  ps->PutCut("ok_cutR", *s);
  ps->PutCut("ok_cutB", *s);
  ps->PutCut("(ok_cutR||ok_cutL)", *s);
  
  ps->PutAvg("bcm1", "avg", *s, "");
  ps->PutAvg("bcm2", "avg", *s, "");
  ps->PutAvg("bcm3", "avg", *s, "");
  ps->PutAvg("bcm6", "avg", *s, "");
  ps->PutAvg("bcmcav1", "avg", *s, "");
  ps->PutAvg("bcmcav2", "avg", *s, "");
  ps->PutAvg("bcmcav3", "avg", *s, "");  
  ps->PutAvg("bcm10", "avg", *s, "ok_cutC");
  ps->PutAvg("bpm4ax", "avg", *s, "");
  ps->PutAvg("bpm4ay", "avg", *s, "");
  ps->PutAvg("bpm4bx", "avg", *s, "");
  ps->PutAvg("bpm4by", "avg", *s, "");
  ps->PutAvg("bpm12x", "avg", *s, "");
  ps->PutAvg("det1", "avg_n", *s, "ok_cutL");
  ps->PutAvg("det3", "avg_n", *s, "ok_cutR");
  
  ps->PutMonAsym("bcm1", *s, "");
  ps->PutMonAsym("bcm2", *s, "");
  ps->PutMonAsym("bcm3", *s, "");
  ps->PutMonAsym("bcm6", *s, "");
  ps->PutMonAsym("bcmcav1", *s, "");
  ps->PutMonAsym("bcmcav2", *s, "");
  ps->PutMonAsym("bcmcav3", *s, "");
  ps->PutMonAsym("bcm10", *s, "ok_cutC");
  
  ps->PutMonDiff("bpm4ax", *s, "");
  ps->PutMonDiff("bpm4ay", *s, "");
  ps->PutMonDiff("bpm4bx", *s, "");
  ps->PutMonDiff("bpm4by", *s, "");
  ps->PutMonDiff("bpm12x", *s, "");
  
  ps->PutMonDDAsym("bcm1", "bcm2", *s, "");
  ps->PutMonDDAsym("bcm1", "bcm3", *s, "");
  ps->PutMonDDAsym("bcm2", "bcm3", *s, "");
  ps->PutMonDDDiff("bpm4ax", "bpm4bx", *s, "");
  ps->PutMonDDDiff("bpm4ay", "bpm4by", *s, "");
  
  ps->PutMonDD("det1", "det3", "reg_asym_n", *s, "ok_cutB");
  ps->PutDetAsymn("blumi1", *s, "");
  ps->PutDetAsymn("blumi2", *s, "");
  ps->PutDetAsymn("blumi3", *s, "");
  ps->PutDetAsymn("blumi4", *s, "");
  ps->PutDetAsymn("blumi5", *s, "");
  ps->PutDetAsymn("blumi6", *s, "");
  ps->PutDetAsymn("blumi7", *s, "");
  ps->PutDetAsymn("blumi8", *s, "");
  ps->PutDetAsymn("blumi_ave", *s, "");
  ps->PutDetAsymn("blumi_sum", *s, "");
  ps->PutDetAsymn("flumi1", *s, "");
  ps->PutDetAsymn("flumi2", *s, "");
  ps->PutDetAsymn("flumi_ave", *s, "");
  ps->PutDetAsymn("flumi_sum", *s, "");
  
  ps->PutDetAsymn("det1", *s, "ok_cutL");
  //       ps->PutDetAsymn("det2", *s, "ok_cutL");
  ps->PutDetAsymn("det3", *s, "ok_cutR");
  //       ps->PutDetAsymn("det4", *s, "ok_cutR");
  ps->PutDetAsymn("det_lo", *s, "ok_cutB");
  //       ps->PutDetAsymn("det_hi", *s, "ok_cutB");
  //       ps->PutDetAsymn("det_all", *s, "ok_cutB");
  //       ps->PutDetAsymn("det_ave", *s, "ok_cutB");
  
  ps->Print();
  delete ps;
  


}
