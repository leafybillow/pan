#ifndef __TaPanam__
#define __TaPanam__
//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan
//
//           TaPanam.hh
//
// Author:  R. Holmes <http://mepserv.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>
// @(#)pan/src:$Name$:$Id$//
//
////////////////////////////////////////////////////////////////////////
//
//  Graphical user interface using Pan for monitoring purposes and specific 
//  analysis quick launch. Inspiration for layout  
//  is from ALICE testbeam monitoring and ROOT test example.   
//
////////////////////////////////////////////////////////////////////////

#define DEBUG 1

// STD includes 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

// ROOT includes
#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TGFrame.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGClient.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
//#include <TGRadioButton.h>
//#include <TGCheckButton.h>
#include <TGTextEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
//#include <TGSlider.h>
//#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h> 
#include <TRandom.h>
//#include <TEnv.h>
#include <TGStatusBar.h>
#include <TGWidget.h>
#include <TFile.h>
#include <TKey.h>
#include <TTimer.h>
#include <TQObject.h> 
#include "TaThread.hh"
#include <TCondition.h>
#include <TMutex.h>
#include <TCanvas.h>
#include <TStyle.h>
// Pan includes 
#include "TaAnalysisManager.hh"

enum ETestCommandIdentifiers {
  M_FILE_OPEN,
  M_FILE_EDITDB,
  M_FILE_PRINT,
  M_FILE_PRINT1,
  M_FILE_SAVE,
  M_FILE_SAVE1,
  M_FILE_SAVE2,
  M_FILE_EXIT,

  M_MON_PRESTART,
  M_MON_START,
  M_MON_STOP,
  M_MON_END,
  M_MON_HISTOALLRAZ,
 
  M_ANA_PROMPT,
  M_ANA_REGRESS,
  M_ANA_BEAMMOD,
};


class TaPanam : public TGMainFrame, public TaThread {
	      //, public TaCanvasWatchDog {

public:

  enum NumOfChan { maxadc = 2, maxchanadc = 4};

