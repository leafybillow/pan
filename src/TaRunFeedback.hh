#ifndef PAN_TaRunFeedBack
#define PAN_TaRunFeedback

//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaRunFeedback.hh  (header file)
//           ^^^^^^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
// Run class with feedback for online Analysis and monitoring. This class is 
// derived from the TaRun class. To make ONLINE version of Pan with
// feedback enabled, replace TaRun by TaRunFeedBack.
//
// 
//  Preliminary NOTES :
//
//  This class require some precompilation flags to work correctly.
//
//  Some member functions need some "Getter" from VaAnalysis class :
// 
//   1. A way to get the number of good pairs (function GetPairUsed() in AparAsyAna).
//      I keep the same name for this function here....      
//   2. Current pair flag "keepair" ( something like GetPairKeepair() )
//   3. bcm1 or/and bcm2 asym from the current pair : GetBcm1asy(), Getbcm2asy()
//      I assume that feedback functions will be called after each pair made    
//   4. The PairTree. End feedback need to read this tree.     
//
//  WARNING :EPICS variables and EPICS scripts should be setted properly 
//           before using this class (see RunChargeFBK() and EndRunChargeFBK() members )  
//
////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "TaRun.hh"
#include "TTree.h"
#include "TH1.h"
#include "PanTypes.hh" 
#include <vector>
#include <string>
#include <map>

class VaPair;
class VaAnalysis;

class TaRunFeedback : public  TaRun {

 public:

       // Constructors/destructors/operators
  TaRunFeedback();   
   TaRunFeedback(const Int_t& run);
   TaRunFeedback(const string& filename);
  ~TaRunFeedback(); 
       // Copy constructor and operator= are defined privately.
       // Major functions :
  virtual void Init();
  virtual void AccumEvent(const TaEvent&);
       // All operations on events after decoding and filtering are made here
  virtual void AccumPair(const VaPair&);
       // All operations on Pairs after filtering are made here. 

  void RunChargeFBK(const VaAnalysis& analysis);  
       // Charge asymmetry feedback. This function make 3 passes on the data 
       // 1. Calculates stats variables after X number of events (pairs)
       // 2. Filters values of charge asymmetry too far from the mean.
       // 3. Recompute mean value and RMS. 
       // then if the value is acceptable for our criteria, send value as 
       // EPICS variables to the source.           
  void EndRunChargeFBK(const VaAnalysis& analysis);
       // End Run feedbacks. Same kind of method for these feedbacks.   

       // No PZT mirror feedback implemented yet since we don't know if we are going to
       // use Pan at the source (injector DAQ).  

  virtual void Finish();
       // this function will end the run object. It contains last feedbacks 
       // and  Stats operation before ending the run.  

       // Data access functions

private:

       // Copy constructor and operator= -- defined null and private
  TaRunFeedback(const TaRunFeedback& run);
  TaRunFeedback& operator=(const TaRunFeedback& run);

       // Member functions :
  void EvStats(const TaEvent& ev);
       // Statistics function for event stats computing. 
  void PairStats(const VaPair& p);
       // Statisitcs function for Pair stats. 

  // Data members
  Int_t fRunNum;
  Int_t fStartPair;      // feedback minirun start at pair fStartPair
  Int_t fStopPair;       // feedback minirun stop at pair fStopPair
  Int_t fGoodPair;       // good pairs
  Int_t fNpair;          // Number of good pair counter 
  Int_t fNfbkminirun;    // feedback minirun counter
  Int_t fNstatminirun;   // stat minirun counter 
  vector <Double_t>  fAsym; 
  Double_t           fAsymbar1, fAsymbar2, fAsymsig, fAsymsigave;
  TH1F*              fbkhisto;  // histogram for end feedback.
  FILE*              frcfile;
  FILE*              ffbk;
  //  TTree*             fbktree;  // it could be interesting to have a tree for feedbacks..

#ifdef DICT
  ClassDef (TaRunFeedback, 0) 
#endif

};

#endif
