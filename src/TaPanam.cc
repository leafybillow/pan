//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan
//
//           TaPanam.cc
//
// Author: A. Vacheret <http://www.jlab.org/~vacheret>
// 
////////////////////////////////////////////////////////////////////////
//
//  Graphical user interface using Pan analysing for monitoring 
//  Inspiration for layout is from ALICE testbeam monitoring, E158 and 
//  ROOT test example.   
//
////////////////////////////////////////////////////////////////////////

#define SIGNAL
#define DEBUGDCOUNT
#define DEBUGOPTION
#define REFRESH
#define CODAFILETEST
//#define ETCONNECT

#include "TaPanam.hh"
#include "TaString.hh"

#include <sstream>

ClassImp(TaPanam)

TaPanam::TaPanam()  
  : TGMainFrame(0, 0, 0),TaThread(0)
{
 // Needed by Root for the dictionnary
 fMainTitle=0;fUtilFrame=0;
 fHistoFrame=0;fButtonFrame=0;
 fMainFrame=0;fEmCanFrame=0;
 // folders frames
 fTimeFrame=0;fTimegFrame=0;fTOptgFrame=0;
 fADCScalerFrame=0;fADCFrame0=0;fADCFrame1=0;fADCFrame2=0;fADCFrame3=0;fADCOptgFrame=0;
 fCurrentFrame=0;fInjBPMWSFrame=0;fInjBPMWSDispFrame=0;fInjBPMWSOptFrame=0;
 fBCMFrame=0;fBCMDispFrame=0;fBCMOptFrame=0;fBCMCavFrame=0;
 fCurrentgFrame=0;fInjBPMWSgFrame=0;fBCMgFrame=0;
 fPositionFrame=0;fInjBPMFrame=0;
 fBPMFrame=0;fBPMDispFrame=0;fBPMOptFrame=0;
 fBPMCavFrame=0;
 fPositiongFrame=0;fBPMgFrame=0;
 fFeedbackFrame=0;fFCurFrame=0;fFPosFrame=0;fDITHFrame=0;fFCurgFrame=0;
 fFPosgFrame=0;fDITHgFrame=0;fControlFrame=0;
 fDetFrame=0;fDalFrame=0;fLumiFrame=0;fDalgFrame=0;fLumigFrame=0;
 // radio buttons
 
 fTitle=0;fL1=0;fL2=0;fL3=0;fL4=0;fL5=0;fL6=0;fL7=0;fL8=0;fL9=0;
 fLogoButton=0;
 fButton1=0;fButton2=0;fButton3=0;
 fHistoTreeView=0;
 fEmCan=0;
 fMenuBar=0;fMenuBarLayout=0;fMenuBarItemLayout=0;
 fMenuFile=0;fMenuMon=0;fMenuAna=0;
 fHistoListBox=0;fTab=0;
 fStripStyle=0;fHistoStyle=0;fGraphStyle=0;
 fBCMplot = 0; fBPMplot =0; 
}


TaPanam::TaPanam(const TGWindow *p, UInt_t w, UInt_t h)
  : TGMainFrame(p, w, h), TaThread(2)
{ 
  if (InitFlags()) 
   {
     cout<<" InitFlags() : Flags Initialized "<<endl;
     //DumpOptionState(); 
   }
  if (InitParameters()) cout<<" InitParameters() : Parameters Initialized "<<endl;
  InitGUIFrames();
  if (CheckDevListConfig()) cout<<" CheckDevListConfig() : Check user device config "<<endl;
}

TaPanam::~TaPanam()
{
  delete fMenuBarLayout;
  delete fMenuBarItemLayout;
  delete fL1; delete fL2; delete fL3;delete fL4; delete fL5; delete fL6; delete fL7;
  delete fL8; delete fL9;
  delete fTimeFrame;      delete  fTimegFrame;     delete  fTOptgFrame; 
  delete fADCScalerFrame; delete  fADCFrame0;      delete  fADCFrame1; 
  delete fADCFrame2;      delete  fADCFrame3;      delete  fADCOptgFrame; 
  delete fCurrentFrame;   delete fInjBPMWSFrame;
  delete fBCMFrame;       delete fBCMDispFrame;    delete fBCMOptFrame;       delete fBCMCavFrame; 
  delete fCurrentgFrame;  delete fBCMgFrame; 
  delete fPositionFrame;  delete fInjBPMFrame; 
  delete fBPMFrame;       delete fBPMDispFrame;    delete fBPMOptFrame; 
  delete fBPMCavFrame;
  delete fPositiongFrame; delete fBPMgFrame;
  delete fFeedbackFrame;  delete fFCurFrame;       delete fFPosFrame;         delete fFCurgFrame;
  delete fDITHFrame;      delete fDITHgFrame;      delete fControlFrame; 
  delete fFPosgFrame;     delete fDetFrame;        delete fDalFrame;          delete fLumiFrame; 
  delete fDalgFrame;      delete fLumigFrame; 
  
  delete fMenuFile;
  delete fMenuMon;
  delete fMenuAna;
  delete fMenuBar; 
}

void TaPanam::CloseWindow()
{
 TGMainFrame::CloseWindow();
 gApplication->Terminate(0);
}


Int_t TaPanam::CheckDevListConfig()
{
 if (fTimeCheck[0]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[0],string("quadsynch"),1); 
 if (fTimeCheck[1]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[1],string("pairsynch"),1);
 if (fTimeCheck[2]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[2],string("helicity"),1);
 if (fTimeCheck[3]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[3],string("timeslot"),1);
 if (fTimeCheck[4]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[4],string("v2fclock"),1);

 // if (fIADCCheck[0]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[0],string("adc10"),1);;
 // if (fIADCCheck[1]->GetState() == kButtonDown) UpdateDevListWithButton(fTimeCheck[0],string("adc11"),1);;

 if (fADCCheck[0]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[0],string("adc0"));
 if (fADCCheck[1]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[1],string("adc1"));
 if (fADCCheck[2]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[2],string("adc2"));
 if (fADCCheck[3]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[3],string("adc3"));
 if (fADCCheck[4]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[4],string("adc4"));
 if (fADCCheck[5]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[5],string("adc5"));
 if (fADCCheck[6]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[6],string("adc6"));
 if (fADCCheck[7]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[7],string("adc7"));
 if (fADCCheck[8]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[8],string("adc8"));
 if (fADCCheck[9]->GetState() == kButtonDown) UpdateADCListWithButton(fADCCheck[9],string("adc9"));
 
 // if (fSADCCheck[0]->GetState() == kButtonDown) ;  
 // if (fSADCCheck[1]->GetState() == kButtonDown) ;

 // if (fInjBPMWSCheck[0]->GetState() == kButtonDown) UpdateDevListWithButton(fInjBPMWSCheck[0],string("bpmin1ws"),2);
 // if (fInjBPMWSCheck[1]->GetState() == kButtonDown) UpdateDevListWithButton(fInjBPMWSCheck[1],string("bpmin2ws"),2);
 if (fBCMCheck[0]->GetState() == kButtonDown) UpdateDevListWithButton(fBCMCheck[0],string("bcm1"),2);
 if (fBCMCheck[1]->GetState() == kButtonDown) UpdateDevListWithButton(fBCMCheck[1],string("bcm2"),2);
 if (fBCMCheck[2]->GetState() == kButtonDown) UpdateDevListWithButton(fBCMCheck[2],string("bcm3"),2);
 //if (fBCMCavCheck[0]->GetState() == kButtonDown) UpdateDevListWithButton(fBCMCavCheck[0],string("bcmcav1"),2);
 //if (fBCMCavCheck[1]->GetState() == kButtonDown) UpdateDevListWithButton(fBCMCavCheck[1],string("bcmcav2"),2);

  
 // if (fInjBPMCheck[0]->GetState() == kButtonDown)  UpdateBPMListWithButton(fInjBPMCheck[0],string("bpmin1"),2); 
 // if (fInjBPMCheck[1]->GetState() == kButtonDown)  UpdateBPMListWithButton(fInjBPMCheck[1],string("bpmin2"),2);

 if (fBPMCheck[0]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCheck[0],string("bpm12"),2);
 if (fBPMCheck[1]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCheck[1],string("bpm10"),2);
 if (fBPMCheck[2]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCheck[2],string("bpm8"),2);
 if (fBPMCheck[3]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCheck[3],string("bpm4a"),2);
 if (fBPMCheck[4]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCheck[4],string("bpm4b"),2);



 // if (fBPMCheck[5]->GetState() == kButtonDown)  UpdateBPMListWithTargetPos();
//  if (fBPMCavCheck[0]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCavCheck[0],string("bpmcav1"),2);
//  if (fBPMCavCheck[1]->GetState() == kButtonDown)  UpdateBPMListWithButton(fBPMCavCheck[1],string("bpmcav2"),2);
  
 if (fDalCheck[0]->GetState() == kButtonDown)  UpdateDevListWithButton(fDalCheck[0],string("det1"),2);
 if (fDalCheck[1]->GetState() == kButtonDown)  UpdateDevListWithButton(fDalCheck[1],string("det2"),2);
 if (fDalCheck[2]->GetState() == kButtonDown)  UpdateDevListWithButton(fDalCheck[2],string("det3"),2);
 if (fDalCheck[3]->GetState() == kButtonDown)  UpdateDevListWithButton(fDalCheck[3],string("det4"),2);

 if (fLumiCheck[0]->GetState() == kButtonDown)  UpdateDevListWithButton(fLumiCheck[0],string("lumi1"),2);
 if (fLumiCheck[1]->GetState() == kButtonDown)  UpdateDevListWithButton(fLumiCheck[1],string("lumi2"),2);
 if (fLumiCheck[2]->GetState() == kButtonDown)  UpdateDevListWithButton(fLumiCheck[2],string("lumi3"),2);
 if (fLumiCheck[3]->GetState() == kButtonDown)  UpdateDevListWithButton(fLumiCheck[3],string("lumi4"),2);
 return 1;
}


Int_t TaPanam::CheckOptionConfig()
{
 if (fTOptCheck[0]->GetState() == kButtonDown) fShowTime = kTRUE;
 else fShowTime = kFALSE; 
 if (fTOptCheck[1]->GetState() == kButtonDown) fTSCorH = kFALSE;
 if (fTOptCheck[2]->GetState() == kButtonDown) fTSCorH = kTRUE; 

 if (fADCOptCheck[0]->GetState() == kButtonDown) fShowIADC = kTRUE;
 else fShowIADC = kFALSE;
 if (fADCOptCheck[1]->GetState() == kButtonDown) fShowPADC = kTRUE;
 else fShowPADC = kFALSE;
 if (fADCOptCheck[2]->GetState() == kButtonDown) fShowSADC = kTRUE;
 else fShowSADC = kFALSE;
 if (fADCOptCheck[3]->GetState() == kButtonDown) fADCdata = kFALSE;
 if (fADCOptCheck[4]->GetState() == kButtonDown) fADCdata = kTRUE;
 if (fADCOptCheck[5]->GetState() == kButtonDown) fADCSCorH = kFALSE;
 if (fADCOptCheck[6]->GetState() == kButtonDown) fADCSCorH = kTRUE;

 if (fBCMDCheck[0]->GetState() == kButtonDown)    fShowBCM =kTRUE;
 else fShowBCM = kFALSE;
 if (fBCMDCheck[1]->GetState() == kButtonDown)    fShowBCMCAV = kTRUE;
 else fShowBCMCAV = kFALSE;
 if (fBCMDispCheck[0]->GetState() == kButtonDown) fBCMplot = 1;
 if (fBCMDispCheck[1]->GetState() == kButtonDown) fBCMplot = 3;
 if (fBCMDispCheck[2]->GetState() == kButtonDown) fBCMplot = 2;
 if (fBCMOptCheck[0]->GetState() == kButtonDown)  fBCMSCorH =kFALSE;
 if (fBCMOptCheck[1]->GetState() == kButtonDown)  fBCMSCorH =kTRUE;
 if (fBPMDCheck[0]->GetState() == kButtonDown)    fShowINBPM = kTRUE;
 else fShowINBPM = kFALSE;
 if (fBPMDCheck[1]->GetState() == kButtonDown)    fShowBPM   =kTRUE;
 else fShowBPM = kFALSE;
// if (fBPMDCheck[2]->GetState() == kButtonDown)     DisplayThis(fBPMDCheck[2],fShowBPMCAV);
 if (fBPMDispCheck[0]->GetState() == kButtonDown) fBPMplot=1;
 if (fBPMDispCheck[1]->GetState() == kButtonDown) fBPMplot=3;
 if (fBPMDispCheck[2]->GetState() == kButtonDown) fBPMplot=2;
 if (fBPMOptCheck[0]->GetState() == kButtonDown)  fBPMSCorH = kFALSE;
 if (fBPMOptCheck[1]->GetState() == kButtonDown)  fBPMSCorH = kTRUE;
 if (fFCurCheck[0]->GetState() == kButtonDown) 
    {
     fShowFDBK = kTRUE;
     fPITA     = kTRUE;
    }
 if (fFCurCheck[1]->GetState() == kButtonDown) 
    {
     fShowFDBK =kTRUE;
     fIA       =kTRUE;
    } 
 if (fFPosCheck->GetState() == kButtonDown) 
    {
     fShowFDBK =kTRUE;
     fPZT      =kTRUE;
    }
 // if (fDITHCheck[0]->GetState() == kButtonDown) fShowDITH = kTRUE;
 // if (fDITHCheck[1]->GetState() == kButtonDown) fShowDITH = kTRUE;

 return 1;
}


