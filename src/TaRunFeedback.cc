//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRunFeedback.cc    (implementation)
//           ^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
// Run class with feedback for online Analysis and monitoring. This class is 
// derived from the TaRun class. To make ONLINE version of Pan with
// feedback enabled, replace TaRun by TaRunFeedBack.
//
//
////////////////////////////////////////////////////////////////////////

#include "TaRunFeedback.hh"
#include "VaPair.hh"
#include "VaAnalysis.hh"
#include <iostream>
#include <stdlib.h>
#include "THaCodaData.h"
#ifdef ONLINE
#include "THaEtClient.h"
#endif
#include "THaCodaFile.h"
#include "THaCodaData.h"
#include "TaEvent.hh"
#include "VaDataBase.hh"
#include "TaAsciiDB.hh"
#ifdef CUTLISTDEFINED  // Eventually we remove this ifdef
#include "TaCutList.hh"
#endif
#ifdef OTHERDEVICE  // I only define TaScaler in round 3
#include "TaBPM.hh"
#endif
#include "TaBCM.hh"
#include "TaScaler.hh"




#ifdef DICT
ClassImp(TaRunFeedback)
#endif

// Constructors/destructors/operators
  // Same c'tor as TaRun but send EPICS variables if ONLINE c'tor is
  // chosen (Same kind of safety as we used in summer 2001 for He3 exp).  
  // 2 precompiling options needed to run Pan with full REAL feedbacks :
  // 1. Need to enable ONLINE analysis.
  // 2. Need to enable Charge asymmetry feedback flag. 
  //
  // If it is not ONLINE and feedback enabled, Pan will not change 
  // EPICS variables for safety. "Virtual" feedbacks are possible.

TaRunFeedback::TaRunFeedback():TaRun() {
      // ------------------ ONLINE  safety -----------------------------------
#ifdef ONLINE
  cout<<"TaRunFeedback:: WARNING: Pan is running ONLINE "<<endl;

      // ------------------- Aq FEEDBACK SAFETY ------------------------------
#ifdef AQFBK
  cout<<"TaRunFeedback:: WARNING: REAL FEEDBACKS on CHARGE ASYMMETRY ENABLED "<<endl;           
#else
  cout<<"TaRunFeedback:: WARNING: REAL FEEDBACKS on CHARGE ASYMMETRY DISABLED "<<endl; 
  cout<< " This mode doesn't send feedback values to the SOURCE Feedback System."<<endl;
  cout<<" If you want to modify EPICS variables you need to set"<<endl;
  cout<<"precompilation Feedback flag properly (AQFBK) in the Makefile and recompile Pan."<<endl;     
#endif
#endif 
}

TaRunFeedback::TaRunFeedback(const Int_t& run): TaRun(run) {
#ifdef ONLINE
  cout<<" WARNING: ONLINE option is ON. "<<endl;
#endif
} 

TaRunFeedback::TaRunFeedback(const string& filename): TaRun(filename) {
#ifdef ONLINE
  cout<<" WARNING: ONLINE option is ON. "<<endl;
#endif
}

TaRunFeedback::~TaRunFeedback(){}

void TaRunFeedback::Init(){
 TaRun::Init();
}

void TaRunFeedback::EvStats (const TaEvent& ev){
  // to be filled with proper TaStats functions for ev
}

void TaRunFeedback::PairStats (const VaPair& p){
  // to be filled with proper TaStats functions for Pair
}
void TaRunFeedback::AccumEvent(const TaEvent& ev){
  EvStats(ev);
}
void TaRunFeedback::AccumPair(const VaPair& p){
  PairStats(p);
}



