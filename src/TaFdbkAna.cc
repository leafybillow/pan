//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaFdbkAna.cc  (implementation)
//           ^^^^^^^^^^^^
//
//    Authors :  A. Vacheret, R. Holmes, R. Michaels
//
//    Analysis + feedbacks   
//    
//
//////////////////////////////////////////////////////////////////////////

#define FDBK1
//#define FDBK

#include "TaFdbkAna.hh"
#include "TaEvent.hh"
#include "TaRun.hh"
#include "VaPair.hh"
#include "TaCutList.hh"
#include "TaLabelledQuantity.hh"

#ifdef DICT
ClassImp(TaFdbkAna)
#endif

// Constructors/destructors/operators
TaFdbkAna::TaFdbkAna():VaAnalysis(),
 fRunNum(0),
 fQStartPair(0),fQStopPair(0),fNQpair(0),fQlevFdbk(0), fQmevFdbk(0), fQnum(0), fQsent(0),
 fZStartpair(0),fZStopPair(0), fNZpair(0),fZlevFdbk(0), fZmevFdbk(0), fZnum(0), fZsent(0),
 fZdiff4AXMean1(0), fZdiff4AXMean2(0), fZdiff4AXRMS(0), fZdiff4AXErr(0),
 fZdiff4AYMean1(0), fZdiff4AYMean2(0), fZdiff4AYRMS(0), fZdiff4AYErr(0),
 fZdiff4BXMean1(0), fZdiff4BXMean2(0), fZdiff4BXRMS(0), fZdiff4BXErr(0),
 fZdiff4BYMean1(0), fZdiff4BYMean2(0), fZdiff4BYRMS(0), fZdiff4BYErr(0),
 fFdbkTree(0)  
{
 fQasym.clear();  
 fZdiff4AX.clear();fZdiff4AY.clear();fZdiff4BX.clear();fZdiff4BY.clear();
}

TaFdbkAna::~TaFdbkAna(){  
}


void TaFdbkAna::Init(){
 VaAnalysis::Init();
 // if we decide to analyze more than one run in the future, we will
 // initialyze  feedback analysis here. 
}


void
TaFdbkAna::InitChanLists ()
{
  // Initialize the lists used by InitTree and AutoPairAna.
  // TaFdbkAna's implementation puts channels associated with beam
  // devices like TaBeamAna.

  // Initialize the lists of devices to analyze
  vector<AnaList* > f;

  // List of channels for which to store left and right values
  fCopyList = ChanList ("tir", "helicity", "");
  f = ChanList ("tir", "pairsynch", "");
  fCopyList.insert (fCopyList.end(), f.begin(), f.end());

  // List of channels for which to store differences
  fDiffList = ChanList ("bpm", "~x", "um");
  f = ChanList ("bpm", "~y", "um");
  fDiffList.insert (fDiffList.end(), f.begin(), f.end());

  // List of channels for which to store asymmetries
  fAsymList = ChanList ("bcm", "bcm1", "ppm");
  f = ChanList ("bcm", "bcm2", "ppm");
  fAsymList.insert (fAsymList.end(), f.begin(), f.end());
}

void TaFdbkAna::InitTree()
{
 VaAnalysis::InitTree();
 
  Int_t bufsize = 5000;

  // add branches in the feedback tree.
  fFdbkTree->Branch ("l_ev_fdbk", &fQlevFdbk, "l_ev_fdbk/I", bufsize); // l_ev feedback mark 
  fFdbkTree->Branch ("m_ev_fdbk", &fQmevFdbk, "m_ev_fdbk/D", bufsize); // m_ev feedback mark   
  fFdbkTree->Branch ("num_fdbk", &fQnum, "num_fdbk/I", bufsize); // Qasym feedback number   
  fFdbkTree->Branch ("asy_fdbk", &fQasymMean2, "asy_fdbk/D", bufsize); // feedback Qasym value   
  fFdbkTree->Branch ("dasy_fdbk", &fQasymErr, "dasy_fdbk/D", bufsize); // feedback Qasym error   
  fFdbkTree->Branch ("Qsent_fdbk", &fQsent, "Qsent_fdbk/I", bufsize); // Qasym EPICS flag (0 or 1)   
  
}