Int_t TaPanam::InitFlags()
{
  fFirstAna=kFALSE;
  fPrestart=kFALSE,fStart=kFALSE,fStop=kFALSE,fEnd=kTRUE; 

  fShowTime = kFALSE;   fShowPADC = kFALSE;
  fShowIADC = kFALSE;   fShowSADC = kFALSE;
  fShowBCM    = kFALSE; fShowBCMCAV = kFALSE;
  fShowINBPM = kFALSE;  fShowBPM    = kFALSE; fShowBPMCAV = kFALSE;
  fShowFDBK   = kFALSE; fShowDITH   = kFALSE;
  fShowLUMI   = kFALSE; fShowDET    = kFALSE;

  fTSCorH = kFALSE;  fADCSCorH  = kFALSE;
  fBCMSCorH = kFALSE; fBPMSCorH= kFALSE;
  fADCdata = kFALSE; fPITA = kFALSE; fIA =kFALSE; fPZT=kFALSE;
  fBCMplot=1; fBPMplot=1;
  return 1;
}


Int_t TaPanam::CheckCanvasConfig()
{ 
//   // reset 
//   fShowTime = kFALSE;   fShowPADC = kFALSE;
//   fShowIADC = kFALSE;   fShowSADC = kFALSE;
//   fShowBCM    = kFALSE; fShowBCMCAV = kFALSE;
//   fShowINBPM = kFALSE;  fShowBPM    = kFALSE; fShowBPMCAV = kFALSE;
//   fShowFDBK   = kFALSE; fShowDITH   = kFALSE;
//   fShowLUMI   = kFALSE; fShowDET    = kFALSE;
 
 // check buttons state before starting monitoring Pan
  if (fTOptCheck[0]->GetState() == kButtonDown)   fShowTime   = kTRUE;   
  if (fADCOptCheck[0]->GetState() == kButtonDown)   fShowIADC   = kTRUE;
  if (fADCOptCheck[1]->GetState() == kButtonDown)   fShowPADC   = kTRUE;
  if (fADCOptCheck[2]->GetState() == kButtonDown)   fShowSADC   = kTRUE;
  if (fBCMDCheck[0]->GetState()   == kButtonDown)   fShowBCM    = kTRUE;
  if (fBCMDCheck[1]->GetState()   == kButtonDown)   fShowBCMCAV = kTRUE;
  if (fBPMDCheck[0]->GetState()   == kButtonDown)   fShowINBPM  = kTRUE;
  if (fBPMDCheck[1]->GetState()   == kButtonDown)   fShowBPM    = kTRUE;
  if (fBPMDCheck[2]->GetState() == kButtonDown)   fShowBPMCAV = kTRUE;
  if (fFPosCheck->GetState() == kButtonDown) fShowFDBK = kTRUE;
  for (Int_t i=0; i<2; i++)
   {
     if (fFCurCheck[i]->GetState()     == kButtonDown)   fShowFDBK  = kTRUE;
     //     if (fDITHCheck[i]->GetState()     == kButtonDown)   fShowDITH  = kTRUE;
   }
  for (Int_t i=0; i<4; i++)
   {
     if (fDalCheck[i]->GetState() == kButtonDown)  fShowDET = kTRUE;
     if (fLumiCheck[i]->GetState() == kButtonDown) fShowLUMI = kTRUE;
   } 
  return 1;
}

Bool_t TaPanam::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
#ifdef DEBUG
      cout << "GET_MSG = " << GET_MSG(msg) << endl;
      cout << "GET_SUBMSG = " << GET_SUBMSG(msg) << endl;
#endif 
   Int_t status;
 switch (GET_MSG(msg))
    { 
    case kC_COMMAND:
      cout<<"kC_COMMAND called"<<endl;
     switch (GET_SUBMSG(msg)) 
        {
	 case kCM_LISTBOX:
           status = HandleListBox(parm1); break;
	 case kCM_CHECKBUTTON:
	   status = HandleCheckButton(parm1); break;
         case kCM_MENUSELECT:
               //printf("Pointer over menu entry, id=%ld\n", parm1);
             break;
         case kCM_MENU:
           status = HandleMenuBar(parm1); break;
	   default:
	     break;
	} // switch GET_SUBMSG(msg)
	 default:
	   break;
    }  // siwtch GET_MSG(msg)
return kTRUE; 
}

Int_t TaPanam::HandleCheckButton(Long_t parm1)
{
 if (!fPrestart && !fStart && !fStop)
   {
  switch(parm1)
    {
      // Time folder
    case 10: UpdateDevListWithButton(fTimeCheck[0],string("quadsynch"),1);  break;
    case 11: UpdateDevListWithButton(fTimeCheck[1],string("pairsynch"),1); break;
    case 12: UpdateDevListWithButton(fTimeCheck[2],string("helicity"),1); break;
    case 13: UpdateDevListWithButton(fTimeCheck[3],string("timeslot"),1);  break;
    case 14: UpdateDevListWithButton(fTimeCheck[4],string("v2fclock"),1);  break;
    case 15: DisplayThis(fTOptCheck[0],fShowTime); break;
    case 16: CheckChoice2(fTOptCheck[1],fTOptCheck[2],fTSCorH); break;
    case 17: CheckChoice2(fTOptCheck[1],fTOptCheck[2],fTSCorH); break;
      // ADC folder
      //    case 20: UpdateADCListWithButton(fIADCCheck[0],string("adc11"));  break;
      //    case 21: UpdateADCListWithButton(fIADCCheck[0],string("adc12"));  break;
    case 22: UpdateADCListWithButton(fADCCheck[0],string("adc0"));  break;
    case 23: UpdateADCListWithButton(fADCCheck[1],string("adc1"));  break;
    case 24: UpdateADCListWithButton(fADCCheck[2],string("adc2"));  break;
    case 25: UpdateADCListWithButton(fADCCheck[3],string("adc3"));  break;
    case 26: UpdateADCListWithButton(fADCCheck[4],string("adc4"));  break;
    case 27: UpdateADCListWithButton(fADCCheck[5],string("adc5"));  break;
    case 28: UpdateADCListWithButton(fADCCheck[6],string("adc6"));  break;
    case 29: UpdateADCListWithButton(fADCCheck[7],string("adc7"));  break;
    case 30: UpdateADCListWithButton(fADCCheck[8],string("adc8"));  break;
    case 31: UpdateADCListWithButton(fADCCheck[9],string("adc9"));  break;
      //    case 32: UpdateADCListWithButton(fSADCCheck[0],string("adc0"));  break;
      //    case 33: UpdateADCListWithButton(fSADCCheck[0],string("adc0"));  break;
    case 34: DisplayThis(fADCOptCheck[0],fShowIADC); break;
    case 35: DisplayThis(fADCOptCheck[1],fShowPADC); break;
    case 36: DisplayThis(fADCOptCheck[2],fShowSADC); break;
    case 37: DisplayThis(fADCOptCheck[3],fADCdata); break;
    case 38: DisplayThis(fADCOptCheck[4],fADCdata); break;
    case 39:  CheckChoice2(fADCOptCheck[5],fADCOptCheck[6],fADCSCorH);  break;
    case 40:  CheckChoice2(fADCOptCheck[5],fADCOptCheck[6],fADCSCorH);  break;

      // BCM folder
      //    case 41:  UpdateDevListWithButton(fInjBPMWSCheck[0],string("bpmin1ws"),2); break;
      //    case 42:  UpdateDevListWithButton(fInjBPMWSCheck[1],string("bpmin2ws"),2); break;
    case 43:  UpdateDevListWithButton(fBCMCheck[0],string("bcm1"),2); break;
    case 44:  UpdateDevListWithButton(fBCMCheck[1],string("bcm2"),2); break;
    case 45:  UpdateDevListWithButton(fBCMCheck[2],string("bcm3"),2); break;
      //    case 50:  UpdateDevListWithButton(fBCMCavCheck[0],string("bcmcav1"),2); break;
      //    case 51:  UpdateDevListWithButton(fBCMCavCheck[1],string("bcmcav2"),2); break;
    case 52:  DisplayThis(fBCMDCheck[0],fShowBCM);   break;
    case 53:  DisplayThis(fBCMDCheck[1],fShowBCMCAV); break;
    case 54:  CheckChoice3(fBCMDispCheck[0],fBCMDispCheck[1],fBCMDispCheck[2],fBCMplot);break;
    case 55:  CheckChoice3(fBCMDispCheck[0],fBCMDispCheck[1],fBCMDispCheck[2],fBCMplot);break;
    case 56:  CheckChoice3(fBCMDispCheck[0],fBCMDispCheck[1],fBCMDispCheck[2],fBCMplot);break;
    case 57:  CheckChoice2(fBCMOptCheck[0],fBCMOptCheck[1],fBCMSCorH); break;
    case 58:  CheckChoice2(fBCMOptCheck[0],fBCMOptCheck[1],fBCMSCorH); break;
      // BPM folder
    case 70:  UpdateBPMListWithButton(fInjBPMCheck[0],string("bpmin1"),2); break;
    case 71:  UpdateBPMListWithButton(fInjBPMCheck[1],string("bpmin2"),2); break;
    case 72:  UpdateBPMListWithButton(fBPMCheck[0],string("bpm12"),2); break;
    case 73:  UpdateBPMListWithButton(fBPMCheck[1],string("bpm10"),2); break;
    case 74:  UpdateBPMListWithButton(fBPMCheck[2],string("bpm8"),2); break;
    case 75:  UpdateBPMListWithButton(fBPMCheck[3],string("bpm4a"),2); break;
    case 76:  UpdateBPMListWithButton(fBPMCheck[4],string("bpm4b"),2); break;
    case 77:  break;
      //    case 78:  UpdateBPMListWithButton(fBPMCavCheck[0],string("bpmcav1"),2); break;
      //    case 79:  UpdateBPMListWithButton(fBPMCavCheck[1],string("bpmcav2"),2); break;
    case 80:  DisplayThis(fBPMDCheck[0],fShowINBPM); break;
    case 81:  DisplayThis(fBPMDCheck[1],fShowBPM); break;
    case 82:  DisplayThis(fBPMDCheck[2],fShowBPMCAV); break;
    case 83:  CheckChoice3(fBPMDispCheck[0],fBPMDispCheck[1],fBPMDispCheck[2],fBPMplot); break;
    case 84:  CheckChoice3(fBPMDispCheck[0],fBPMDispCheck[1],fBPMDispCheck[2],fBPMplot); break;
    case 85:  CheckChoice3(fBPMDispCheck[0],fBPMDispCheck[1],fBPMDispCheck[2],fBPMplot); break;
    case 86:  CheckChoice2(fBPMOptCheck[0],fBPMOptCheck[1],fBPMSCorH);break;
    case 87:  CheckChoice2(fBPMOptCheck[0],fBPMOptCheck[1],fBPMSCorH);break;
    case 100:  DisplayThis(fFCurCheck[0],fShowFDBK);DisplayThis(fFCurCheck[0],fPITA);break;
    case 101:  DisplayThis(fFCurCheck[1],fShowFDBK);DisplayThis(fFCurCheck[1],fIA);break;
    case 102:  DisplayThis(fFPosCheck,fShowFDBK);DisplayThis(fFPosCheck,fShowBPM);break;
    case 103:  DisplayThis(fDITHCheck[0],fShowDITH);break;
    case 104:  DisplayThis(fDITHCheck[1],fShowDITH);break;
    case 110:  UpdateDevListWithButton(fDalCheck[0],string("det1"),2);break;
    case 111:  UpdateDevListWithButton(fDalCheck[1],string("det2"),2);break;
    case 112:  UpdateDevListWithButton(fDalCheck[2],string("det3"),2);break;
    case 113:  UpdateDevListWithButton(fDalCheck[3],string("det4"),2);break;
    case 114:  UpdateDevListWithButton(fLumiCheck[0],string("lumi1"),2);break;
    case 115:  UpdateDevListWithButton(fLumiCheck[1],string("lumi2"),2);break;
    case 116:  UpdateDevListWithButton(fLumiCheck[2],string("lumi3"),2);break;
    case 117:  UpdateDevListWithButton(fLumiCheck[3],string("lumi4"),2);break;
    }
  }
  return 1;
}

