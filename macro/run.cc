// Interactive root script example
// If one runs ./pan, the in the root shell .x run.cc
{

  UInt_t runnumber;
  cout << "Enter run number: "<< endl;
  cin >> runnumber;
  cout << "Will process run = " << runnumber << endl;
  
  TaAnalysisManager am;
  
  am.Init(runnumber);
  am.Process();
  am.End();
  
  // Print the output tree
  R->Print();
  
  // Draw something (you can also do it interactively)
  //R->Draw("bpm8x");
  //R->Draw("bpm8xp+bpm8xm:bcm1");
  
  //R->Draw("bcm1:event_number");
}