void TaFdbkAna::RunIni(TaRun& run) {
   // since it is called at the beginning of each run, 
  // feedbacks stuff is initialized here.

 
 fFdbkTree = new TTree("F","Feedback data DST");
 VaAnalysis::RunIni(run);
 fNQpair = 0;
 fQnum   = 0;
 fQasym.clear(); 
}



// Protected member functions


void TaFdbkAna::EventAnalysis()
{
  // put everything needed to analyze one event 
  // Just for demo purposes for now we hard code four results, the
  // values of bcm1 and bcm2 like in TaBeamAna
  fEvt->AddResult ( TaLabelledQuantity ( "bcm1 raw",
					 fEvt->GetData(IBCM1R), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm1",
					 fEvt->GetData(IBCM1), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm2 raw",
					 fEvt->GetData(IBCM2R), 
					 "chan" ) );
  fEvt->AddResult ( TaLabelledQuantity ( "bcm2",
					 fEvt->GetData(IBCM2), 
					 "chan" ) );
  // put here what is needed for GUI EVENT DISPLAY
 
}



void 
TaFdbkAna::PairAnalysis()
{ 
  // Analyze a pair and compute everything needed for a beam analysis
  // we compute bcms asymmetries, bpms differences and feedbacks values.

  AutoPairAna();
  QasymRunFeedback();
 
}

void 
TaFdbkAna::ProcessRun(){ 
  VaAnalysis::ProcessRun();
}

void TaFdbkAna::RunFini(){
 QasymEndRunFeedback();  
 VaAnalysis::RunFini();
}


void TaFdbkAna::Finish(){
  VaAnalysis::Finish(); 
}