Int_t TaPanam::HandleMenuBar(Long_t parm1)
{
  switch(parm1)
    {
     case M_FILE_EXIT:
       CloseWindow(); break; // exit Paname

     case M_MON_PRESTART:
       cout<<" ###  PRESTART ### "<<endl; 
        TThread::Kill("refreshThread"); 
        TThread::Kill("PanThread");
        RefreshThreadStop(); 
        PanThreadStop();             
        //DumpOptionState();
        fPrestart=kTRUE;
        fEnd=kFALSE; 
        if (ResizeDevList(1)) cout<<" ResizeDevList(1):  --- resizing device  list --- "<<endl;  
        if (ResizeDevList(2)) cout<<" ResizeDevList(2):  --- resizing Adevice list --- "<<endl;  
        //cout<<"creating new analyzer"<<endl; 
        fAnaMan = new TaAnalysisManager();
        //cout<<"sending list to analyzer"<<endl;
        if (InitListOfAllPlots()) cout<<" InitListOfAllPlots() : Device list filled "<<endl;
        fAnaMan->SetMonitorList(fAllDeviceList); 
        if (CheckOptionConfig()) cout<<" CheckOptionConfig() : User Config Initialized "<<endl;
        //DumpOptionState();           
#ifdef CODAFILETEST 
        fAnaMan->Init(1504);
#endif
#ifdef ETCONNECT 
        fAnaMan->Init();
#endif
        //fAnaMan->GetAnalysis()->DefineADCStacks(fTSCorH);
        if (InitHistoListBox(fAnaMan->GetAnalysis()->GetHistoForListBox())) 
	  cout<<" InitHistoListBox() : List Box Filled \n";              
        if (CheckCanvasConfig()) cout<<" CheckCanvasConfig() : Canvas Config Initialized "<<endl;
        fPrestart=kTRUE;
        //if ( fPrestart) cout<<" prestart OK"<<endl;
        if (InitCanvasPads()) cout<<" InitCanvasPads() : Canvas Flock Initialized "<<endl;
        cout<<" PRESTART SUCCEEDED.......\n";
	fMenuMon->DisableEntry(M_MON_PRESTART);   // disable PRESTART
        fMenuMon->EnableEntry(M_MON_START); 
        fMenuMon->EnableEntry(M_MON_END);
        break;
     case M_MON_START:
	{
         cout<<" ### START ### "<<endl;
         fPrestart=kFALSE;
         fStart=kTRUE;
	 fMenuMon->DisableEntry(M_MON_START);   // disable START
	 fMenuMon->EnableEntry(M_MON_STOP);     // enable STOP
         if (!PanThreadStart())
	   { 
             cout<<" START SUCCEEDED..........\n ";
             RefreshThreadStart();
               
            }
         else cout<<" thread not launched....\n";
          
	}
	break;
      case M_MON_STOP:
	{ 
	  cout<<" ### STOP ###\n ";            
	  TThread::Kill("refreshThread"); 
	  TThread::Kill("PanThread");
          RefreshThreadStop(); 
          PanThreadStop();             
          cout<<" STOPPED..................\n";
          fStart=kFALSE;
          fStop=kTRUE;
	  fMenuMon->DisableEntry(M_MON_STOP);     // disable STOP
	  fMenuMon->DisableEntry(M_MON_START);   // disable START
	  fMenuMon->EnableEntry(M_MON_PRESTART);   // enable PRESTART
        } 
	break;
      case M_MON_END:
        { 
          cout<<" ### END ### \n";
	  if (ClearCanvas())  cout<<" ClearCanvas()  : Canvas flock cleared"<<endl;
	  if (ClearListBox()) cout<<" ClearListBox() : ListBox Emptied"<<endl;
          if (ClearListOfDevice()) cout<<" ClearListofDevice() : Device lists cleared"<<endl;
          if (InitFlags())        cout<<" InitFlags() : Flags Reinitialized"<<endl;
	  if (fPrestart)
	    {
             //if still prestart fAnaMan is at Init() step
	      fAnaMan->End();
              delete fAnaMan; 
              fAnaMan = NULL;
	    }
          else if (fStop)
	    {
	      delete fAnaMan; 
              fAnaMan = NULL;
	    }
	 if (InitParameters()) cout<<" InitParameters() : Parameters Initialized "<<endl;
         if (CheckDevListConfig()) cout<<" CheckDevListConfig() : Check user device config "<<endl;
         fPrestart=kFALSE;
         fStart=kFALSE;
         fStop=kFALSE;
         fEnd=kTRUE;
         fMenuMon->DisableEntry(M_MON_START);    // disable START
         fMenuMon->DisableEntry(M_MON_STOP);     // disable STOP
	 fMenuMon->EnableEntry(M_MON_PRESTART);     // enable PRESTART
         cout<<" ENDED................................................................... "<<endl; 
        }
        break;
      default:
	break;
    }
  return 1;
}

Int_t TaPanam::HandleListBox(Long_t parm1)
{
//  cout<<" kCM_LISTBOX called "<<endl;
// #ifdef LISTBOX
//  Int_t id;
//  id=fHistoListBox->GetSelected();
//  cout<<" id="<<id<<endl;
//  TGTextLBEntry* tentry;
//  tentry = (TGTextLBEntry*) (fHistoListBox->GetSelectedEntry());
//  cout<<" text of selected entry="<<tentry->GetText()->GetString()<<endl;
//  fEmCanDevPlotName = (Char_t*) tentry->GetText()->GetString();
//  Char_t* plstr; 
//  plstr =  (Char_t*) tentry->GetText()->GetString();
//  fEmCanDevName= strtok (plstr,"_");
//  //cout<<"fEmCanDevName="<<fEmCanDevName<<endl;
//  fEmCanDevName = strtok (NULL,"_");
//  if (!strncmp((Char_t*) tentry->GetText()->GetString(),"S",1)) fEmCanDevPlotType=1;
//  if (!strncmp((Char_t*) tentry->GetText()->GetString(),"H",1)) fEmCanDevPlotType=2;
//  cout<<"fEmCanDevName="<<fEmCanDevName<<endl; 
//  cout<<"fEmCanDevPlotName="<<fEmCanDevPlotName<<endl;   
//  cout<<"fEmCanDevPlotType="<<fEmCanDevPlotType<<endl;
//  fHistoListBox->AddEntrySort(fEmCanDevPlotName,id);
//  MapSubwindows();
//  Resize(GetDefaultSize()); 
//  // reloader l'entry dans la liste box.....a cause des problemes....d'affichages....
//  delete plstr;   
// #ifdef EMCANON
//   UpdateEmCanvas();
// #endif
// #endif
  return 1;
}

Int_t TaPanam::InitParameters(){

  fTimeNum =0; fPADCNum  = 0; 
  fIADCNum  = 0; fSADCNum  = 0;
  fBCMNum  = 0; fBCMCAVNum = 0;
  fBPMNum  = 0; fBPMINNum = 0; fBPMCAVNum = 0; 
  fLUMNum = 0; fDETNum  = 0;
  fTimeidx.clear();fIADCidx.clear();fPADCidx.clear();fSADCidx.clear();
  fBCMidx.clear();fBCMCAVidx.clear();fBPMidx.clear();fBPMINidx.clear();
  fBPMCAVidx.clear();fLUMIidx.clear();fDETidx.clear();
  
  fDeviceList.clear();fADeviceList.clear();fAllDeviceList.clear();
  fADCdata = kTRUE;

  return 1;
}

string 
TaPanam::itos(Int_t i)
{
  ostringstream buffer;
  buffer<<i;
  return buffer.str();
}

Int_t  
TaPanam::InitCanvasFlock()
{
 CountDataInList();
 return 1;
}

Int_t TaPanam::IsDeviceName(string devname, Int_t devtype) const
{// devtype =1 is TaPanamDevice, devtype =2 is TaPanamADevice  

  switch (devtype)
    {
    case 1:
       for (vector<string>::const_iterator i= fDeviceList.begin(); i!= fDeviceList.end(); i++)
         {
          if (*i == devname) 
            {
             cout<<"found same name :"<<devname<<endl;
             return 1;
            }
          }
       break;
    case 2:
       for (vector<string>::const_iterator i= fADeviceList.begin(); i!= fADeviceList.end(); i++)
         {
          if (*i == devname) 
            {
             cout<<"found same name :"<<devname<<endl;
             return 1;
            }
          }
       break;
    }
  return 0;
}

void TaPanam::AddDeviceName(string devname, Int_t devtype)
{ 
  switch (devtype)
    {
    case 1:
     fDeviceList.push_back(devname);
     cout<<" Added "<<devname<<" in the device list"<<endl;
     break;
    case 2:
     fADeviceList.push_back(devname);
     cout<<" Added "<<devname<<" in the Adevice list"<<endl;
     break;
    }
}

void TaPanam::RemoveDeviceName(string devname, Int_t devtype)
{ 
  switch (devtype)
    {
    case 1:
      for (UInt_t i=0;i<fDeviceList.size();i++)
        {
         if (fDeviceList[i] == devname) {
             fDeviceList[i]="-";
             cout<<devname<<"erased from list"<<endl;
          }
	}
	break;
    case 2:
      for (UInt_t i=0;i<fADeviceList.size();i++)
        {
         if (fADeviceList[i] == devname) {
             fADeviceList[i]="-";
             cout<<devname<<"erased from list"<<endl;
           }
	}
       break;
    }
//  for (vector<string>::iterator i= fDeviceList.begin(); i!= fDeviceList.end(); i++)
//    {
//      idx++;
//     if (*i == devname) {
//       stridx=idx; 
//       cout<<"TaPanam::RemoveDeviceName() found already def device"<<endl;
//       fDeviceList[stridx] = "";
//     }
//   }
}

Int_t TaPanam::ResizeDevList(Int_t devtype)
{
  Int_t count =0;
  //DumpDevList(devtype);
 if (devtype == 1)
   {
     //cout<<"before devlist SIZE="<<fDeviceList.size()<<endl;
    for (vector<string>::iterator i= fDeviceList.begin(); i!= fDeviceList.end(); i++)
       {
        if ((*i) == "-") 
          {
	   count++;
           fDeviceList.erase(i);
          }
        }
    //cout<<"after devlist SIZE="<<fDeviceList.size()<<" count of spaces="<<count<<endl; 
   }
 else if (devtype == 2)
   {
     //cout<<"before devlist SIZE="<<fADeviceList.size()<<endl;
    for (vector<string>::iterator i= fADeviceList.begin(); i!= fADeviceList.end(); i++)
       {
        if ((*i) == "-") 
          {
	   count++;
           fADeviceList.erase(i);
          }
        }
    //cout<<"after devlist SIZE="<<fADeviceList.size()<<" count of spaces="<<count<<endl; 
   }
 //DumpDevList(devtype);
 return 1;
}

void TaPanam::UpdateDevListWithButton(TGCheckButton *thebutton, string devname, Int_t devtype)
{
 if(thebutton->IsDown())
   {
     if (thebutton->GetState() == kButtonUp)
       {
        cout<<"result of already def device name"<< devname<<"="<<IsDeviceName(devname,devtype)<<endl;
        if(IsDeviceName(devname,devtype)) { cout<<"already def "<<endl;RemoveDeviceName(devname,devtype);
          }
       }
     else{ if(!IsDeviceName(devname, devtype)) AddDeviceName(devname, devtype);       
       }
   }
 DumpDevList(devtype);
}


void TaPanam::UpdateADCListWithButton(TGCheckButton *thebutton, string devname)
{
 string  dataname; 
 if(thebutton->IsDown())
   {
    if (thebutton->GetState() ==  kButtonUp)
      {
       for (Int_t j=0; j<maxchanadc; j++)
         {
	   if (!fADCdata)
	     {
              dataname = (string(devname)+ string("_") + itos(j)).c_str();
              if (IsDeviceName(dataname,2)) RemoveDeviceName(dataname,2);
	     }
	   else
	     {
              dataname = (string(devname)+ string("_") + itos(j) + string("_cal")).c_str();
              if (IsDeviceName(dataname,2)) RemoveDeviceName(dataname,2);
	     }      
         }
      }
     else
      {
       for (Int_t j=0; j<maxchanadc; j++)
         {
	   if (!fADCdata)
	     {
              dataname = (string(devname)+ string("_") + itos(j)).c_str();
              if(!IsDeviceName(dataname,2)) AddDeviceName(dataname,2);
	     }
           else
	     { 
              dataname = (string(devname)+ string("_") + itos(j) + string("_cal")).c_str();
              if(!IsDeviceName(dataname,2)) AddDeviceName(dataname,2);
	     } 
         }      
      }
   }
 DumpDevList(2);
}

void TaPanam::UpdateBPMListWithButton(TGCheckButton *thebutton, string devname, Int_t devtype)
{
 string  dataname; 
 if(thebutton->IsDown())
   {
    if (thebutton->GetState() ==  kButtonUp)
      {
       dataname = (string(devname) + string("x")).c_str();
       if(!IsDeviceName(dataname,2)) RemoveDeviceName(dataname,2);
       dataname = (string(devname)+ string("y")).c_str();
       if(!IsDeviceName(dataname,2)) RemoveDeviceName(dataname,2);
      }
    else 
      {
       dataname = (string(devname)+ string("x")).c_str();
       if(!IsDeviceName(dataname,2)) AddDeviceName(dataname,2);
       dataname = (string(devname)+ string("y")).c_str();
       if(!IsDeviceName(dataname,2)) AddDeviceName(dataname,2);      
      }
 }  
}

void TaPanam::DumpDevList(Int_t devtype)
{
  switch (devtype)
    {
    case 1:
      cout<<" ++ Dump device name list ++"<<endl;        
      for (vector<string>::const_iterator i= fDeviceList.begin(); i!= fDeviceList.end(); i++)
         cout<<*i<<endl;
      break;
    case 2:
      cout<<" ++ Dump Adevice name list ++"<<endl;        
      for (vector<string>::const_iterator i= fADeviceList.begin(); i!= fADeviceList.end(); i++)
         cout<<*i<<endl;
      break;
    }
}

void TaPanam::CheckChoice2(TGCheckButton *thebutton1,TGCheckButton *thebutton2, Bool_t flag)
{
  if (thebutton1->IsDown()) 
    {
      if (thebutton1->GetState() == kButtonDown) 
        {
         flag = kFALSE;
         //cout<<"CheckChoice2() : Set to FALSE"<<endl; 
         if (thebutton2->GetState() == kButtonDown) thebutton2->SetState(kButtonUp);
	}
    }
  if (thebutton2->IsDown()) 
    {
      if (thebutton2->GetState() == kButtonDown) 
        {
         flag = kTRUE;
         //cout<<"CheckChoice2() : Set to TRUE"<<endl; 
         if (thebutton1->GetState() == kButtonDown) thebutton1->SetState(kButtonUp);
	}
    }     
}