  TaPanam();
  TaPanam(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~TaPanam();
  // GUI functions                     
  virtual   void         CloseWindow();
  Int_t                  CheckDevListConfig();
  Int_t                  CheckOptionConfig();
  Int_t                  InitFlags(); 
  Int_t                  CheckCanvasConfig();
  virtual Bool_t         ProcessMessage(Long_t msg, Long_t parm1, Long_t);
  Int_t                  HandleCheckButton(Long_t parm1);
  Int_t                  HandleMenuBar(Long_t parm1);
  Int_t                  HandleListBox(Long_t parm1);
  void                   UpdateEmCanvas();
  Int_t                  InitParameters();              // initialization of parameters
  Int_t                  InitCanvasFlock();                 // initialization of Canvas list 
  Int_t                  IsDeviceName(string devname, Int_t devtype) const;   // check if device name defined
  void                   AddDeviceName(string devname, Int_t devtype);    // add device name in devlist
  void                   RemoveDeviceName(string devname, Int_t devtype);    // Remove device name in devlist
  Int_t                   ResizeDevList(Int_t devtype);    // Shrink dev list 
  void                   UpdateDevListWithButton(TGCheckButton *thebutton, string devname, Int_t devtype);
  void                   UpdateADCListWithButton(TGCheckButton *thebutton, string devname);
  void                   UpdateBPMListWithButton(TGCheckButton *thebutton, string devname, Int_t devtype);
  void                   DumpDevList(Int_t devtype);
  void                   CheckChoice2(TGCheckButton *thebutton1,TGCheckButton *thebutton2, Bool_t flag);
  void                   CheckChoice3(TGCheckButton *thebutton1,TGCheckButton *thebutton2,TGCheckButton *thebutton3,Int_t flag);
  Int_t                   InitListOfAllPlots();                // initialization of device list
  string                 itos(Int_t i);                    // integer to string converter
  Int_t                  InitCanvasPads();                 // Initialization of canvas pads
  Int_t                  UpdatePads();
  void                   InitADCStacks();
  void                   InitGraphs(Int_t dettype);
  void                   UpdateADCStacks();
  void                   UpdateGraphs(Int_t dettype);
  void                   CountDataInList();                // count data if defined
  void                   DumpOptionState();
  void                   CanvasSafeDelete(TCanvas* can, char* name);                //
  Int_t                  UpdateCanvasFlock();                //
  void                   InitCanFDBK();                //
  void                   UpdateCanFDBK();                //
  void                   UpdateCanDITH();                //
  void                   ResetIdxVector();                //
  void                   UpdateCanvas(Char_t* title, vector<Int_t> idxvect, Int_t devtype, Bool_t disptype);                //
  void                   DisplayThis(TGCheckButton *thebutton,Bool_t showcan);
  Int_t                   InitHistoListBox(vector<string> thelist); // Get list of plots for lsitbox 
  void                   InitGUIFrames();                  // Initialization of the GUI 
  Int_t                  ClearCanvas();
  Int_t                  ClearListBox();
  Int_t                  ClearListOfDevice();                   
  // Thread related functions
  Int_t                  ThreadIsRunning();                // 
  void                   DoPan();                          //  Threaded  method start Pan analysis 
  void                   DoRefreshing();                   //  Threaded method  to refresh display 
  Int_t                   RefreshCanvas();                  //  Actual canvas update method
  void                   SetStyles();
 private: 
  // GUI frame
  TGCompositeFrame    *fMainTitle;                                   // 
  TGCompositeFrame    *fUtilFrame;                                   //
  TGCompositeFrame    *fHistoFrame;                                  //
  TGCompositeFrame    *fButtonFrame;                                 // 
  TGCompositeFrame    *fMainFrame;                                   //
  TGCompositeFrame    *fEmCanFrame;                                  //
  TGCompositeFrame    *fTimeFrame;                                   //
  TGCompositeFrame    *fADCScalerFrame;                              //
  TGCompositeFrame    *fADCFrame0;                              //
  TGCompositeFrame    *fADCFrame1;                              //
  TGCompositeFrame    *fADCFrame2;                              //
  TGCompositeFrame    *fADCFrame3;                              //
  TGCompositeFrame    *fCurrentFrame;                                       //
  TGCompositeFrame    *fInjBPMWSFrame;                                       //
  TGCompositeFrame    *fInjBPMWSDispFrame;                                       //
  TGCompositeFrame    *fInjBPMWSOptFrame;                                       //
  TGCompositeFrame    *fBCMFrame;                                       //
  TGCompositeFrame    *fBCMDFrame;                                       //
  TGCompositeFrame    *fBCMDispFrame;                                       //
  TGCompositeFrame    *fBCMOptFrame;                                       //
  TGCompositeFrame    *fBCMCavFrame;                                    //
  TGCompositeFrame    *fPositionFrame;                                       //
  TGCompositeFrame    *fInjBPMFrame;                                      //
  TGCompositeFrame    *fBPMFrame;                                       //
  TGCompositeFrame    *fBPMDispFrame;                                       //
  TGCompositeFrame    *fBPMDFrame;                                       //
  TGCompositeFrame    *fBPMOptFrame;                                       //
  TGCompositeFrame    *fBPMCavFrame;                                       //
  TGCompositeFrame    *fFeedbackFrame;                                       //
  TGCompositeFrame    *fFCurFrame;                                       //
  TGCompositeFrame    *fFPosFrame;
  TGCompositeFrame    *fDITHFrame;
  TGCompositeFrame    *fControlFrame;                                         //
  TGCompositeFrame    *fDetFrame;                               //
  TGCompositeFrame    *fDalFrame;                               //
  TGCompositeFrame    *fLumiFrame;                               //
  TGGroupFrame        *fTimegFrame;
  TGGroupFrame        *fTOptgFrame;
  TGGroupFrame        *fADCOptgFrame;
  TGGroupFrame        *fCurrentgFrame;
  TGGroupFrame        *fInjBPMWSgFrame;
  TGGroupFrame        *fBCMgFrame;
  TGGroupFrame        *fPositiongFrame;
  TGGroupFrame        *fBPMgFrame;
  TGGroupFrame        *fFCurgFrame;
  TGGroupFrame        *fFPosgFrame;
  TGGroupFrame        *fDITHgFrame;
  TGGroupFrame        *fDalgFrame;
  TGGroupFrame        *fLumigFrame;
  TGLabel             *fTitle;                                       // 
  TGLayoutHints       *fL1,*fL2,*fL3,*fL4,*fL5,*fL6,*fL7,*fL8,*fL9;            //
  TGPictureButton     *fLogoButton;                                  // 
  TGTextButton        *fButton1,*fButton2,*fButton3;                 //
  TGCheckButton       *fTimeCheck[5],*fTOptCheck[3];  
  TGCheckButton       *fADCOptCheck[7],*fADCCheck[10],*fIADCCheck[2],*fSADCCheck[2];
  TGCheckButton       *fInjBPMWSCheck[2]; 
  TGCheckButton       *fBCMCheck[3],*fBCMDCheck[2],*fBCMDispCheck[3],*fBCMOptCheck[2];
  TGCheckButton       *fBCMCavCheck[2];
  TGCheckButton       *fInjBPMCheck[2]
;  
  TGCheckButton       *fBPMCheck[6],*fBPMDispCheck[3],*fBPMDCheck[3],*fBPMOptCheck[2];
  TGCheckButton       *fBPMCavCheck[2];
  TGCheckButton       *fFCurCheck[2],*fFPosCheck,*fDITHCheck[2];              
  TGCheckButton       *fDalCheck[4],*fLumiCheck[4];
                
  TGCanvas            *fHistoTreeView;                                // 
  TRootEmbeddedCanvas *fEmCan;                                        //
  TGMenuBar           *fMenuBar;                                      // GUI menu bar widget
  TGLayoutHints       *fMenuBarLayout, *fMenuBarItemLayout;           // 
  TGPopupMenu         *fMenuFile, *fMenuMon, *fMenuAna;               // 
  TGListBox           *fHistoListBox;                                                   //
  TGTab               *fTab; 
  TCanvas             *fTimeCan,*fIADCCan,*fPADCCan,*fSADCCan;                                               //
  TCanvas             *fBCMCan,*fBCMCAVCan,*fBPMINCan,*fBPMCan;                                               //
  TCanvas             *fBPMCAVCan,*fFDBKCan,*fDITHCan;                                               //
  TCanvas             *fLUMICan,*fDETCan;                                               //
  TaAnalysisManager   *fAnaMan;   // Pan Analysis Manager 
  TStyle              *fStripStyle,*fHistoStyle,*fGraphStyle;  
  vector<string>      fDeviceList;
  vector<string>      fADeviceList;
  vector<string>      fAllDeviceList;
  vector<string>      fMultiDeviceList;
  vector<Int_t>       fTimeidx,fIADCidx,fPADCidx,fSADCidx;
  vector<Int_t>       fBCMidx,fBCMCAVidx,fBPMidx,fBPMINidx,fBPMCAVidx,fLUMIidx,fDETidx;
  Int_t               fColor[maxchanadc];                           //
  Int_t               fEntries;                           // 
  Int_t               fHistPadNum,fStripPadNum;            //
  Int_t               fTimeNum, fPADCNum, fIADCNum, fSADCNum;
  Int_t               fBCMNum, fBCMCAVNum; 
  Int_t               fBPMINNum, fBPMNum, fBPMCAVNum;
  Int_t               fLUMNum, fDETNum;
  Int_t               fBPMplot,fBCMplot;
  Bool_t              fPanThreadRun;    // Pan Thread flag 
  Bool_t              fShowTime, fShowPADC, fShowIADC, fShowSADC; // 
  Bool_t              fShowBCM,  fShowBCMCAV;
  Bool_t              fShowINBPM, fShowBPM,  fShowBPMCAV;
  Bool_t              fShowFDBK, fShowDITH;
  Bool_t              fShowLUMI, fShowDET;
  Bool_t              fFirstAna;
  Bool_t              fPrestart,fStart,fStop,fEnd;
  Bool_t              fADCdata,fPITA,fIA,fPZT;
  Bool_t              fTSCorH,fADCSCorH,fBCMSCorH,fBPMSCorH;  

  ClassDef(TaPanam, 1)   // 
};

#endif