void TaRunFeedback::RunChargeFBK(const VaAnalysis& analysis) {
fStopPair = 0; 
 if (fStartPair == -1 ) 
   {
    // need to get the current number of good pairs if start or restart a feedback minirun.
    fStartPair = analysis.GetPairsUsed();  
   }

    // count is made on good pairs 
 if (analysis.GetPairKeepPair() == 1) 
   { 
    fNpair++;
    fAsym.push_back(analysis.GetBcm1Asy());
    fAsymbar1 += analysis.GetBcm1Asy();
    //    cout<<" enter keep pair loop :  asym "<<analysis.GetBcm1Asy()<<" sum "<<asymbar1<<" number of pairs "<<fNpair<<" paircounter "<<fNpair<<endl;
   }
           // start feedback when get enough good pairs : typically 3 minutes 
           //  3*60*30/2 = 2700              
 if (fNpair > 2700 )
   {
     // declare char* to use C  string functions sprintf etc....
     // instead of istringstream classes....since egcs compiler on 
     // adaqcp and adaql2 is not yet recognizing <sstream> library.
       
    Double_t epics_value1;
    Double_t epics_value2;
    char* variable1  = new char[50];
    char* value1     = new char[50];
    char* variable2  = new char[50];
    char* value2     = new char[50];
    char* epics_asym_name  = "HA:Q_ASY";  // summer 2001 EPICS var name...
    char* epics_error_name = "HA:DQ_ASY";
    char* epics_asym_value = new char[100];
    char* epics_error_value = new char[100];
    char* command1   = new char[100];
    char* command2   = new char[100];
    
    //cout<<" vector size : "<<fAsym.size()<<" vector capacity : "<<fAsym.capacity()<<endl; 
    //cout<<" start pair "<<fStartPair<<endl;; 
    fAsymbar1 = fAsymbar1/fNpair; // calulate charge asym mean for bunch of pair
    // cout<<" enter enough pair loop  mean "<<fAsymbar1<<endl;
 
             // FIRST PASS to calculate statistics variables
    vector<Double_t>::iterator  idx;  
     for ( idx = fAsym.begin() ; idx != fAsym.end() ; idx++)
       {
	fAsymsig += ( *idx - fAsymbar1 )*( *idx - fAsymbar1 );  
       } 
     fAsymsig    = sqrt( fAsymsig / fNpair ); // RMS
     fAsymsigave = fAsymsig / sqrt(fNpair); // error on mean
     fNpair = 0;
     // cout<<" first PASS : mean1 "<<fAsymbar1<<" RMS "<<fAsymsig<<" error "<<fAsymsigave<<endl;
            // SECOND PASS to test/reject bad values... 
     for ( idx = fAsym.begin(); idx != fAsym.end() ; idx++)
 	{
          // if asymmetry value is not too far from the mean value 
          // (filter very large values )    
	  // let's say < 6 sigma away from the calculated mean
          if ( abs(Int_t(*idx) - Int_t(fAsymbar1)) < 6*fAsymsig ) 
	    { 
             fNpair++;
	     fAsymbar2 += *idx;             
            }
          else
	    {
	     fAsym.erase(idx); 
	    }
        } 
        //  loop to see if filtering correctly
     for ( idx = fAsym.begin(); idx != fAsym.end() ; idx++)
 	{
	  // cout<<"Asym value after filtering "<<*idx<<endl;
	}
     fAsymbar2 = fAsymbar2/fNpair; // recompute mean
     cout<<"Second PASS : mean2 "<<fAsymbar2<<" number of pairs "<<fNpair<<endl;
          // third pass to get the mean value and error on the asymmetry         
     for ( idx = fAsym.begin() ; idx != fAsym.end() ; idx++)
       {
        fAsymsig += (*idx - fAsymbar2)*(*idx - fAsymbar2);
       }
     fAsymsig =sqrt(fAsymsig/fNpair);     // recompute RMS after filtering 
     fAsymsigave = fAsymsig/sqrt(fNpair); // recompute error on mean 
          // clear vectors and variables 
     fNpair = 0;
     fAsym.clear();
     cout<<"Third PASS : mean "<<fAsymbar2<<" RMS "<<fAsymsig<<" error "<<fAsymsigave<<endl;

          // for the future we will need somethig like < 25 ppm so :
          // if absolute value of asymmetry < 25 ppm, value is ok continue 
          // feedback or if absolute value is higher than 25 ppm and the 
          // error is acceptable for us (e.g. ratio > 2) do feedback...
    
     if ( abs(Int_t(fAsymbar2)) < 25 || abs(Int_t(fAsymbar2)) > 25 && 
          abs(Int_t(fAsymbar2)/(Int_t(fAsymsigave))) > 2 ) {
         // CRITERIA are OK, send EPICS variables 
         cout<<"START pair "<<fStartPair<<endl;
         fStopPair = analysis.GetPairsUsed(); 
         cout<<"STOP pair "<<fStopPair<<endl;         
         // recopy the value of the mean 
         epics_value1 = fAsymbar2;
         sprintf(epics_asym_value,"%6.0f",epics_value1); 
         strcpy(variable1, epics_asym_name);
         strcpy(value1   , epics_asym_value);
         // dummy name for the script right now.....
         sprintf(command1,"/adaqfs/halla/Pan/feedback/feedback_shscript");
         strcat(command1,variable1);
         strcat(command1," ");
         strcat(command1,value1);
         // recopy the value of the error on the mean 
         epics_value2 = fAsymsigave;
         sprintf(epics_error_value,"%6.0f",epics_value2); 
         strcpy(variable2, epics_error_name);
         strcpy(value2   , epics_error_value);
         sprintf(command2,"/adaqfs/halla/Pan/feedback/feedback_shscript");
         strcat(command2,variable2);
         strcat(command2," ");
         strcat(command2,value2);
#ifdef ONLINE
#ifdef AQFBK
          // if we keep He3 experiment feedback style,  
          // format will be something like ( value in ppm ) : scriptname Q_ASY 14   
          cout << "Shell command = "<<command1<<endl<<flush;          
          system(command1);
          // format will be something like (value in ppm ) : scriptname DQ_ASY 5  
          cout << "Shell command = "<<command2<<endl<<flush;
          system(command2);       
#endif 
#endif
         fNfbkminirun++; // number of "feedback minirun"
	  // to keep fbk file updated, need runnumber...
	 fRunNum = GetRunNumber();      
         delete [] command1;  delete [] command2;  
         delete [] variable1; delete [] variable2;
         delete [] value1;    delete [] value2;
         delete [] epics_error_name; delete [] epics_error_value; 

#ifdef AQFBK
          // Will probably need to keep track of feedback value by 
          //writting on a file.
// ffbk = fopen("/adaqfs/halla/apar/feedback/historyfastasy.dat","a");
// fprintf(ffbk, "Run %d : minirun  %d  asym  = %6.0f  +- %6.0f \n",RunNum,Nfeed,fAsymbar2,fAsymsigave);
// fclose(ffbk); 
#endif
          // We can also fill a feedback tree for the root file....
// FBKtree.Fill()              
       }    
     else
       {
#ifdef AQFBK
         // keep track when feedback is not done (criteria not approved)   
	 //          ffbk = fopen("/adaqfs/..../historyfile.dat","a");
	 // fprintf(ffbk, " Run %d : NO FEEDBACK DONE at pair #   \n",RunNum,asyana->GetPairsUsed());
	 //fclose(ffbk);
#endif
       cout<<" No FASTFEEDBACK DONE at pair # "<<fStopPair<<endl;
       }
     fStartPair = -1;
   }
}