void TaFdbkAna::QasymRunFeedback(){
       // Charge asymmetry feedback during the run. This function make 3 passes on the data 
       // 1. Calculates stats variables after X number of events (pairs)
       // 2. Filters values of charge asymmetry too far from the mean.
       // 3. Recompute mean value and RMS. 
       // then if the value is acceptable for our criteria, 
       // call TaRun::SendEPICSInfo to request feedbacks at the source            


#ifdef FDBK
  clog <<" entering TaFdbkAna::QasymRunFeedBack() "<<endl; 
#endif

   // if current pair is good ( i.e. both events passed cuts)
  // increment number of good pairs and store values of Qasym
  if ( !fPair->GetRight().CutStatus() && !fPair->GetLeft().CutStatus()){
    fNQpair++;
    fQasym.push_back(fPair->GetAsy(IBCM1)*1E6);
    fQasymMean1+=fPair->GetAsy(IBCM1)*1E6;
#ifdef FDBK
    clog <<" QasymRunFeedBack() compute QASYM : bcm1asy "<<fPair->GetAsy(IBCM1)*1E6
         <<" Npair "<<fNQpair<<" sum "<<fQasymMean1<<" current Mean "<<fQasymMean1/fNQpair<<endl; 
#endif
  }
      
  // start feedback when get enough good pairs : typically 3 minutes 
  // in the future, get it from the database 
  //  3*60*30/2 = 2700              
  if ( fNQpair >= 2700 ){
    fQnum++; // increment feedback counter
    fQsent =0; // reinitialize EPICS flag
    fQStopPair = fPair->GetLeft().GetEvNumber(); 
    fQlevFdbk  = fQStopPair; // get the left ev number where feedback is done
    fQmevFdbk =(fPair->GetRight().GetEvNumber()+
		fPair->GetLeft().GetEvNumber())*0.5; // get the mean ev number where feedback is done 
#ifdef FDBK
    clog <<" --- Enough good pair -----\n"<<" Feedback 1st stat calculation  at Event "
         <<fQStopPair<<" sum "<<fQasymMean1<<"  Mean1 "<<fQasymMean1/fNQpair<<endl; 
#endif
    fQasymMean1 = fQasymMean1/fNQpair;
    vector<Double_t>::iterator qi;
    for (qi = fQasym.begin() ; qi != fQasym.end() ; qi++){
      fQasymRMS += (*qi - fQasymMean1)*(*qi - fQasymMean1);      
    }     
    fQasymRMS = sqrt(fQasymRMS/fNQpair); //  Qasym RMS
    fQasymErr = fQasymRMS/sqrt(fNQpair); //  error on Qasym Mean
    
#ifdef FDBK1
    clog <<" *RESULTS 1st stats "<<" mean "<<fQasymMean1<<" +- "<<fQasymErr
         <<" RMS "<<fQasymRMS<<endl;
    clog <<" Start Filtering values"<<endl; 
#endif
    fNQpair = 0;    
    for ( qi = fQasym.begin(); qi != fQasym.end() ; qi++){
        // if asymmetry value is not too far from the mean value 
        // (filter very large values )    
	// let's say < 6 sigma away from the calculated mean
        if ( abs(Int_t(*qi) - Int_t(fQasymMean1)) < 6*fQasymRMS ) { 
             fNQpair++;
	     fQasymMean2 += *qi;             
            }
          else
	    {
	     fQasym.erase(qi); 
	}
    } 
#ifdef FDBK1
    clog <<"**Filtering processed, new nember of good pairs :"<<fNQpair<<endl; 
#endif
    fQasymMean2 = fQasymMean2/fNQpair;
    for (qi = fQasym.begin() ; qi != fQasym.end() ; qi++){
      fQasymRMS += (*qi - fQasymMean2)*(*qi - fQasymMean2);      
    }     
    fQasymRMS = sqrt(fQasymRMS/fNQpair); //  Qasym RMS
    fQasymErr = fQasymRMS/sqrt(fNQpair); //  error on Qasym Mean
#ifdef FDBK1
    clog <<"***RESULTS 2nd stats "<<" mean "<<fQasymMean2<<" +- "<<fQasymErr
         <<" RMS "<<fQasymRMS<<endl;
    clog <<" "<<endl; 
#endif
    fNQpair = 0;
    fQasym.clear();

    // During HAPPEX2 run we will need somethig like Qasym < 25 ppm so :
    // if absolute value of asymmetry < 25 ppm, value is ok continue 
    // feedback or if absolute value is higher than 25 ppm and the 
    // error is acceptable for us (e.g. ratio > 2) do feedback...
    if ( abs(Int_t(fQasymMean2)) < 25 || abs(Int_t(fQasymMean2)) > 25 && 
         abs(Int_t(fQasymMean2)/(Int_t(fQasymErr))) > 2 ) {
        // CRITERIA are OK, send EPICS variables 
#ifdef ONLINE
      fQsent =1;
      clog<<" *** Pan is sending Qasym EPICS values *** "<<endl; 
      pair<char*, Double_t> asyval;
      pair<char*, Double_t> asyerr;
      asyval.first = "HA:Q_ASY"; asyval.second = fQasymMean2; 
      asyerr.second = "HA:DQ_ASY"; asyerr.second = fQasymErr; 
      //fRun->SendEPICSInfo(asyval); 
      //fRun->SendEPICSInfo(asyerr); 
#endif
    }
    fQStartPair = fQStopPair;
      
    fFdbkTree->Fill();         
  } // end if fNQpair >= 2700


}