void TaPanam::CheckChoice3(TGCheckButton *thebutton1,TGCheckButton *thebutton2,TGCheckButton *thebutton3,Int_t flag)
{
  if (thebutton1->IsDown()) 
    {
      if (thebutton1->GetState() == kButtonDown) 
        {
         flag = 1;
         //cout<<"CheckChoice3() : Set to 1 "<<endl; 
         if (thebutton2->GetState() == kButtonDown || thebutton3->GetState() == kButtonDown) 
           {
            thebutton2->SetState(kButtonUp);
            thebutton3->SetState(kButtonUp);
	   }
	}
    }
    {
      if (thebutton2->GetState() == kButtonDown) 
        {
         flag = 3;
         //cout<<"CheckChoice3() : Set to 3 "<<endl; 
         if (thebutton1->GetState() == kButtonDown || thebutton3->GetState() == kButtonDown) 
           {
            thebutton1->SetState(kButtonUp);
            thebutton3->SetState(kButtonUp);
	   }
	}
    }
    {
      if (thebutton3->GetState() == kButtonDown) 
        {
         flag = 2;
         //cout<<"CheckChoice3() : Set to 2 "<<endl; 
         if (thebutton1->GetState() == kButtonDown || thebutton2->GetState() == kButtonDown) 
           {
            thebutton1->SetState(kButtonUp);
            thebutton2->SetState(kButtonUp);
	   }
	}
    }
}

void TaPanam::DisplayThis(TGCheckButton *thebutton, Bool_t showcan)
{
  if (thebutton->IsDown()) 
    {
      if (thebutton->GetState() == kButtonDown) showcan = kTRUE;
      else showcan = kFALSE; 
    }
}

Int_t TaPanam::InitHistoListBox(vector<string> thelist)
{// display the list of "sub" histos and stripcharts contained
 // in each TaPAnamDevice object in the List box of the main GUI frame. 
  Int_t k=1;
  if (ClearListBox() && thelist.size() !=0)
    {
     for ( vector<string>::const_iterator i=thelist.begin(); i!=thelist.end();i++)
        { 
	  //	  cout<<" add entry to list box string="<<*i<<" char*="<<(*i).c_str()<<"with number :"<<k<<endl;
         fHistoListBox->AddEntry((char*)(*i).c_str(),k);
         k++;
        }
     MapSubwindows();
     fHistoListBox->Layout();
     Resize(1000,500);
     MapWindow();
     return 1;
    }
  else 
    {
     cout<<"TaPanam::InitHistoListBox() nothing initialized, vector size null \n";
     return 0;
    }
  return 0;
}

Int_t TaPanam::InitListOfAllPlots()
{
 for (vector<string>::const_iterator i= fDeviceList.begin(); i!= fDeviceList.end(); i++) 
   fAllDeviceList.push_back(*i);
 for (vector<string>::const_iterator i= fADeviceList.begin(); i!= fADeviceList.end(); i++) 
   fAllDeviceList.push_back(*i);
 cout<<" = = =  Dump ALL DEVICE LIST = = = "<<endl;        
 for (vector<string>::const_iterator i= fAllDeviceList.begin(); i!= fAllDeviceList.end(); i++)
    cout<<*i<<endl;
 return 1; 
}




void TaPanam::CountDataInList()
{
  ResetIdxVector();
  for (UInt_t i = 0; i< fDeviceList.size(); i++)
    {
     if (!strncmp((fDeviceList[i]).c_str(),"hel",3) || !strncmp((fDeviceList[i]).c_str(),"pair",4) || 
         !strncmp((fDeviceList[i]).c_str(),"quad",4) ||  !strncmp((fDeviceList[i]).c_str(),"time",4) ||
         !strncmp((fDeviceList[i]).c_str(),"v2f",3) ) 
      {
       fTimeNum++;
       fTimeidx.push_back(i);     
      }
    }
  for (UInt_t i = 0; i< fADeviceList.size(); i++)
    {
     if (!strncmp((fADeviceList[i]).c_str(),"adc",3)) 
      {
       fPADCNum++;
       fPADCidx.push_back(i);     
      }
     if (!strncmp((fADeviceList[i]).c_str(),"bcmcav",6)) 
       { 
        fBCMCAVNum++;
        fBCMCAVidx.push_back(i);
       }
     else if (!strncmp((fADeviceList[i]).c_str(),"bcm",3)) 
       { 
        fBCMNum++;
        fBCMidx.push_back(i);
       }
//      if (!strncmp((fADeviceList[i]).c_str(),"bat",3)) 
//         { 
//          fBCMNum++;
//          fBCMidx.push_back(i);
//         }     
     if (!strncmp((fADeviceList[i]).c_str(),"bpmin",5)) 
       {
        fBPMINNum++;
        fBPMINidx.push_back(i); 
       }
     else if (!strncmp((fADeviceList[i]).c_str(),"bpmws",5))
       {
        fBCMNum++;
        fBCMidx.push_back(i);          
       }
     else if (!strncmp((fADeviceList[i]).c_str(),"bpm",3))
       {
        fBPMNum++;
        fBPMidx.push_back(i);         
       }
     if (!strncmp((fADeviceList[i]).c_str(),"lum",3)) 
       {
        fLUMNum++;
	fLUMIidx.push_back(i);    
       }
     if (!strncmp((fADeviceList[i]).c_str(),"det",3)) 
       {
        fDETNum++;
	fDETidx.push_back(i);    
       }
    }

#ifdef DEBUGDCOUNT
  cout<<" fTimeNum="<<fTimeNum<<" fIADCNum="<<fIADCNum<<endl;
  cout<<" fPADCNum="<<fPADCNum<<" fSADCNum="<<fSADCNum<<endl;
  cout<<" fBCMNum="<<fBCMNum<<" fBCMCAVNum="<<fBCMCAVNum<<endl;
  cout<<" fBPMINNum="<<fBPMINNum<<" fBPMNum="<<fBPMNum<<endl;
  cout<<" fBPMCAVNum="<<fBPMCAVNum<<endl;
  cout<<" fLUMNum="<<fLUMNum<<" fDETNum="<<fDETNum<<endl;

  cout<<"Time idx size ="<<fTimeidx.size()<<endl;  
  cout<<"IADC idx size ="<<fIADCidx.size()<<endl;
  cout<<"PADC idx size ="<<fPADCidx.size()<<endl;
  cout<<"SADC idx size ="<<fSADCidx.size()<<endl;
  cout<<"BCM  idx size ="<<fBCMidx.size()<<endl;
  cout<<"BPMIN idxsize ="<<fBPMidx.size()<<endl;
  cout<<"BPM idx size ="<<fBPMINidx.size()<<endl;
  cout<<"BCMCAV idx size ="<<fBCMCAVidx.size()<<endl;
  cout<<"BPMCAV idx size ="<<fBPMCAVidx.size()<<endl;
  cout<<"LUMI idx   vector  size ="<<fLUMIidx.size()<<endl;
  cout<<"DET idx   vector  size ="<<fDETidx.size()<<endl;

#endif

}
#ifdef DEBUGOPTION
void TaPanam::DumpOptionState()
{
  if (fFirstAna) cout <<"fFirstAna = T "; else cout<<"fFirstAna = F \n\n";

  if (fShowTime) cout <<"fShowTime = T "; else cout<<"fShowTime = F ";
  if (fShowIADC) cout <<" fShowIADC = T ";else cout<<" fShowIADC = F";
  if (fShowPADC) cout <<" fShowPADC = T ";else cout<<" fShowPADC = F";
  if (fShowSADC) cout <<" fShowSADC = T ";else cout<<" fShowSADC = F ";
  if (fShowBCM) cout <<" fShowBCM = T ";else cout<<" fShowBCM = F ";
  if (fShowBCMCAV) cout <<" fShowBCMCAV = T \n\n";else cout<<" fShowBCMCAV = F \n\n";

  if (fShowINBPM) cout <<"fShowINBPM = T ";else cout<<"fShowINBPM = F ";
  if (fShowBPM) cout <<" fShowBPM = T ";else cout<<" fShowBPM = F ";
  if (fShowBPMCAV) cout <<" fShowBPMCAV = T ";else cout<<" fShowBPMCAV = F ";
  if (fShowFDBK) cout <<" fShowFDBK = T ";else cout<<" fShowFDBK = F ";
  if (fShowDITH) cout <<" fShowDITH = T ";else cout<<" fShowDITH = F ";
  if (fShowLUMI) cout <<" fShowLUMI = T ";else cout<<" fShowLUMI = F ";
  if (fShowDET) cout <<" fShowDET = T \n\n";else cout<<" fShowDET = F \n\n";

  if (fTSCorH) cout <<" fTSCorH = T ";else cout<<"fTSCorH = F ";
  if (fADCSCorH) cout <<" fADCSCorH = T ";else cout<<" fADCSCorH = F ";
  if (fBCMSCorH) cout <<" fBCMSCorH = T ";else cout<<" fBCMSCorH = F ";
  if (fBPMSCorH) cout <<" fBPMSCorH = T \n\n";else cout<<" fBPMSCorH = F \n\n";

  if (fADCdata) cout <<" fADCdata = T ";else cout<<" fADCdata = F ";
  if (fPITA) cout <<" fPITA = T ";else cout<<" fPITA = F ";
  if (fIA) cout <<" fIA = T ";else cout<<" fIA = F ";
  if (fPZT) cout <<" fPZT = T \n\n";else cout<<" fPZT = F \n\n";

  cout <<" fBCMplot = "<<fBCMplot;
  cout <<" fBPMplot = "<<fBPMplot<<" \n \n ";

  if (fPrestart) cout <<" fPrestart = T ";else cout<<" fPrestart = F ";
  if (fStart) cout    <<" fStart = T ";else cout<<" fStart = F ";
  if (fStop) cout     <<" fStop = T ";else cout<<" fStop = F ";
  if (fEnd) cout      <<" fEnd = T \n\n";else cout<<" fEnd = F \n\n";   

}
#endif

void TaPanam::CanvasSafeDelete(TCanvas* can, char* name )
{
  TObject* obj = (gROOT->GetListOfCanvases()->FindObject(name));
  if (obj == (TObject*) can)
  if (can) delete can;
  can = NULL;
}