void TaRunFeedback::EndRunChargeFBK( const VaAnalysis& analysis) {


Int_t Nstatpair,RunNum, i, npair;
Float_t  Mean,Sigave,Rms;

//TBranch* branch_event = apartree->GetBranch("evcntr");
TBranch* branch_c1 = analysis.GetPairTree()->GetBranch("bcm1asy");
TBranch* branch_c2 = analysis.GetPairTree()->GetBranch("keepair");
Double_t bcm1asycp; 
UInt_t keep_paircp;

 branch_c1->SetAddress(&bcm1asycp);
 branch_c2->SetAddress(&keep_paircp);
 RunNum = GetRunNumber();
 cout<<"  =================== End feedback ============="<<endl;
 // cout<<"runnumber :"<<RunNum<<endl; 
 // cout<<"number of pairs used :"<<analysis.GetPairsUsed()<<endl;
  
 // test if number of good pairs is bigger than 3 mn of run. 
 if ( analysis.GetPairsUsed() > 3000 )
    { 
     npair = (Int_t) analysis.GetPairTree()->GetEntries();
     fGoodPair = 0; 
     cout<<" NUMBER OF PAIRS in tree : "<<npair<<endl;  
     //     cout<< " START VALUE OF i : "<<npair - 4500<<endl;
     // count in the last 5 minutes of event counting if enough good pairs 
     // to avoid counting a pair in two different feedback.
     cout<<" npair - 2700 : "<<npair-2700<<endl;
     cout<<" fStopPair "<<fStopPair<<endl; 
     if ( npair - 2700 > fStopPair )
       {    
        for (i = npair - 2700 ; i < npair ; i++)
          {
            branch_c2->GetEvent(i);
            if ( keep_paircp > 0 ) fGoodPair++;  
          }
       }
    } 
 cout<<" END fGoodPair : "<<fGoodPair<<endl;  
      // if enough good pairs for last three minutes continue
 if ( fGoodPair > 2700 ) 
   {
    for (i = npair - 2700 ; i < npair ; i++)
     {
      branch_c1->GetEvent(i);
      branch_c2->GetEvent(i);
      if ( keep_paircp > 0 ) 
         {
          fbkhisto->Fill(bcm1asycp);
         }
     }
     Nstatpair = (Int_t) fbkhisto->GetEntries();     
     Mean  = fbkhisto->GetMean();
     Rms   = fbkhisto->GetRMS();
     Sigave= Rms/sqrt( Nstatpair );
     cout<<" Nstatpair : "<<Nstatpair<<" Mean : "<<Mean<<" Rms : "<<Rms<<" Sigave : "<<Sigave<<endl;    
      // if sigma in average is small enough ...
      if ( abs(Int_t(Mean)) < 25 || abs(Int_t(Mean)) > 25 && abs(Int_t(Mean)/(Int_t(Sigave))) > 2 && Int_t(Mean) != 0) 
        { 
         char *variable1 = new char[50];
         char *value1    = new char[50];
         char *variable2 = new char[50];
         char *value2    = new char[50];
         char *command1 = new char[100];
         char *command2 = new char[100];
         Double_t epics_value;
         char* epics_asym_name   = "HA:Q_ASY";
         char* epics_asym_value  = new char[100];
         char* epics_error_name  ="HA:DQ_ASY";
         char* epics_error_value = new char[100];

         epics_value = Mean;
         sprintf(epics_asym_value,"%6.0f",epics_value); 
         strcpy(variable1, epics_asym_name);
         strcpy(value1   , epics_asym_value);

         sprintf(command1,"/adaqfs/halla/.../Aq_feedback_shell_script");
         strcat(command1,variable1);
         strcat(command1," ");
         strcat(command1,value1);
         epics_value = Sigave;
         sprintf(epics_error_value,"%6.0f",epics_value); 
         strcpy(variable2, epics_error_name);
         strcpy(value2   , epics_error_value);

         sprintf(command2,"/adaqfs/halla/apar/feedback/Aq_feedback_shell_script ");
         strcat(command2,variable2);
         strcat(command2," ");
         strcat(command2,value2);
#ifdef ONLINE
#ifdef AQFBK
           cout << "Shell command = "<<command1<<endl;
           cout << "Shell command = "<<command2<<endl;
           system(command1);
           system(command2);       
           ffbk = fopen("/adaqfs/halla/apar/feedback/feedbackhistory.dat","a");
	   fprintf(ffbk, " Run %d : END minirun **  asym  = %6.0f +- %6.0f \n",RunNum,Mean,Sigave);
	   fprintf(ffbk, " -------------------------------------------------------------------------\n");
	   fclose(ffbk);
#endif
#endif
         delete [] command1;  delete [] command2;  
         delete [] variable1; delete [] variable2 ;
         delete [] value1;    delete [] value2 ;
         delete []epics_error_name; delete [] epics_error_value; 

         cout<<" After "<<npair<<" Pairs Apar sent feedback info : "<<Mean<<" +- "<<Sigave<<endl;
         cout<<"run NUMBER : "<<RunNum<<endl;
 	}
      else
       {    
        cout<<"no feedback made, value of asym not valid  ";   
	cout<<" asym "<<Mean<<" +- "<<Sigave<<endl;
#ifdef AQFBK  
          ffbk = fopen("/adaqfs/......./feedbackhistory.dat","a");
	  fprintf(ffbk, " Run %d : NO FEEDBACK DONE AT END minirun : asym not valid \n",RunNum);
 	  fprintf(ffbk, " -------------------------------------------------------------------------\n");
	  fclose(ffbk);
#endif
       } 
   }
   else 
    {
      cout<<" NO FEEDBACK DONE AT END minirun : not enough good events "<<endl; 
#ifdef AQFBK  
      ffbk = fopen("/adaqfs/halla/apar/feedback/historyfastasy.dat","a");
      fprintf(ffbk, " Run %d : NO FEEDBACK DONE AT END minirun : not enough good events \n",RunNum);
      fprintf(ffbk, " -------------------------------------------------------------------------\n");
      fclose(ffbk);
#endif
    
    }
    
}

void TaRunFeedback::Finish(){
  TaRun::Finish();
}