void 
TaFdbkAna::QasymEndRunFeedback(){
  // Last feedback at the end of the run. To have a chance to send 
  // a feedback, this function need enough pair to do it . 
  // it checks the QAsym vector size, compute an error and 
  // test the value and send it.

#ifdef FDBK1
  clog<< " last chance for Feedback, queue size "<<fQasym.size()<<endl;
#endif  
 
  if (fQasym.size() > 2700 ) {     
    fNQpair = fQasym.size();
    fQnum = -1; // arbitrary end feedback number is -1  
    fQsent =0; // reinitialize EPICS flag
    fQStopPair = 0; // give 0 for end feedback (easier to cut on feedback number) 
    fQlevFdbk  = fQStopPair; // get the left ev number where feedback is done
    fQmevFdbk = 0; // give 0 for end feedback 
    vector<Double_t>::iterator qi;
    for (qi = fQasym.begin() ; qi != fQasym.end() ; qi++){
      fQasymMean1+=*qi;
    }    
#ifdef FDBK
    clog <<"\n\n ++++ END Qasym FEEDBACK, Enough good pair -----\n"<<" Feedback 1st stat calculation sum "
         <<fQasymMean1<<"  Mean1 "<<fQasymMean1/fNQpair<<endl; 
#endif
    fQasymMean1 = fQasymMean1/fNQpair;
    for (qi = fQasym.begin() ; qi != fQasym.end() ; qi++){
      fQasymRMS += (*qi - fQasymMean1)*(*qi - fQasymMean1);      
    }     
    fQasymRMS = sqrt(fQasymRMS/fNQpair); //  Qasym RMS
    fQasymErr = fQasymRMS/sqrt(fNQpair); //  error on Qasym Mean
    
#ifdef FDBK1
    clog <<"+RESULTS 1st stats "<<" mean "<<fQasymMean1<<" +- "<<fQasymErr
         <<"RMS "<<fQasymRMS<<endl;
    clog <<" Start Filtering values"<<endl; 
#endif
    fNQpair = 0;    
    for ( qi = fQasym.begin(); qi != fQasym.end() ; qi++){
        // if asymmetry value is not too far from the mean value 
        // (filter very large values )    
	// let's say < 6 sigma away from the calculated mean
        if ( abs(Int_t(*qi) - Int_t(fQasymMean1)) < 6*fQasymRMS ) { 
             fNQpair++;
	     fQasymMean2 += *qi;             
            }
          else
	    {
	     fQasym.erase(qi); 
	}
     } 
#ifdef FDBK1
    clog <<"++Filtering processed, new nember of good pairs :"<<fNQpair<<endl; 
#endif
    fQasymMean2 = fQasymMean2/fNQpair;
    for (qi = fQasym.begin() ; qi != fQasym.end() ; qi++){
      fQasymRMS += (*qi - fQasymMean2)*(*qi - fQasymMean2);      
    }     
    fQasymRMS = sqrt(fQasymRMS/fNQpair); //  Qasym RMS
    fQasymErr = fQasymRMS/sqrt(fNQpair); //  error on Qasym Mean
#ifdef FDBK1
    clog <<"+++ END Qasym FEEDBACK : RESULTS 2nd stats "<<" mean "<<fQasymMean2<<" +- "<<fQasymErr
         <<" RMS "<<fQasymRMS<<endl;
    clog <<" \n\n "<<endl; 
#endif
    fNQpair = 0;
    fQasym.clear();

    // Look at criteria to send values to the source.
    if ( abs(Int_t(fQasymMean2)) < 25 || abs(Int_t(fQasymMean2)) > 25 && 
         abs(Int_t(fQasymMean2)/(Int_t(fQasymErr))) > 2 ) {
        // CRITERIA are OK, send EPICS variables 
#ifdef ONLINE
      fQsent =1;
      clog<<" *** Pan is sending Qasym EPICS values *** "<<endl; 
      pair<char*, Double_t> asyval;
      pair<char*, Double_t> asyerr;
      asyval.first = "HA:Q_ASY"; asyval.second = fQasymMean2; 
      asyerr.second = "HA:DQ_ASY"; asyerr.second = fQasymErr; 
      //fRun->SendEPICSInfo(asyval);
      //fRun->SendEPICSInfo(asyerr); 
#endif
     }
    fFdbkTree->Fill();            
  }
  else{
    cout<<"+++++ NO END FEEDBACK, NOT ENOUGH PAIRS !!! +++++"<<endl;
  } 
} 



// We should not need to copy or assign an analysis, so copy
// constructor and operator= are private.

TaFdbkAna::TaFdbkAna (const TaFdbkAna& copy) 
{

}


TaFdbkAna& TaFdbkAna::operator=( const TaFdbkAna& assign) 
{ 
  return *this; 
}