Int_t TaPanam::UpdateCanvasFlock()
{
 if (fTimeNum)
   {
    fTimeCan =  new TCanvas("TimeSignal", " HAPPEX Time signals ",1,1,300,300);
    if (!fShowTime)  {
         fTimeCan->Iconify();
	 cout<<" Iconyfying Time canvas "<<endl;
    }
   } 
 else if (!fTimeNum && gROOT->GetListOfCanvases()->FindObject("TimeSignal"))
   {
     cout<<"already defined canvas TimeSignal deleting"<<endl;
     CanvasSafeDelete(fTimeCan,"TimeSignal");
   }
 else cout<<"No canvas TimeSignal created"<<endl;

 if (fIADCNum)
   {
    fIADCCan =  new TCanvas("IADC", "Parity DAQ ADCs",1,1,800,1200);
    if (!fShowIADC)  {
     fIADCCan->Iconify();
     cout<<" Iconyfying ADC canvas "<<endl;
    }
   } 
 else if (!fIADCNum && gROOT->GetListOfCanvases()->FindObject("IADC"))
   {
     cout<<"already defined canvas IADC deleting"<<endl;
     CanvasSafeDelete(fIADCCan,"IADC");
   }
 else cout<<"No canvas IADC created"<<endl;
 if (fPADCNum)
   {
    fPADCCan =  new TCanvas("PADC", "Parity DAQ ADCs",1,5,800,1200);
    if (!fShowPADC)  fPADCCan->Iconify();
   } 
 else if (!fPADCNum && gROOT->GetListOfCanvases()->FindObject("PADC"))
   {
     cout<<"already defined canvas PADC deleting"<<endl;
     CanvasSafeDelete(fPADCCan,"IADC");
   }
 else cout<<"No canvas PADC created"<<endl;
 if (fSADCNum)
   {
    fSADCCan =  new TCanvas("SADC", "Parity DAQ ADCs",1,1,100,100);
    if (!fShowSADC)  fSADCCan->Iconify();
   } 
 else if (!fSADCNum && gROOT->GetListOfCanvases()->FindObject("SADC"))
   {
     cout<<"already defined canvas SADC deleting"<<endl;
     CanvasSafeDelete(fSADCCan,"SADC");
   }
 else cout<<"No canvas SADC created"<<endl;

 if (fBCMNum)
   {
    fBCMCan =  new TCanvas("BCM", "Injector/HAll A Beam Current",620,1,600,600);
    if (!fShowBCM)  fBCMCan->Iconify();
   } 
 else if (!fBCMNum && gROOT->GetListOfCanvases()->FindObject("BCM"))
   {
     cout<<"already defined canvas BCM deleting"<<endl;
     CanvasSafeDelete(fBCMCan,"BCM");
   }
 else cout<<"No canvas BCM created"<<endl;

 if (fBCMCAVNum)
   {
    fBCMCAVCan =  new TCanvas("BCMCAV", "Beam Current Cavity monitors",1,10,600,600);
    if (!fShowBCMCAV)  fBCMCAVCan->Iconify();
   } 
 else if (!fBCMCAVNum && gROOT->GetListOfCanvases()->FindObject("BCMCAV"))
   {
     cout<<"already defined canvas BCMCAV deleting"<<endl;
     CanvasSafeDelete(fBCMCAVCan,"BCMCAV");
   }
 else cout<<"No canvas BCMCAV created"<<endl;

 if (fBPMINNum)
   {
    fBPMINCan =  new TCanvas("BPMIN", "Injector BPMs ",1,1,100,100);
    if (!fShowINBPM)  fBPMINCan->Iconify();
   } 
 else if (!fBPMINNum && gROOT->GetListOfCanvases()->FindObject("BPMIN"))
   {
     cout<<"already defined canvas BPMIN deleting"<<endl;
     CanvasSafeDelete(fBPMINCan,"BPMIN");
   }
 else cout<<"No canvas BPMIN created"<<endl;

 if (fBPMNum)
   {
    fBPMCan =  new TCanvas("BPM", "HALL A Stripline BPMs",1,20,800,1200);
    if (!fShowBPM)  fBPMCan->Iconify();
   } 
 else if (!fBPMNum && gROOT->GetListOfCanvases()->FindObject("BPM"))
   {
     cout<<"already defined canvas BPM deleting"<<endl;
     CanvasSafeDelete(fBPMCan,"BPM");
   }
 else cout<<"No canvas BPM created"<<endl;

 if (fBPMCAVNum)
   {
    fBPMCAVCan =  new TCanvas("BPMCAV", "HALL A BPM cavities",1,1,100,100);
    if (!fShowBPMCAV)  fBPMCAVCan->Iconify();
   } 
 else if (!fBPMCAVNum && gROOT->GetListOfCanvases()->FindObject("BPMCAV"))
   {
     cout<<"already defined canvas BPMCAV deleting"<<endl;
     CanvasSafeDelete(fBPMCAVCan,"BPMCAV");
   }
 else cout<<"No canvas BPMCAV created"<<endl;

 if (fShowFDBK) fFDBKCan =  new TCanvas("FDBK", "Feedback control",1,1,100,100);
 else if (!fShowFDBK && gROOT->GetListOfCanvases()->FindObject("FDBK"))
   {
     cout<<"already defined canvas FDBK deleting"<<endl;
     CanvasSafeDelete(fFDBKCan,"FDBK");
   }
 else cout<<"No canvas FDBK created"<<endl;

 if (fShowDITH) fDITHCan =  new TCanvas("DITH", "Beam modulation",1,1,100,100);
 else if (!fShowDITH && gROOT->GetListOfCanvases()->FindObject("DITH"))
   {
     cout<<"already defined canvas DITH deleting"<<endl;
     CanvasSafeDelete(fDITHCan,"DITH");
   }
 else cout<<"No canvas DITH created"<<endl;

 if (fLUMNum)
   {
    fLUMICan =  new TCanvas("LUMI", "Luminosity detector",1,1,800,600);
    if (!fShowLUMI)  fLUMICan->Iconify();
   } 
 else if (!fLUMNum && gROOT->GetListOfCanvases()->FindObject("LUMI"))
   {
     cout<<"already defined canvas LUMI deleting"<<endl;
     CanvasSafeDelete(fLUMICan,"LUMI");
   }
 else cout<<"No canvas LUMI created"<<endl;

 if (fDETNum)
   {
    fDETCan =  new TCanvas("DET", "The Daltons",1,1,800,600);
    if (!fShowDET)  fDETCan->Iconify();
   } 
 else if (!fDETNum && gROOT->GetListOfCanvases()->FindObject("DET"))
   {
     cout<<"already defined canvas DET deleting"<<endl;
     CanvasSafeDelete(fDETCan,"DET");
   }
 else cout<<"No canvas DET created"<<endl;
 return 1;
}

void TaPanam::ResetIdxVector()
{
  fTimeidx.clear();fIADCidx.clear();
  fPADCidx.clear();fSADCidx.clear();
  fBCMidx.clear();fBCMCAVidx.clear();
  fBPMidx.clear();fBPMINidx.clear();
  fBPMCAVidx.clear();fLUMIidx.clear();
  fDETidx.clear();
}

void TaPanam::UpdateCanvas(Char_t * title, vector<Int_t> idxvect, Int_t devtype, Bool_t disptype)
{
  TCanvas* currentcan;
  if ((gROOT->GetListOfCanvases()->FindObject(title))!=NULL)
    { 
      cout<<"found canvas named="<<title<<flush<<endl;
      currentcan = (TCanvas*)(gROOT->GetListOfCanvases()->FindObject(title));
      if (fPrestart == kTRUE)
	{
          cout<<"enter prestart condition..."<<endl;
	  switch (devtype)
             {
	     case 0:
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->Divide((UInt_t(idxvect.size()))); 
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->InitDevicePad(idxvect[idx],0,disptype);
	             }
		   }
		  else 
		   {
		    fHistoStyle->cd();
                    currentcan->Divide(idxvect.size()); 
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->InitDevicePad(idxvect[idx],0,disptype);
	             }
		   }               
	       break;
	    case 1:
	      cout<<" case 1 :DEV type device.... vector size="<<idxvect.size()<<flush<<endl; 
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->Divide((UInt_t(idxvect.size()*2))); 
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->InitDevicePad(idxvect[idx],0,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->InitDevicePad(idxvect[idx],1,disptype);
	             }
		   }
		  else 
		   {
		    fHistoStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->Divide(idxvect.size()); 
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->InitDevicePad(idxvect[idx],0,disptype);
	             }
		   }
	       break;
	   case 2: 
	     cout<<" case 2 : ADEV type device... vector size="<<idxvect.size()<<flush<<endl; 
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->Divide(4,idxvect.size()); 
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*4+1);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],0,disptype);
	              currentcan->cd(idx*4+2);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],1,disptype);
	              currentcan->cd(idx*4+3);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],2,disptype);
	              currentcan->cd(idx*4+4);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],3,disptype);
	             }
		   }
		  else 
		   {
		    fHistoStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->Divide(2,idxvect.size()); 
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],0,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],1,disptype);
	             }
		   }
	       break;
	  case 3: 
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->Divide(2,idxvect.size()); 
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],2,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],3,disptype);
	             }
		   }
		  else 
		   {
		    fHistoStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->Divide(idxvect.size()); 
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx+1);
                      cout<<"kiki"<<endl;
                      fAnaMan->GetAnalysis()->InitADevicePad(idxvect[idx],1,disptype);
	             }
		   }
	       break;
              
	    break;

	   } // switch
	}// prestart cond
      else if(fStart)
	{
	 switch (devtype)
            {
	    case 0:
              	 if (!disptype)
	           {
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
		      fStripStyle->cd();
                      currentcan->UseCurrentStyle();
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->DisplayDevice(idxvect[idx],0,disptype);
	             }
                    currentcan->Update();	             
		   }
		  else 
		   {
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
		      fHistoStyle->cd();
                      currentcan->UseCurrentStyle();
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->DisplayDevice(idxvect[idx],0,disptype);
	             }
                    currentcan->Update();	             
		   }             
	      break;
	    case 1:
              	 if (!disptype)
	           {
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
		      fStripStyle->cd();
                      currentcan->UseCurrentStyle();
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->DisplayDevice(idxvect[idx],0,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->DisplayDevice(idxvect[idx],1,disptype);
	             }
                    currentcan->Update();	             
		   }
		  else 
		   {
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
		      fHistoStyle->cd();
                      currentcan->UseCurrentStyle();
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->DisplayDevice(idxvect[idx],0,disptype);
	             }
                    currentcan->Update();	             
		   }
	       break;
	   case 2: 
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*4+1);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],0,disptype);
	              currentcan->cd(idx*4+2);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],1,disptype);
	              currentcan->cd(idx*4+3);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],2,disptype);
	              currentcan->cd(idx*4+4);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],3,disptype);
                      currentcan->Update();	             
	             }
		   }
		  else 
		   {
		    fHistoStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],0,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],1,disptype);
                      currentcan->Update();	             
                     }
		   }
	       break;
	  case 3: 
              	 if (!disptype)
	           {
		    fStripStyle->cd();
                    currentcan->UseCurrentStyle();
                    currentcan->cd();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx*2+1);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],2,disptype);
	              currentcan->cd(idx*2+2);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],3,disptype);
                      currentcan->Update();	             
	             }
		   }
		  else 
		   {
                    currentcan->cd();
		    fHistoStyle->cd();
                    currentcan->UseCurrentStyle();
                    for ( UInt_t idx = 0; idx < idxvect.size(); idx++)
	             {
	              currentcan->cd(idx+1);
                      fAnaMan->GetAnalysis()->DisplayADevice(idxvect[idx],1,disptype);
                      currentcan->Update();	             
                     }
		   }
	    break;
	     } // switch
	  } // start
      else if (!fPrestart) cout <<" Not prestart..."<<endl;
      else if (!fStart) cout <<" Not start..."<<endl;
    }
}

void TaPanam::InitCanFDBK()
{
  // start runbird
  //  gSystem->Exec("/runbirdPATH/runbird");
  // Init integrated plots...
}

void TaPanam::UpdateCanFDBK()
{

}

void TaPanam::UpdateCanDITH()
{

}

Int_t TaPanam::InitCanvasPads()
{
  CountDataInList();
  UpdateCanvasFlock();
  SetStyles();
  UpdateCanvas("TimeSignal",fTimeidx,0,fTSCorH);// 1 for dev
  //UpdateCanvas("PADC",fPADCidx,2,fADCSCorH); // 2 for Adev !!!
  InitADCStacks();
  UpdateCanvas("BCM",fBCMidx,fBCMplot,fBCMSCorH); 
  UpdateCanvas("BCMCAV",fBCMCAVidx,fBCMplot,fBCMSCorH); 
  UpdateCanvas("BPMIN",fBPMINidx,fBPMplot,fBPMSCorH); 
  UpdateCanvas("BPM",fBPMidx,fBPMplot,fBPMSCorH); 
  UpdateCanvas("BPMCAV",fBPMCAVidx,fBPMplot,fBPMSCorH);
  UpdateCanFDBK(); 
  UpdateCanDITH();
  UpdateCanvas("LUMI",fLUMIidx,3,kFALSE); 
  //InitGraphs(1);
  //InitGraphs(2);
 return 1;
}

Int_t TaPanam::UpdatePads()
{
  //UpdateCanvasFlock();
 UpdateCanvas("TimeSignal",fTimeidx,0,fTSCorH);  
 UpdateADCStacks();
 //UpdateCanvas("IADC",fPADCidx,2,fADCSCorH);
 //UpdateCanvas("PADC",fPADCidx,2,fADCSCorH);
 //UpdateCanvas("SADC",fPADCidx,2,fADCSCorH);
 UpdateCanvas("BCM",fBCMidx,fBPMplot,fBCMSCorH); 
 UpdateCanvas("BPM",fBPMidx,fBPMplot,fBPMSCorH); 
 UpdateCanFDBK(); 
 UpdateCanDITH(); 
 UpdateCanvas("LUMI",fLUMIidx,3,kFALSE); 
  //UpdateGraphs(1);
 //UpdateGraphs(2);
  return 1;
}

void TaPanam::InitADCStacks()
{
   if ((gROOT->GetListOfCanvases()->FindObject("PADC"))!=NULL) 
      {
	if ((UInt_t)fPADCidx.size()/4 <= 5 )
	  {
           fPADCCan->Divide((UInt_t)fPADCidx.size()/4);
           fPADCCan->cd();
           for ( UInt_t idx = 0; idx < (UInt_t) fPADCidx.size()/4; idx++)
             {
              fPADCCan->cd(idx+1);
              fAnaMan->GetAnalysis()->InitADCStack(idx);
	     }
           }
	else
	  {
           fPADCCan->Divide(2,(UInt_t)((fPADCidx.size()/8)+0.5));
           fPADCCan->cd();
           for ( UInt_t idx = 0; idx < (UInt_t) fPADCidx.size()/4; idx++)
             {
              fPADCCan->cd(idx+1);
              fAnaMan->GetAnalysis()->InitADCStack(idx);
	     }
	  }
      }
       fPADCCan->Update();
}

void TaPanam::UpdateADCStacks()
{
   if ((gROOT->GetListOfCanvases()->FindObject("PADC"))!=NULL && fAnaMan != NULL)
     {
       for ( UInt_t idx = 0; idx < (UInt_t)fPADCidx.size()/4 ; idx++)
       {
         fPADCCan->cd(idx+1);
	 fAnaMan->GetAnalysis()->DrawADCStack(idx); 
       }
       fPADCCan->Update();
     }
}

void TaPanam::InitGraphs(Int_t devtype)
{
  switch (devtype)
    {
    case 1:
      if ((gROOT->GetListOfCanvases()->FindObject("LUMI"))!=NULL)
        {
	  fLUMICan->Divide(3,2);
	  fLUMICan->cd(1);
          fAnaMan->GetAnalysis()->InitLumiGraph(0);
	  fLUMICan->cd(4);
          fAnaMan->GetAnalysis()->InitLumiGraph(1);
	  fLUMICan->cd(2);
          fAnaMan->GetAnalysis()->InitLumiGraph(2);
	  fLUMICan->cd(5);
          fAnaMan->GetAnalysis()->InitLumiGraph(3);
	  fLUMICan->cd(3);
          fAnaMan->GetAnalysis()->InitLumiGraph(4);
	  fLUMICan->cd(6);
          fAnaMan->GetAnalysis()->InitLumiGraph(5);
          fLUMICan->Update();
        }
      break;
    case 2:
      if ((gROOT->GetListOfCanvases()->FindObject("DET"))!=NULL)
        { 
          fDETCan->Divide(4,2);
          fDETCan->cd(1);
          fAnaMan->GetAnalysis()->InitDaltonGraph(0);
	  fDETCan->cd(5);
          fAnaMan->GetAnalysis()->InitDaltonGraph(1);
	  fDETCan->cd(2);
          fAnaMan->GetAnalysis()->InitDaltonGraph(2);
	  fDETCan->cd(6);
          fAnaMan->GetAnalysis()->InitDaltonGraph(3);
	  fDETCan->cd(3);
          fAnaMan->GetAnalysis()->InitDaltonGraph(4);
	  fDETCan->cd(7);
          fAnaMan->GetAnalysis()->InitDaltonGraph(5);
	  fDETCan->cd(4);
          fAnaMan->GetAnalysis()->InitDaltonGraph(6);
	  fDETCan->cd(8);
          fAnaMan->GetAnalysis()->InitDaltonGraph(7);
          fDETCan->Update();
        }
      break;
    }    
}

