// Interactive root script example
// If one runs ./pan, the in the root shell .x run.cc
{

  UInt_t run;
  cout << "Enter run number: "<< endl;
  cin >> run;
  cout << "Will process run = " << run << endl;
  
  TaAnalysisManager am;
  
  if (am.Init (run) != 0)
    return 1;
  if (am.InitLastPass() != 0 ||
      am.Process() != 0 ||
      am.End() != 0)
    return 1;
  
  gROOT->LoadMacro("macro/open.macro");
  
  open(run,"standard");
  TTree *raw = (TTree*)gROOT.FindObject("R");
  //   raw->Print();
  TTree *asy = (TTree*)gROOT.FindObject("P");
  //  asy->Print();
  
  raw->Draw("bcm1:ev_num");
  

}