void TaPanam::UpdateGraphs(Int_t devtype)
{
  switch(devtype)
    {
    case 1:
      if ((gROOT->GetListOfCanvases()->FindObject("LUMI"))!=NULL)
        {
	 fLUMICan->cd(1);
         fAnaMan->GetAnalysis()->DrawLumiGraph(0);
	 fLUMICan->cd(4);
         fAnaMan->GetAnalysis()->DrawLumiGraph(1);
	 fLUMICan->cd(2);
         fAnaMan->GetAnalysis()->DrawLumiGraph(2);
	 fLUMICan->cd(5);
         fAnaMan->GetAnalysis()->DrawLumiGraph(3);
	 fLUMICan->cd(3);
         fAnaMan->GetAnalysis()->DrawLumiGraph(4);
	 fLUMICan->cd(6);
         fAnaMan->GetAnalysis()->DrawLumiGraph(5);
         fLUMICan->Update();
       }
      break;
    case 2:
      if ((gROOT->GetListOfCanvases()->FindObject("DET"))!=NULL)
        { 
  	 fDETCan->cd(1);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(0);
         fDETCan->cd(5);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(1);
         fDETCan->cd(2);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(2);
         fDETCan->cd(6);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(3);
         fDETCan->cd(3);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(4);
         fDETCan->cd(7);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(5);
         fDETCan->cd(4);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(6);
         fDETCan->cd(8);
         fAnaMan->GetAnalysis()->DrawDaltonGraph(7);
         fDETCan->Update();
        }
      break;
    }
}

void TaPanam::UpdateEmCanvas()
{
//  if (fEmCanDevPlotType) fEmCanToUpdate = kTRUE;
//  if (fEmCanToUpdate) 
//    {
//     fEmCan->GetCanvas()->cd();
//     cout<<" cd on Embedded canvas"<<endl;
//     //    (fAnaMan->GetAnalysis()->GetMonitorDevice(fEmCanDevName))->GetPlot(fEmCanDevPlotName,fEmCanDevPlotType)->Draw();
//     fEmCan->GetCanvas()->Update();
//    }
}

Int_t TaPanam::ClearCanvas()
{
  if ((fTimeCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("TimeSignal")) 
     CanvasSafeDelete(fTimeCan,"TimeSignal");

  if ((fIADCCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("IADC")) 
     CanvasSafeDelete(fIADCCan,"IADC");
  if ((fPADCCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("PADC")) 
     CanvasSafeDelete(fPADCCan,"PADC");
  if ((fSADCCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("SADC")) 
     CanvasSafeDelete(fSADCCan,"SADC");
  if ((fBCMCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("BCM")) 
     CanvasSafeDelete(fBCMCan,"BCM");
  if ((fBCMCAVCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("BCMCAV")) 
     CanvasSafeDelete(fBCMCAVCan,"BCMCAV");
  if ((fBPMINCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("BPMIN")) 
     CanvasSafeDelete(fBPMINCan,"BPMIN");
  if ((fBPMCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("BPM")) 
     CanvasSafeDelete(fBPMCan,"BPM");
  if ((fBPMCAVCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("BPMCAV")) 
     CanvasSafeDelete(fBPMCAVCan,"BPMCAV");
  if ((fFDBKCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("FDBK")) 
     CanvasSafeDelete(fFDBKCan,"FDBK");
  if ((fDITHCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("DITH")) 
     CanvasSafeDelete(fDITHCan,"DITH");
  if ((fLUMICan!=NULL) && gROOT->GetListOfCanvases()->FindObject("LUMI")) 
     CanvasSafeDelete(fLUMICan,"LUMI");
  if ((fDETCan!=NULL) && gROOT->GetListOfCanvases()->FindObject("DET")) 
     CanvasSafeDelete(fDETCan,"DET");
  return 1;
}
Int_t TaPanam::ClearListBox()
{  
  Int_t j=1;
  while (fHistoListBox->GetSelection(j) != kFALSE )
    {
      fHistoListBox->RemoveEntry(j);
      j++;
    }
     MapSubwindows();
     fHistoListBox->Layout();
     Resize(950,500);
     MapWindow();
 return 1;
}

Int_t TaPanam::ClearListOfDevice()
{  
  fDeviceList.clear();
  fADeviceList.clear();
  fAllDeviceList.clear();
 return 1;
}

void TaPanam::InitGUIFrames(){
  ////////////////     Menu bar  //////////////////////////////////////////

  //   define layout parameters for menu bar  
  fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
				     0, 0, 1, 1);  
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
     // File menu
  fMenuFile = new TGPopupMenu(fClient->GetRoot());
//   fMenuFile->AddEntry("&Open Root file...", M_FILE_OPEN);
//   fMenuFile->AddEntry("Edit Pan &MySQL Data Base",M_FILE_EDITDB);
//   fMenuFile->AddSeparator();
//   fMenuFile->AddEntry("&Save all Canvas as ps file", M_FILE_SAVE);
//   fMenuFile->AddEntry("&Save current as eps file", M_FILE_SAVE1);
//   fMenuFile->AddEntry("&Save current as gif file", M_FILE_SAVE2);
//   fMenuFile->AddEntry("&Print all Canvas",M_FILE_PRINT);
//   fMenuFile->AddEntry("&Print current Canvas",M_FILE_PRINT1);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_FILE_EXIT);
      // Monitoring menu
  fMenuMon = new TGPopupMenu(fClient->GetRoot());
  fMenuMon->AddEntry("Prestart ", M_MON_PRESTART);
  fMenuMon->AddEntry("Start ", M_MON_START);
  fMenuMon->DisableEntry(M_MON_START);
  fMenuMon->AddEntry("Stop", M_MON_STOP);
  fMenuMon->DisableEntry(M_MON_STOP);
  fMenuMon->AddEntry("End ", M_MON_END);
  fMenuMon->DisableEntry(M_MON_END);
  fMenuMon->AddSeparator();
  fMenuMon->AddEntry("Reset all plots",M_MON_HISTOALLRAZ);
  fMenuMon->DisableEntry(M_MON_HISTOALLRAZ);
       // Analysis Launch Menu
  fMenuAna =  new TGPopupMenu(fClient->GetRoot());
//   fMenuAna->AddEntry("Prompt Analysis", M_ANA_PROMPT);
//   fMenuMon->DisableEntry(M_ANA_PROMPT);
//   fMenuAna->AddEntry("Regression Analysis", M_ANA_REGRESS);
//   fMenuMon->DisableEntry(M_ANA_REGRESS);
//   fMenuAna->AddEntry("Beam Modulation analysis", M_ANA_BEAMMOD);   
//  fMenuMon->DisableEntry(M_ANA_BEAMMOD);
     // Menu button messages are handled by the main frame (i.e. "this")
     // ProcessMessage() method.
  fMenuFile->Associate(this);
  fMenuMon->Associate(this);
  fMenuAna->Associate(this);

  fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Monitoring", fMenuMon, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Analysis", fMenuAna, fMenuBarItemLayout);
  AddFrame(fMenuBar, fMenuBarLayout);
  //////////////////////  TITLE FRAME   //////////////////////////////////////////
   
  //  define layout parameters
   fL1  = new TGLayoutHints(kLHintsTop | kLHintsLeft,0,0,0,0);
   fL2  = new TGLayoutHints(kLHintsCenterX ,20,10,20,10);
   fL5  = new TGLayoutHints(kLHintsTop | kLHintsLeft,2,2,2,2);
   fL3  = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY  ,5,5,2,2);
   fL4  = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
			   kLHintsExpandY, 2, 2, 2, 2);
   fL6  = new TGLayoutHints(kLHintsCenterX ,20,10,20,10);
   fL7  = new TGLayoutHints(kLHintsCenterX,0,0,0,0);
   fL8  = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2);
   fL9  =  new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,2, 2, 2, 2);
   //LOGO+TITLE
   fMainTitle = new TGCompositeFrame(this, 50, 100,kHorizontalFrame | kLHintsExpandX );
   //fLogoButton = new TGPictureButton(fMainTitle,gClient->GetPicture("HAPPEX2.xpm"));    
   FontStruct_t labelfont = gClient->GetFontByName("-*-times-bold-r-*-*-24-*-*-*-*-*-*-*");
   GCValues_t   gval;
   gval.fMask = kGCForeground | kGCFont;
   gval.fFont = gVirtualX->GetFontHandle(labelfont);
   gClient->GetColorByName("orange", gval.fForeground);
   GContext_t fTextGC = gVirtualX->CreateGC(gClient->GetRoot()->GetId(), &gval);
   fTitle = new TGLabel(fMainTitle,"PAN Monitoring ",fTextGC,labelfont);
   fMainTitle->AddFrame(fTitle,fL2);
   AddFrame(fMainTitle,fL1);
   //////////////////  MAIN FRAME   //////////////////////////////////////////////

     // Main frame
   fMainFrame = new TGCompositeFrame(this,100,100,kHorizontalFrame | kRaisedFrame);
   AddFrame(fMainFrame,fL4);

   /////////////////////  UTILITY FRAME ///////////////////////////////////////////////
   fUtilFrame = new TGCompositeFrame(fMainFrame,100,100,kVerticalFrame);
   fMainFrame->AddFrame(fUtilFrame,fL3);
   // Button box
   fButtonFrame = new TGCompositeFrame(fUtilFrame,100,100,kHorizontalFrame);
   fUtilFrame->AddFrame(fButtonFrame,fL5);
  // Some buttons
//   fButton1 = new TGTextButton(fButtonFrame,"Plot on new Canvas",1);
//   fButton1->Associate(this);
//   fButtonFrame->AddFrame(fButton1,fL5);
  //  fButton2 = new TGTextButton(fButtonFrame,"sleep",2);
  //  fButton2->Associate(this);
  //  fButtonFrame->AddFrame(fButton2,fL5);
  //  fButton3 = new TGTextButton(fButtonFrame,"",3);
  //  fButton3->Associate(this);
  //  fButtonFrame->AddFrame(fButton3,fL5);

  fHistoTreeView = new TGCanvas(fUtilFrame, 300, 100,kSunkenFrame | kDoubleBorder);
  fHistoListBox = new TGListBox(fHistoTreeView->GetViewPort(),10, 10, kHorizontalFrame);
  fHistoListBox->Associate(this);
  //fHistoListBox->AddEntry("List of plots initialized ... ",1);
  fHistoTreeView->SetContainer(fHistoListBox);
  fUtilFrame->AddFrame(fHistoTreeView,fL3);

  /////////////////  Draw folder with canvas and checkbox options ////  
  fTab = new TGTab(fMainFrame,300,300);
  fTab->Associate(this);
  // set time/helicity signal displaying pairsynch and helicity stripchart
  fHistoFrame = fTab->AddTab("Time & Helicity");
  fTab->Associate(this);
  fTimeFrame  = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);    
  fTimegFrame  = new TGGroupFrame(fTimeFrame,new TGString(" Signals "));
  fTimeCheck[0] = new TGCheckButton(fTimegFrame, " Quadsynch     ", 10);
  fTimeCheck[1] = new TGCheckButton(fTimegFrame, " Pairsynch     ", 11);
  fTimeCheck[2] = new TGCheckButton(fTimegFrame, " Helicitity    ", 12);
  fTimeCheck[3] = new TGCheckButton(fTimegFrame, " Timeslot      ", 13);
  fTimeCheck[4] = new TGCheckButton(fTimegFrame, " v2fclock      ", 14);
  fTimeCheck[0]->Associate(this);
  fTimeCheck[1]->Associate(this);
  fTimeCheck[2]->Associate(this);
  fTimeCheck[3]->Associate(this);
  fTimeCheck[4]->Associate(this);
  // set default  
  fTimeCheck[1]->SetState(kButtonDown);
  fTimeCheck[2]->SetState(kButtonDown);
  for (Int_t i =0 ; i< 5 ;i++) fTimegFrame->AddFrame(fTimeCheck[i],fL4);
  fTOptgFrame =   new TGGroupFrame(fTimeFrame,new TGString(" Display Options ")); 
  fTOptCheck[0] = new TGCheckButton(fTOptgFrame, " Display ON/OFF ",15);
  fTOptCheck[1] = new TGCheckButton(fTOptgFrame, " StripChart     ",16);
  fTOptCheck[2] = new TGCheckButton(fTOptgFrame, " Histogram      ",17);
  fTOptCheck[0]->Associate(this);
  fTOptCheck[1]->Associate(this);
  fTOptCheck[2]->Associate(this);
  //set default
  //fTOptCheck[0]->SetState(kButtonDown); 
  fTOptCheck[1]->SetState(kButtonDown); 
  for (Int_t i =0 ; i< 3 ;i++) fTOptgFrame->AddFrame(fTOptCheck[i],fL4);
  fTimeFrame->AddFrame(fTimegFrame,fL2);
  fTimeFrame->AddFrame(fTOptgFrame,fL2);
  fHistoFrame->AddFrame(fTimeFrame,fL2);
    
  fHistoFrame = fTab->AddTab("ADC");
  fTab->Associate(this);
  fADCScalerFrame = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);
  fADCFrame0 = new TGCompositeFrame(fHistoFrame, 60, 60, kHorizontalFrame);
//  fIADCCheck[0] = new TGCheckButton(fADCFrame0, " IADC0  ",20); 
//  fIADCCheck[1] = new TGCheckButton(fADCFrame0, " IADC1  ",21); 
//   fIADCCheck[0]->Associate(this);
//   fIADCCheck[1]->Associate(this);
//   for (Int_t i =0 ; i<2  ;i++) fADCFrame0->AddFrame(fIADCCheck[i],fL4);
  fADCFrame1 = new TGCompositeFrame(fHistoFrame, 60, 60, kHorizontalFrame);
  fADCCheck[0] = new TGCheckButton(fADCFrame1, " ADC0   ",22); 
  fADCCheck[1] = new TGCheckButton(fADCFrame1, " ADC1   ",23); 
  fADCCheck[2] = new TGCheckButton(fADCFrame1, " ADC2   ",24); 
  fADCCheck[3] = new TGCheckButton(fADCFrame1, " ADC3   ",25); 
  fADCCheck[4] = new TGCheckButton(fADCFrame1, " ADC4   ",26); 
  fADCCheck[0]->Associate(this);
  fADCCheck[1]->Associate(this);
  fADCCheck[2]->Associate(this);
  fADCCheck[3]->Associate(this);
  fADCCheck[4]->Associate(this);
  fADCCheck[0]->SetState(kButtonDown);
  fADCCheck[1]->SetState(kButtonDown);
  fADCCheck[2]->SetState(kButtonDown);
  fADCCheck[3]->SetState(kButtonDown);
  fADCCheck[4]->SetState(kButtonDown);
  for (Int_t i =0 ; i<5  ;i++) fADCFrame1->AddFrame(fADCCheck[i],fL4);
  fADCFrame2 = new TGCompositeFrame(fHistoFrame, 60, 60, kHorizontalFrame);
  fADCCheck[5] = new TGCheckButton(fADCFrame2, " ADC5   ",27); 
  fADCCheck[6] = new TGCheckButton(fADCFrame2, " ADC6   ",28); 
  fADCCheck[7] = new TGCheckButton(fADCFrame2, " ADC7   ",29); 
  fADCCheck[8] = new TGCheckButton(fADCFrame2, " ADC8   ",30); 
  fADCCheck[9] = new TGCheckButton(fADCFrame2, " ADC9   ",31); 
  fADCCheck[5]->Associate(this);
  fADCCheck[6]->Associate(this);
  fADCCheck[7]->Associate(this);
  fADCCheck[8]->Associate(this);
  fADCCheck[9]->Associate(this);
  //set default
  fADCCheck[5]->SetState(kButtonDown);
  fADCCheck[6]->SetState(kButtonDown);
  fADCCheck[7]->SetState(kButtonDown);
  fADCCheck[8]->SetState(kButtonDown);
  fADCCheck[9]->SetState(kButtonDown);
  for (Int_t i =5 ; i<10  ;i++) fADCFrame2->AddFrame(fADCCheck[i],fL4);    
   fADCFrame3 = new TGCompositeFrame(fHistoFrame, 60, 60, kHorizontalFrame);
//   fSADCCheck[0] = new TGCheckButton(fADCFrame3, " SADC0  ",32); 
//   fSADCCheck[1] = new TGCheckButton(fADCFrame3, " SADC1  ",33); 
//   fSADCCheck[0]->Associate(this);
//   fSADCCheck[1]->Associate(this);
//   for (Int_t i =0 ; i<2  ;i++) fADCFrame3->AddFrame(fSADCCheck[i],fL4);
  fADCOptgFrame   = new TGGroupFrame(fADCScalerFrame,new TGString(" Display Options ")); 
  fADCOptCheck[0] = new TGCheckButton(fADCOptgFrame, " Display Injector DAQ ADCs ON/OFF ",34);
  fADCOptCheck[1] = new TGCheckButton(fADCOptgFrame, " Display Parity DAQ ADCs   ON/OFF ",35);
  fADCOptCheck[2] = new TGCheckButton(fADCOptgFrame, " Display Spectro DAQ ADCs  ON/OFF ",36);
  fADCOptCheck[3] = new TGCheckButton(fADCOptgFrame, " Raw Data        ",37);
  fADCOptCheck[4] = new TGCheckButton(fADCOptgFrame, " Data, pedestal/DAC noise subtracted ",38);
  fADCOptCheck[5] = new TGCheckButton(fADCOptgFrame, " StripChart      ",39);
  fADCOptCheck[6] = new TGCheckButton(fADCOptgFrame, " Histos          ",40);
  fADCOptCheck[0]->Associate(this);
  fADCOptCheck[1]->Associate(this);
  fADCOptCheck[2]->Associate(this);
  fADCOptCheck[3]->Associate(this);
  fADCOptCheck[4]->Associate(this);
  fADCOptCheck[5]->Associate(this);
  fADCOptCheck[6]->Associate(this);
  //set default
  fADCOptCheck[1]->SetState(kButtonDown); 
  fADCOptCheck[4]->SetState(kButtonDown); 
  fADCOptCheck[5]->SetState(kButtonDown); 
  for (Int_t i =0 ; i< 7 ;i++) fADCOptgFrame->AddFrame(fADCOptCheck[i],fL1);
  fADCScalerFrame->AddFrame(fADCFrame0);  
  fADCScalerFrame->AddFrame(fADCFrame1);  
  fADCScalerFrame->AddFrame(fADCFrame2);  
  fADCScalerFrame->AddFrame(fADCFrame3);  
  fADCScalerFrame->AddFrame(fADCOptgFrame);  
  fHistoFrame->AddFrame(fADCScalerFrame,fL2);
  
  fHistoFrame = fTab->AddTab("Beam Current");
  fTab->Associate(this);
  fCurrentFrame  = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);   
   
  fBCMgFrame    = new TGGroupFrame(fCurrentFrame,new TGString(" BCMs "));
  //  fInjBPMWSFrame     = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
//   fInjBPMWSCheck[0]  = new TGCheckButton(fInjBPMWSFrame, " BPMIN1 wiresums  ",41);
//   fInjBPMWSCheck[1]  = new TGCheckButton(fInjBPMWSFrame, " BPMIN2 wiresums  ",42);
//   fInjBPMWSCheck[0]->Associate(this);
//   fInjBPMWSCheck[1]->Associate(this);
//   for (Int_t i =0 ; i<2  ;i++) fInjBPMWSFrame->AddFrame(fInjBPMWSCheck[i],fL3);
//  fBCMgFrame->AddFrame(fInjBPMWSFrame,fL3);
  fBCMFrame     = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
  fBCMCheck[0]  = new TGCheckButton(fBCMFrame, " BCM1  ",43);
  fBCMCheck[1]  = new TGCheckButton(fBCMFrame, " BCM2  ",44);
  fBCMCheck[2]  = new TGCheckButton(fBCMFrame, " Unser ",45);
  fBCMCheck[0]->Associate(this);
  fBCMCheck[1]->Associate(this);
  fBCMCheck[2]->Associate(this);
  fBCMCheck[0]->SetState(kButtonDown);
  fBCMCheck[1]->SetState(kButtonDown);
  // fBCMCheck[2]->SetState(kButtonDown);
  for (Int_t i =0 ; i<3  ;i++) fBCMFrame->AddFrame(fBCMCheck[i],fL3);
  fBCMgFrame->AddFrame(fBCMFrame,fL3);
  //  fBCMCavFrame     = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
//   fBCMCavCheck[0]  = new TGCheckButton(fBCMCavFrame, " BCMCavity1  ",50);
//   fBCMCavCheck[1]  = new TGCheckButton(fBCMCavFrame, " BCMCavity2  ",51);
//   fBCMCavCheck[0]->Associate(this);
//   fBCMCavCheck[1]->Associate(this);
//   for (Int_t i =0 ; i<2  ;i++) fBCMCavFrame->AddFrame(fBCMCavCheck[i],fL3);
//  fBCMgFrame->AddFrame(fBCMCavFrame,fL3);

  fBCMDFrame     = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
  fBCMDCheck[0]  = new TGCheckButton(fBCMDFrame, " Display BCMs    ON/OFF  ",52);
  fBCMDCheck[1]  = new TGCheckButton(fBCMDFrame, " Display BCMCAVs ON/OFF  ",53);
  fBCMDCheck[0]->Associate(this);
  fBCMDCheck[1]->Associate(this);
  fBCMDCheck[0]->SetState(kButtonDown);
  for (Int_t i =0 ; i<2  ;i++) fBCMDFrame->AddFrame(fBCMDCheck[i],fL3);
  fBCMgFrame->AddFrame(fBCMDFrame,fL3);

  fBCMDispFrame = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
  fBCMDispCheck[0]  = new TGCheckButton(fBCMDispFrame, " Data  ",54);
  fBCMDispCheck[1]  = new TGCheckButton(fBCMDispFrame, " Asym ",55);
  fBCMDispCheck[2]  = new TGCheckButton(fBCMDispFrame, " All ",56);
  fBCMDispCheck[0]->Associate(this);
  fBCMDispCheck[1]->Associate(this);
  fBCMDispCheck[2]->Associate(this);
  fBCMDispCheck[1]->SetState(kButtonDown);
  for (Int_t i =0 ; i<3  ;i++) fBCMDispFrame->AddFrame(fBCMDispCheck[i],fL4);
  fBCMgFrame->AddFrame(fBCMDispFrame,fL3);
  fBCMOptFrame  = new TGCompositeFrame(fBCMgFrame, 60, 60, kHorizontalFrame);
  fBCMOptCheck[0]  = new TGCheckButton(fBCMOptFrame, " Stripchart ",57);
  fBCMOptCheck[1]  = new TGCheckButton(fBCMOptFrame, " Histogram  ",58);
  fBCMOptCheck[0]->Associate(this);
  fBCMOptCheck[1]->Associate(this);
  fBCMOptCheck[0]->SetState(kButtonDown);
  for (Int_t i =0 ; i<2  ;i++) fBCMOptFrame->AddFrame(fBCMOptCheck[i],fL4);
  fBCMgFrame->AddFrame(fBCMOptFrame,fL3);
  fCurrentFrame->AddFrame(fBCMgFrame ,fL2);      
  fHistoFrame->AddFrame(fCurrentFrame,fL2);
  
  fHistoFrame = fTab->AddTab("Beam Position");
  fTab->Associate(this);
     fPositionFrame  = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);   
    fBPMgFrame = new TGGroupFrame(fPositionFrame,new TGString(" Stripline BPM "));
    //    fInjBPMFrame     = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
//     fInjBPMCheck[0]  = new TGCheckButton(fInjBPMFrame, " BPMIN1 ",70);
//     fInjBPMCheck[1]  = new TGCheckButton(fInjBPMFrame, " BPMIN2 ",71);
//     fInjBPMCheck[0]->Associate(this);
//     fInjBPMCheck[1]->Associate(this);
//     for (Int_t i =0 ; i<2  ;i++) fInjBPMFrame->AddFrame(fInjBPMCheck[i],fL3);
//    fBPMgFrame->AddFrame(fInjBPMFrame,fL3);
    fBPMFrame     = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
    fBPMCheck[0]  = new TGCheckButton(fBPMFrame, " BPM12 ",72);
    fBPMCheck[1]  = new TGCheckButton(fBPMFrame, " BPM10 ",73);
    fBPMCheck[2]  = new TGCheckButton(fBPMFrame, " BPM8 ",74);
    fBPMCheck[3]  = new TGCheckButton(fBPMFrame, " BPM4A ",75);
    fBPMCheck[4]  = new TGCheckButton(fBPMFrame, " BPM4B ",76);
    fBPMCheck[5]  = new TGCheckButton(fBPMFrame, " Angles(tx,ty) ",77);
    fBPMCheck[0]->Associate(this);
    fBPMCheck[1]->Associate(this);
    fBPMCheck[2]->Associate(this);
    fBPMCheck[3]->Associate(this);
    fBPMCheck[4]->Associate(this);
    fBPMCheck[5]->Associate(this);
//        fBPMCheck[0]->SetState(kButtonDown);
    fBPMCheck[3]->SetState(kButtonDown);
    fBPMCheck[4]->SetState(kButtonDown);
    for (Int_t i =0 ; i<6  ;i++) fBPMFrame->AddFrame(fBPMCheck[i],fL3);
    fBPMgFrame->AddFrame(fBPMFrame,fL3);
    //    fBPMCavFrame     = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
//     fBPMCavCheck[0]  = new TGCheckButton(fBPMCavFrame, " BPMCavity1 ",78);
//     fBPMCavCheck[1]  = new TGCheckButton(fBPMCavFrame, " BPMCavity2 ",79);
//     fBPMCavCheck[0]->Associate(this);
//     fBPMCavCheck[1]->Associate(this);
//     for (Int_t i =0 ; i<2  ;i++) fBPMCavFrame->AddFrame(fBPMCavCheck[i],fL3);
//    fBPMgFrame->AddFrame(fBPMCavFrame,fL3);

    fBPMDFrame     = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
    fBPMDCheck[0]  = new TGCheckButton(fBPMDFrame, " Display Inj BPMs ON/OFF ",80);
    fBPMDCheck[1]  = new TGCheckButton(fBPMDFrame, " Hall A BPMs ON/OFF ",81);
    fBPMDCheck[2]  = new TGCheckButton(fBPMDFrame, " Hall A Cav ON/OFF  ",82);
    fBPMDCheck[0]->Associate(this);
    fBPMDCheck[1]->Associate(this);
    fBPMDCheck[2]->Associate(this);
    fBPMDCheck[1]->SetState(kButtonDown);  
    for (Int_t i =0 ; i<3  ;i++) fBPMDFrame->AddFrame(fBPMDCheck[i],fL3);
    fBPMgFrame->AddFrame(fBPMDFrame,fL2);

    fBPMDispFrame = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
    fBPMDispCheck[0]  = new TGCheckButton(fBPMDispFrame, " Data  ",83);
    fBPMDispCheck[1]  = new TGCheckButton(fBPMDispFrame, " Differences  ",84);
    fBPMDispCheck[2]  = new TGCheckButton(fBPMDispFrame, " Data & Differences  ",85);
    fBPMDispCheck[0]->Associate(this);
    fBPMDispCheck[1]->Associate(this);
    fBPMDispCheck[2]->Associate(this);
    fBPMDispCheck[1]->SetState(kButtonDown);
    for (Int_t i =0 ; i<3  ;i++) fBPMDispFrame->AddFrame(fBPMDispCheck[i],fL3);
    fBPMgFrame->AddFrame(fBPMDispFrame,fL2);

    fBPMOptFrame  = new TGCompositeFrame(fBPMgFrame, 60, 60, kHorizontalFrame);
    fBPMOptCheck[0]  = new TGCheckButton(fBPMOptFrame, " Stripchart  ",86);
    fBPMOptCheck[1]  = new TGCheckButton(fBPMOptFrame, " Histogram  ",87);
    fBPMOptCheck[0]->Associate(this);
    fBPMOptCheck[1]->Associate(this);
    fBPMOptCheck[0]->SetState(kButtonDown);  
    for (Int_t i =0 ; i<2  ;i++) fBPMOptFrame->AddFrame(fBPMOptCheck[i],fL3);
    fBPMgFrame->AddFrame(fBPMOptFrame,fL2);
     
    fPositionFrame->AddFrame(fBPMgFrame,fL2);      
    fHistoFrame->AddFrame(fPositionFrame,fL2);
  
  fHistoFrame = fTab->AddTab("Beam Control");
  fTab->Associate(this);
    fControlFrame = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);
    fFeedbackFrame  = new TGCompositeFrame(fControlFrame, 60, 60, kHorizontalFrame);
    fFCurgFrame   = new TGGroupFrame(fFeedbackFrame,new TGString("Current Feedback"));    
    fFCurFrame    = new TGCompositeFrame(fFCurgFrame, 60, 60, kHorizontalFrame);
    fFCurCheck[0]  = new TGCheckButton(fFCurFrame, "PITA",100);
    fFCurCheck[1]  = new TGCheckButton(fFCurFrame, "IA Cell",101);  
    for (Int_t i =0 ; i<2  ;i++) 
      {
       fFCurCheck[i]->Associate(this);
       fFCurFrame->AddFrame(fFCurCheck[i],fL3);
      }
    fFCurgFrame->AddFrame(fFCurFrame,fL3);
    fFPosgFrame   = new TGGroupFrame(fFeedbackFrame,new TGString("Position Feedback"));    
    fFPosFrame    = new TGCompositeFrame(fFPosgFrame, 60, 60, kHorizontalFrame);
    fFPosCheck    = new TGCheckButton(fFPosFrame, "PZT",102);
    fFPosCheck->Associate(this);
    fFPosFrame->AddFrame(fFPosCheck,fL3);
    fFPosgFrame->AddFrame(fFPosFrame,fL3);    
    fFeedbackFrame->AddFrame(fFCurgFrame,fL2);
    fFeedbackFrame->AddFrame(fFPosgFrame,fL2);

//     fDITHgFrame   = new TGGroupFrame(fControlFrame,new TGString("Beam Dithering"));    
//     fDITHFrame    = new TGCompositeFrame(fDITHgFrame, 60, 60, kHorizontalFrame);
//     fDITHCheck[0]    = new TGCheckButton(fDITHFrame, "Position",103);
//     fDITHCheck[1]    = new TGCheckButton(fDITHFrame, "Energy",104);
//     fDITHCheck[0]->Associate(this);
//     fDITHCheck[1]->Associate(this);
//     fDITHFrame->AddFrame(fDITHCheck[0],fL3);
//     fDITHFrame->AddFrame(fDITHCheck[1],fL3);
//     fDITHgFrame->AddFrame(fDITHFrame,fL3); 
           
    fControlFrame->AddFrame(fFeedbackFrame,fL2);
//     fControlFrame->AddFrame(fDITHgFrame,fL2);
    fHistoFrame->AddFrame(fControlFrame,fL2);
  
  fHistoFrame = fTab->AddTab("Lumi & Daltons");
  fTab->Associate(this);
    fDetFrame     = new TGCompositeFrame(fHistoFrame, 60, 60, kVerticalFrame);
    fDalgFrame    = new TGGroupFrame(fDetFrame,new TGString(" The Daltons "));
    fDalFrame     = new TGCompositeFrame(fDalgFrame, 60, 60, kHorizontalFrame);
    fDalCheck[0]  = new TGCheckButton(fDalFrame, "Joe",110);    
    fDalCheck[1]  = new TGCheckButton(fDalFrame, "Jack",111);    
    fDalCheck[2]  = new TGCheckButton(fDalFrame, "William",112);    
    fDalCheck[3]  = new TGCheckButton(fDalFrame, "Averell",113); 
   for (Int_t i =0 ; i<4  ;i++) 
      {
       fDalCheck[i]->Associate(this);
       fDalFrame ->AddFrame(fDalCheck[i],fL2);
      }
    fDalgFrame->AddFrame(fDalFrame ,fL2);
  
    fLumigFrame    = new TGGroupFrame(fDetFrame,new TGString(" Luminosity detectors"));
    fLumiFrame     = new TGCompositeFrame(fLumigFrame, 60, 60, kHorizontalFrame);
    fLumiCheck[0]  = new TGCheckButton(fLumiFrame, "LUMI1",114);    
    fLumiCheck[1]  = new TGCheckButton(fLumiFrame, "LUMI2",115);    
    fLumiCheck[2]  = new TGCheckButton(fLumiFrame, "LUMI3",116);    
    fLumiCheck[3]  = new TGCheckButton(fLumiFrame, "LUMI4",117); 
    for (Int_t i =0 ; i<4  ;i++) 
      {
       fLumiCheck[i]->Associate(this);
       fLumiFrame ->AddFrame(fLumiCheck[i],fL2);
      }
    fLumigFrame->AddFrame(fLumiFrame ,fL2);
   
    fDetFrame->AddFrame(fDalgFrame,fL2); 
    fDetFrame->AddFrame(fLumigFrame,fL2); 
    fHistoFrame->AddFrame(fDetFrame,fL2);
       
  fHistoFrame = fTab->AddTab("Single PLOT");
  fTab->Associate(this);
  TGCompositeFrame *fEmCanFrame = new TGCompositeFrame(fHistoFrame, 60, 60, kHorizontalFrame);
  fEmCan = new TRootEmbeddedCanvas("EmCan", fEmCanFrame, 300, 300);
  fEmCanFrame->AddFrame(fEmCan,fL4);
  fHistoFrame->AddFrame(fEmCanFrame,fL4);
  fMainFrame->AddFrame(fTab,fL4);
  //fEmCan->GetCanvas()->Divide(1,2);
  
   //----------------------------------------------------------------------//
     // Display interface windows on screen
  SetWindowName("PANAM");
  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow(); 
  //-----------------------------------------------------------------------//
}

Int_t
TaPanam::ThreadIsRunning()
{
  if (!fPanThreadRun) return 0;
  else return 1;
}


void
TaPanam::DoPan()
{ // launch Pan analysis ( run analysis for the moment )
  // try initialize analysis manager out of the thread...  
  fAnaMan->Process();
  cout<<" killing refresh thread"<<endl;
  TThread::Kill("refreshThread"); 
  gSystem->Sleep(1000);
  RefreshThreadStop();   
  fAnaMan->End();
  cout<<" deleting fAnaMan object "<<endl; 
  delete fAnaMan;
  fAnaMan = NULL;
  cout<<"fAnaMan ptr :"<<fAnaMan<<endl;
  fMenuMon->DisableEntry(M_MON_STOP);     // disable STOP
  fMenuMon->DisableEntry(M_MON_START);   // disable START
  fMenuMon->EnableEntry(M_MON_PRESTART);   // enable PRESTART
  fMenuMon->EnableEntry(M_MON_END);   // enable PRESTART
  //   just a test....
  //  cout<<" Pan analysis"<<endl;
  //  gSystem->Sleep(500);   
}
void 
TaPanam::DoRefreshing()
{
#ifdef REFRESH
  cout<<"Refresh..."<<endl;
  // wait for 2 seconds of data
  if (RefreshCanvas()) cout<<" RefreshCanvas() : Canvases REFRESHED..."<<flush<<endl;
  gSystem->Sleep(2000);
#endif
}

Int_t TaPanam::RefreshCanvas()
{
  UpdatePads();
 return 1;
}

void TaPanam::SetStyles()
{
     fStripStyle = new TStyle("STRIPCHART_STYLE","STRIPCHART_STYLE");
     fStripStyle->SetOptDate(0);
     fStripStyle->SetOptStat(0);
     fStripStyle->SetOptLogy(0);
     fStripStyle->SetFrameBorderMode(0);
     fStripStyle->SetFrameBorderSize(0);
     fStripStyle->SetCanvasColor(10);
     fStripStyle->SetFrameFillColor(10);
     //fStripStyle->SetFrameLineWidth(10);
  
     // pads parameters
     fStripStyle->SetGridColor(33);
     fStripStyle->SetPadColor(10);
     fStripStyle->SetPadBorderMode(0);
     fStripStyle->SetPadBorderSize(0); 
     fStripStyle->SetPadLeftMargin(0.25);
     fStripStyle->SetPadGridX(1);
     fStripStyle->SetPadGridY(1);
     fStripStyle->SetFuncWidth(1);
     fStripStyle->SetTitleColor(10);
     fStripStyle->SetLabelSize(0.07,"x");
     fStripStyle->SetLabelSize(0.07,"y");
     fStripStyle->SetTitleFont(42);
     fStripStyle->SetTitleH(0.09);
     fStripStyle->SetTitleW(0.55);
     fStripStyle->SetTitleX(0.2);
     fStripStyle->SetTitleBorderSize(1);

  fHistoStyle = new TStyle("HISTOGRAMS_STYLE","HISTOGRAMS_STYLE");
  fHistoStyle->SetOptDate(1);
  fHistoStyle->SetOptStat(111111);
  fHistoStyle->SetOptLogy(0);
  fHistoStyle->SetFrameBorderMode(0);
  fHistoStyle->SetFrameBorderSize(0);
  fHistoStyle->SetCanvasColor(10);
  fHistoStyle->SetFrameFillColor(10);
  // pads parameters  
  fHistoStyle->SetFuncWidth(2);
  fHistoStyle->SetPadColor(10);
  fHistoStyle->SetPadBorderMode(0);
  fHistoStyle->SetPadBorderSize(0);
  fStripStyle->SetPadGridX(0);
  fStripStyle->SetPadGridY(0);
  fStripStyle->SetFuncWidth(1);
  fHistoStyle->SetTitleColor(10);
  fHistoStyle->SetLabelSize(0.05,"x");
  fHistoStyle->SetLabelSize(0.05,"y");
  fHistoStyle->SetTitleFont(42);
  fHistoStyle->SetTitleH(0.09);
  fHistoStyle->SetTitleW(0.8);
  fHistoStyle->SetTitleX(0.1);
  fHistoStyle->SetTitleBorderSize(1);
  fHistoStyle->SetOptFit(111111);

//   fGraphStyle = new TStyle("GRAPH_STYLE","GRAPH_STYLE");
//   fGraphStyle->SetOptStat(0);
//   fGraphStyle->SetOptLogx(0);
//   fGraphStyle->SetOptLogy(0);
//   fGraphStyle->SetPadGridX(0);
//   fGraphStyle->SetPadGridY(0);
//   fGraphStyle->SetFrameBorderMode(0);
//   fGraphStyle->SetFrameBorderSize(0);
//   fGraphStyle->SetPadBorderMode(0);
//   fGraphStyle->SetPadBorderSize(0);
//   fGraphStyle->SetCanvasColor(10);
//   fGraphStyle->SetFrameFillColor(10);
//   fGraphStyle->SetPalette(9,0);
//   fGraphStyle->SetFuncColor(2);
//   fGraphStyle->SetFuncWidth(2);
//   fGraphStyle->SetLabelSize(0.10,"x");
//   fGraphStyle->SetLabelSize(0.07,"y");
//   fGraphStyle->SetTitleFont(42);
//   fGraphStyle->SetTitleH(0.09);
//   fGraphStyle->SetTitleW(0.8);
//   fGraphStyle->SetTitleX(0.1);
//   fGraphStyle->SetTitleBorderSize(1);
//   fGraphStyle->SetOptFit(0);

}

