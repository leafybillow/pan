//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaPanamAna.cc  (implementation)
//
// Author: A. Vacheret <http://www.jlab.org/~vacheret>
//
////////////////////////////////////////////////////////////////////////
//
//    Online Panaming analysis.  This class derives from VaAnalysis.
//    It simply displays plots defined in the list to the screen.  
//     
//    
//
////////////////////////////////////////////////////////////////////////

#include "TaPanamAna.hh"
#include "TaEvent.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"
#include "VaPair.hh"
//#define DEBUGDEV
//#define DEBUGBOX

#define ADCSTACK

ClassImp(TaPanamAna)
// Constructors/destructors/operators

  TaPanamAna::TaPanamAna():VaAnalysis(),
  fLumiADC(0),
  fLumiADCRMS(0),
  fLumiAsy(0), 
  fLumiAsyRMS(0), 
  fLumiADCBCMDiffAsy(0), 
  fLumiADCBCMDiffAsyRMS(0), 
  fDaltonADC(0), 
  fDaltonADCRMS(0), 
  fDaltonBCMNormADC(0),
  fDaltonBCMNormADCRMS(0),
  fDaltonBCMNormAsy(0),  
  fDaltonBCMNormAsyRMS(0), 
  fLimits(0),fADCStackSCorH(kFALSE)
{
  fArrayOfDataName.clear();
  fMonDev.clear();
  fMonADev.clear();
  //  fMultiDev.clear(); 
  fADC_count=0;  
  fBCM_count=0;
  fBPM_count=0;
  fDoSlice = kFALSE;
  fDoRun = kFALSE;
  fDoRoot =kFALSE;
  fADCStackSCorH =kFALSE;
}

TaPanamAna::~TaPanamAna()
{
  delete fLumiADC; delete fLumiADCRMS;
  delete fLumiAsy; delete fLumiAsyRMS;
  delete fLumiADCBCMDiffAsy; delete fLumiADCBCMDiffAsyRMS;
  delete fDaltonADC; delete fDaltonADCRMS; 
  delete fDaltonBCMNormADC; delete fDaltonBCMNormADCRMS;
  delete fDaltonBCMNormAsy; delete fDaltonBCMNormAsyRMS;
  delete fLimits;

  fArrayOfDataName.clear();
  fMonDev.clear();
  fMonADev.clear();
}

TaPanamAna::TaPanamAna (const TaPanamAna& copy) 
{
}


TaPanamAna& TaPanamAna::operator=( const TaPanamAna& assign) 
{ 
  return *this; 
}

string TaPanamAna::itos(Int_t i)
{
  ostringstream buffer;
  buffer<<i;
  return buffer.str();
}
vector<string>
TaPanamAna::GetHistoForListBox() const
{
  vector<string> boxlist;
  boxlist.clear();
  for (vector<TaPanamDevice*>::const_iterator i= fMonDev.begin(); i != fMonDev.end(); i++)
    {
     boxlist.push_back(string((*i)->GetSData()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetSDataRMS()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetHData()->GetName())); 
    }
  for (vector<TaPanamADevice*>::const_iterator i= fMonADev.begin(); i != fMonADev.end(); i++)
    {
     boxlist.push_back(string((*i)->GetSData()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetSDataRMS()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetSAData()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetSADataRMS()->GetSCHist()->GetName())); 
     boxlist.push_back(string((*i)->GetHData()->GetName())); 
     boxlist.push_back(string((*i)->GetHAData()->GetName())); 
    }
 
#ifdef DEBUGBOX
  for (vector<string>::const_iterator s= boxlist.begin(); s != boxlist.end(); s++)
    { 
     cout<<"boxlist name="<<*s<<endl;
    }
#endif
  return boxlist;
}

void 
TaPanamAna::InitMonitorDevices()
{  
  fADC_color[0] = 2; fADC_color[1] = 3;
  fADC_color[2] = 4;fADC_color[3] = 6;
  fIsADC = kFALSE;
  fIsLumi = kFALSE;
  fIsDalton = kFALSE;
  fLastADCName="";
  
  cout<<"------------------------Entering histos initialization------------------------"<<endl;

#ifdef DEBUGDEV
  cout<<" fMondev size= "<<fMonDev.size()<<endl;
  cout<<" *** Name of device and plots  "<<endl;
#endif  
  // clear vectors
  if ( fMonDev.size() != 0)  fMonDev.clear();
  if ( fMonADev.size() != 0)  fMonADev.clear();
  if ( fSTADC.size() != 0)  fSTADC.clear();
  if ( fLumiGraph.size() != 0)fLumiGraph.clear();
  if ( fDaltonGraph.size() != 0) fDaltonGraph.clear();  
  if ( fLumiDev.size() != 0)fLumiDev.clear();  
  if ( fDaltonDev.size() != 0)fDaltonDev.clear();  

  for (vector<string>::const_iterator ci= fArrayOfDataName.begin(); 
                                      ci!= fArrayOfDataName.end(); 
                                      ci++)
     {
      if (!strncmp((*ci).c_str(),"heli",4) || !strncmp((*ci).c_str(),"pair",4)  || 
          !strncmp((*ci).c_str(),"quad",4) || !strncmp((*ci).c_str(),"times",5) || 
          !strncmp((*ci).c_str(),"v2f",3)  || !strncmp((*ci).c_str(),"heli",4)) 
        {
         TaPanamDevice* md = new TaPanamDevice((char*)(*ci).c_str(),fRun->GetKey((char*)(*ci).c_str()),
                                               striptime,60,100,0.,(Float_t) striptime,-2,2,0);
         fMonDev.push_back(md);
	}
      if (!strncmp((*ci).c_str(),"adc",3)) 
        {
         TaPanamADevice* md = new TaPanamADevice((char*)(*ci).c_str(),
                                                 fRun->GetKey((char*)(*ci).c_str()),
                                                 striptime,60,striptime,60,
                                                 200,
                                                 0.,(Float_t) striptime,
                                                 0.,65535.,
                                                 100,
                                                 0.,(Float_t) striptime,
                                                 -500,500,
                                                 0,2,kFALSE);
         fMonADev.push_back(md);
         if (!strncmp((md->GetName()),"adc",3)) fIsADC=kTRUE;
	}
     if (!strncmp((*ci).c_str(),"bcm",3) || !strncmp((*ci).c_str(),"det",3) || 
         !strncmp((*ci).c_str(),"bpminws",7)) 
        {
         TaPanamADevice* md = new TaPanamADevice((char*)(*ci).c_str(),
                                                 fRun->GetKey((char*)(*ci).c_str()),
                                                 striptime,60,striptime,60,
                                                 100,
                                                 0.,(Float_t) striptime,
                                                 0.,65535.,
                                                 200,
                                                 0.,(Float_t) striptime,
                                                 -5000,5000,
                                                 0,2,kFALSE);
         fMonADev.push_back(md);
         if (!strncmp((md->GetName()),"det",3)) fDaltonDev.push_back(md);
	}
      if (!strncmp((*ci).c_str(),"lumi",4)) 
        {
         TaPanamADevice* md = new TaPanamADevice((char*)(*ci).c_str(),
                                                 fRun->GetKey((char*)(*ci).c_str()),
                                                 striptime,60,striptime,60,
                                                 100,
                                                 0.,(Float_t) striptime,
                                                 0.,65535.,
                                                 200,
                                                 0.,(Float_t) striptime,
                                                 -10000,10000,
                                                 0,2,kFALSE);
         fMonADev.push_back(md);
         fLumiDev.push_back(md);
	}
      if (!strncmp((*ci).c_str(),"bpm",3)) 
        {
         TaPanamADevice* md = new TaPanamADevice((char*)(*ci).c_str(),
                                                 fRun->GetKey((char*)(*ci).c_str()),
                                                 striptime,60,striptime,60,
                                                 100,
                                                 0.,(Float_t) striptime,
                                                 0.,65535.,
                                                 200,
                                                 0.,(Float_t) striptime,
                                                 -500,500,
                                                 0,2,kTRUE);
         fMonADev.push_back(md);
	}
      }

#ifdef DEBUGDEV

  cout<<" === === === debug monitor list === ==== ==="<<endl; 
 for (vector<TaPanamDevice*>::const_iterator md  = fMonDev.begin(); md!= fMonDev.end(); md++)
   {
    cout<<"\ndevice name="<<(*md)->GetName()<<" string="<<string((*md)->GetName())<<endl;
    cout<<"fSData name="<<(*md)->GetSData()->GetName()<<" string="<<
          string((*md)->GetSData()->GetName())<<endl;
    cout<<"fSData TH1D name="<<(*md)->GetSData()->GetSCHist()->GetName()<<" string="<<
          string((*md)->GetSData()->GetSCHist()->GetName())<<endl;
    cout<<"fSDataRMS name="<<(*md)->GetSDataRMS()->GetName()<<" string="<<string((*md)->GetSDataRMS()->GetName())<<endl;
    cout<<"fSDataRMS TH1D name="<<(*md)->GetSDataRMS()->GetSCHist()->GetName()<<" string="<<string((*md)->GetSDataRMS()->GetSCHist()->GetName())<<endl;
    cout<<"fHData name="<<(*md)->GetHData()->GetName()<<" string="<<string((*md)->GetHData()->GetName())<<endl;
   }

 for (vector<TaPanamADevice*>::const_iterator md  = fMonADev.begin(); 
                                      md!= fMonADev.end(); 
                                      md++)
   {
    cout<<"\ndevice name="<<(*md)->GetName()<<" string="<<string((*md)->GetName())<<endl;
    cout<<"fSData name="<<(*md)->GetSData()->GetName()<<" string="<<
          string((*md)->GetSData()->GetName())<<endl;
    cout<<"fSData TH1D name="<<(*md)->GetSData()->GetSCHist()->GetName()<<" string="<<
    cout<<"fSAData name="<<(*md)->GetSAData()->GetName()<<" string="<<
          string((*md)->GetSAData()->GetName())<<endl;
    cout<<"fSAData TH1D name="<<(*md)->GetSAData()->GetSCHist()->GetName()<<" string="<<
          string((*md)->GetSAData()->GetSCHist()->GetName())<<endl;
    cout<<"fSDataRMS name="<<(*md)->GetSDataRMS()->GetName()<<" string="<<
          string((*md)->GetSDataRMS()->GetName())<<endl;
    cout<<"fSDataRMS TH1D name="<<(*md)->GetSDataRMS()->GetSCHist()->GetName()<<" string="<<
          string((*md)->GetSDataRMS()->GetSCHist()->GetName())<<endl;
    cout<<"fSADataRMS name="<<(*md)->GetSADataRMS()->GetName()<<" string="<<
          string((*md)->GetSADataRMS()->GetName())<<endl;
    cout<<"fSADataRMS TH1D name="<<(*md)->GetSADataRMS()->GetSCHist()->GetName()<<" string="<<
          string((*md)->GetSADataRMS()->GetSCHist()->GetName())<<endl;
    cout<<"fHData name="<<(*md)->GetHData()->GetName()<<" string="<<
          string((*md)->GetHData()->GetName())<<endl;
    cout<<"fHAData name="<<(*md)->GetHAData()->GetName()<<" string="<<
          string((*md)->GetHAData()->GetName())<<endl;
  }
 cout<<"=== === ===  debug complete === === ==="<<endl;

#endif  

#ifdef ADCSTACK
  Char_t* devname;
  Char_t* splitstr;
  Char_t* channum;
  Int_t   numchan=0; 
 
  if (fIsADC)
    {
     for (vector<TaPanamADevice*>::const_iterator didx= fMonADev.begin(); didx != fMonADev.end(); didx++)
       {
	if (!strncmp((*didx)->GetName(),"adc",3))
	  {
           splitstr= (Char_t*) (*didx)->GetName();
           //cout<<"ADC init  splitstr="<<splitstr<<endl;
           devname=strtok(splitstr,"_");
	   //           devname=strtok(NULL,"_");
           channum= strtok(NULL,"_ ");
           //cout<<"ADC init  devname="<<devname<<endl;
           //cout<<"last adc name "<<fLastADCName<<endl;   
           //cout<<"ADC channum="<<channum<<endl;
           numchan=atoi(channum);
           // create new stack if first encounter of adc number else just add channel in existing stack
           // the channel should not been declared two times in the list....
           //cout<<"fSTADC.size()="<<fSTADC.size()<<endl;           
           (*didx)->GetSData()->GetSCHist()->SetLineColor(fADC_color[numchan]); 
           (*didx)->GetHData()->SetLineColor(fADC_color[numchan]); 
           if (strcmp(devname,fLastADCName))
             {
	      fLastADCName=devname; 
              THStack *ast = new THStack(devname,devname);
              //cout<<"Stack : "<<devname<<" initialized 2 \n";
              //cout<<" asociated to dev "<<(*didx)->GetName()<<endl;
              //cout<<" color setting="<<fADC_color[numchan]<<endl;
              if (!fADCStackSCorH) ast->Add((*didx)->GetSData()->GetSCHist());
              else ast->Add((*didx)->GetHData());
              ast->Print();
              fSTADC.push_back(ast);
	      //cout<<"new stack list size="<<fSTADC.size()<<endl;
	     }
	   else
	     {
	      Int_t laststack=fSTADC.size()-1;
 	      if (!fADCStackSCorH) fSTADC[laststack]->Add((*didx)->GetSData()->GetSCHist());
              else fSTADC[laststack]->Add((*didx)->GetHData());
	     }
	  }
       }
         // summary of histos in stacks.  
    for (vector<THStack*>::const_iterator sdx= fSTADC.begin(); sdx != fSTADC.end(); sdx++)
      {
       cout<<(*sdx)->GetName()<<" stack list  of histos : "<<endl;
       (*sdx)->Print();
      }
    }
#endif

  if (fLumiDev.size() > 0) 
    {
     fIsLumi=kTRUE;
     fLumiADC              = new TGraph();
     fLumiADC->SetTitle("Lumi ADC Mean");
     fLumiGraph.push_back(fLumiADC);
     fLumiADCRMS           = new TGraph();
     fLumiADCRMS->SetTitle("Lumi ADC RMS");
     fLumiGraph.push_back(fLumiADCRMS);
     fLumiAsy              = new TGraph();
     fLumiAsy->SetTitle("Lumi Asymmetries");
     fLumiGraph.push_back(fLumiAsy);
     fLumiAsyRMS           = new TGraph();
     fLumiAsyRMS->SetTitle("Lumi Asymmetries RMS");
     fLumiGraph.push_back(fLumiAsyRMS);
     fLumiADCBCMDiffAsy    = new TGraph();
     fLumiADCBCMDiffAsy->SetTitle("Lumi BCM substracted Asymmetries");
     fLumiGraph.push_back(fLumiADCBCMDiffAsy);
     fLumiADCBCMDiffAsyRMS = new TGraph();
     fLumiADCBCMDiffAsyRMS->SetTitle("LumiBCM substracted  Asymmetries RMS");
     fLumiGraph.push_back(fLumiADCBCMDiffAsyRMS);
     for (Int_t i=0;i<numlumi;i++)
       {
	 LumiChan[i]= i+1;
       }
    }
//   cout<<" size of lumigraph vector"<<fLumiGraph.size()<<endl;
      for (Int_t i=0;i<numdalton;i++)
        {
 	 Daltons[i]= i+1;
//          DaltonsADC[i]= 5000*i;
//          DaltonsADCRMS[i]= 300*i;        
//          cout<<" dalton de i "<<Daltons[i]<<endl;
//          cout<<" dalton de i "<<DaltonsADC[i]<<endl;
//          cout<<" dalton de i "<<DaltonsADCRMS[i]<<endl;  
        }
  if (fDaltonDev.size() > 0)
    { 
     fIsDalton=kTRUE;
     fDaltonADC            = new TGraph();
     fDaltonADC->SetTitle("Daltons ADC");
     fDaltonGraph.push_back(fDaltonADC);
     fDaltonADCRMS         = new TGraph();
     fDaltonADCRMS->SetTitle("Daltons ADC RMS");
     fDaltonGraph.push_back(fDaltonADCRMS);
     fDaltonAsy            = new TGraph();
     fDaltonAsy->SetTitle("Daltons Asymmetries");
     fDaltonGraph.push_back(fDaltonAsy);
     fDaltonAsyRMS         = new TGraph();
     fDaltonAsyRMS->SetTitle("Daltons Asymmetries RMS");
     fDaltonGraph.push_back(fDaltonAsyRMS);
     fDaltonBCMNormADC     = new TGraph();
     fDaltonBCMNormADC->SetTitle("Daltons BCM normalized ADC");
     fDaltonGraph.push_back(fDaltonBCMNormADC);
     fDaltonBCMNormADCRMS  = new TGraph();
     fDaltonBCMNormADCRMS->SetTitle("Daltons BCM normalized ADC RMS");
     fDaltonGraph.push_back(fDaltonBCMNormADCRMS);
     fDaltonBCMNormAsy     = new TGraph();
     fDaltonBCMNormAsy->SetTitle("Daltons BCM normalized Asymmetries");
     fDaltonGraph.push_back(fDaltonBCMNormAsy);
     fDaltonBCMNormAsyRMS  = new TGraph();
     fDaltonBCMNormAsyRMS->SetTitle("Daltons BCM normalized Asymmetries RMS");
     fDaltonGraph.push_back(fDaltonBCMNormAsyRMS);
     
     }
  if (fIsLumi || fIsDalton) fLimits = new TH2D("","",2,0,5,2,-0.01,2.0);

 cout<<"---------------------Initialization complete ! ----------------\n"<<endl;
}

void
TaPanamAna::DefineADCStacks(Bool_t opt)
{
#ifdef ADCSTACK
  Char_t* devname;
  Char_t* splitstr;
  Char_t* channum;
  Int_t   numchan=0; 
 
  if (fIsADC)
    {
     for (vector<TaPanamADevice*>::const_iterator didx= fMonADev.begin(); didx != fMonADev.end(); didx++)
       {
	if (!strncmp((*didx)->GetName(),"adc",3))
	  {
           splitstr= (Char_t*) (*didx)->GetName();
           //cout<<"ADC init  splitstr="<<splitstr<<endl;
           devname=strtok(splitstr,"_");
	   //           devname=strtok(NULL,"_");
           channum= strtok(NULL,"_ ");
           //cout<<"ADC init  devname="<<devname<<endl;
           //cout<<"last adc name "<<fLastADCName<<endl;   
           //cout<<"ADC channum="<<channum<<endl;
           numchan=atoi(channum);
           // create new stack if first encounter of adc number else just add channel in existing stack
           // the channel should not been declared two times in the list....
           //cout<<"fSTADC.size()="<<fSTADC.size()<<endl;           
           (*didx)->GetSData()->GetSCHist()->SetLineColor(fADC_color[numchan]); 
           (*didx)->GetHData()->SetLineColor(fADC_color[numchan]); 
           if (strcmp(devname,fLastADCName))
             {
	      fLastADCName=devname; 
              THStack *ast = new THStack(devname,devname);
              //cout<<"Stack : "<<devname<<" initialized 2 \n";
              //cout<<" asociated to dev "<<(*didx)->GetName()<<endl;
              //cout<<" color setting="<<fADC_color[numchan]<<endl;
              if (!opt) ast->Add((*didx)->GetSData()->GetSCHist());
              else ast->Add((*didx)->GetHData());
              ast->Print();
              fSTADC.push_back(ast);
	      //cout<<"new stack list size="<<fSTADC.size()<<endl;
	     }
	   else
	     {
	      Int_t laststack=fSTADC.size()-1;
 	      if (!opt) fSTADC[laststack]->Add((*didx)->GetSData()->GetSCHist());
              else fSTADC[laststack]->Add((*didx)->GetHData());
	     }
	  }
       }
         // summary of histos in stacks.  
    for (vector<THStack*>::const_iterator sdx= fSTADC.begin(); sdx != fSTADC.end(); sdx++)
      {
       cout<<(*sdx)->GetName()<<" stack list  of histos : "<<endl;
       (*sdx)->Print();
      }
    }
#endif
}


void
TaPanamAna::FillEventPlots()
{ 
  for (vector<TaPanamDevice*>::const_iterator i  = fMonDev.begin(); 
                                              i != fMonDev.end(); 
                                              i++) 
      (*i)->FillFromEvent(*fRun);
  
  for (vector<TaPanamADevice*>::const_iterator i  = fMonADev.begin(); 
                                              i != fMonADev.end(); 
                                              i++) 
      (*i)->FillFromEvent(*fRun);  

 if (fIsLumi)
    {// fill Lumi graph if Lumi channels are defined
         for (Int_t lu =0; lu< numlumi; lu++)
           {
            LumiADC[lu]     = fLumiDev[lu]->GetSData()->GetSCHist()->GetBinContent(striptime-1);
            LumiADCRMS[lu]  = fLumiDev[lu]->GetSDataRMS()->GetSCHist()->GetBinContent(striptime-1);
           }
    }
  if (fIsDalton)
    {// fill Dalton graph if Dalton channel defined
     for (Int_t dn =0; dn< numdalton; dn++)
       {
	DaltonsADC[dn]     = fDaltonDev[dn]->GetSData()->GetSCHist()->GetBinContent(striptime-1);
	DaltonsADCRMS[dn]  = fDaltonDev[dn]->GetSDataRMS()->GetSCHist()->GetBinContent(striptime-1);             
       }
     cout<<" dalton ADC value "<<DaltonsADC[0]<<endl;
     cout<<" dalton ADC RMS value "<<DaltonsADCRMS[0]<<endl;
    }

}

void 
TaPanamAna::FillPairPlots()
{
for (vector<TaPanamADevice*>::const_iterator i  = fMonADev.begin(); 
                                             i != fMonADev.end(); 
                                             i++)
       (*i)->FillFromPair(*fPair);

  if (fIsLumi)
    {
      for (Int_t lu =0; lu< numlumi; lu++)
        {
	  cout<<" LUMI DATA : L"<<lu<<"="<<
	    fLumiDev[lu]->GetSAData()->GetSCHist()->GetBinContent(striptime-1)<<endl;
         LumiAsy[lu]     = fLumiDev[lu]->GetSAData()->GetSCHist()->GetBinContent(striptime-1);
         LumiAsyRMS[lu]  = fLumiDev[lu]->GetSADataRMS()->GetSCHist()->GetBinContent(striptime-1);
        }
     }
  if (fIsDalton)
    {
     for (Int_t dn =0; dn< numdalton; dn++)
       {
	cout<<" DALTON DATA : L"<<dn<<"="<<
	  fDaltonDev[dn]->GetSAData()->GetSCHist()->GetBinContent(striptime-1)<<endl;
        DaltonsAsy[dn]     = fDaltonDev[dn]->GetSAData()->GetSCHist()->GetBinContent(striptime-1);
        DaltonsAsyRMS[dn]  = fDaltonDev[dn]->GetSAData()->GetSCHist()->GetBinContent(striptime-1);
       }
     }
}

void TaPanamAna::InitDevicePad(Int_t devidx, UInt_t plotidx, Bool_t choice)
{
  if (!choice) {
    cout<<"initscpad"<<endl;
   fMonDev[devidx]->InitSCPad(plotidx);
  }
 else fMonDev[devidx]->DrawHPad();
}

void TaPanamAna::InitADevicePad(Int_t devidx, UInt_t plotidx, Bool_t choice)
{
 if (!choice) fMonADev[devidx]->InitSCPad(plotidx);
 else fMonADev[devidx]->DrawHPad(plotidx);
}

void TaPanamAna::DisplayDevice(Int_t devidx,UInt_t plotidx, Bool_t choice)
{
 if ( !choice) fMonDev[devidx]->DisplaySC(plotidx);
 else fMonDev[devidx]->DrawHPad();
}

void TaPanamAna::DisplayADevice(Int_t devidx,UInt_t plotidx, Bool_t choice)
{
 if ( !choice) fMonADev[devidx]->DisplaySC(plotidx);
 else fMonADev[devidx]->DrawHPad(plotidx);
}

void TaPanamAna::InitADCStack(Int_t idx)
{
 fSTADC[idx]->Draw("nostack");
}

void TaPanamAna::DrawADCStack(Int_t idx)
{
 fSTADC[idx]->Draw("nostack");
}

void TaPanamAna::InitLumiGraph(Int_t idx)
{
  fLimits->Draw();
  fLumiGraph[idx]->SetMarkerStyle(20);
  fLumiGraph[idx]->SetMarkerSize(0.5);
  fLumiGraph[idx]->SetMarkerColor(2);  
   switch (idx) 
      {
  case 0:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
    break; 
  case 1:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCRMS,"P");
    break; 
  case 2:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiAsy,"P");
    break; 
  case 3:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiAsyRMS,"P");
    break; 
  case 4:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCBCMDiffAsy,"P");
    break; 
  case 5:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCBCMDiffAsyRMS,"P");
    break; 
  case 6:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
    break; 
  case 7:
    fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
    break; 

 }
}

void TaPanamAna::DrawLumiGraph(Int_t idx)
{
   switch (idx) 
     {
      case 0:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
       break; 
      case 1:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCRMS,"P");
       break; 
      case 2:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiAsy,"P");
       break; 
      case 3:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiAsyRMS,"P");
       break; 
      case 4:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCBCMDiffAsy,"P");
       break; 
      case 5:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADCBCMDiffAsyRMS,"P");
       break; 
      case 6:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
       break; 
      case 7:
       fLumiGraph[idx]->DrawGraph(numlumi,LumiChan,LumiADC,"P");
       break; 
     }
}

void TaPanamAna::InitDaltonGraph(Int_t idx)
{
 fLimits->Draw(); 
  fDaltonGraph[idx]->SetMarkerStyle(20);
  fDaltonGraph[idx]->SetMarkerColor(9);  
  fDaltonGraph[idx]->SetMarkerSize(0.5); 
   switch (idx) 
     {
      case 0:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsADC,"P");
       break; 
      case 1:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsADCRMS,"P");
       break; 
      case 2:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsAsy,"P");
       break; 
      case 3:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsAsyRMS,"P");
       break; 
      case 4:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormADC,"P");
       break; 
      case 5:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormADCRMS,"P");
       break; 
      case 6:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormAsy,"P");
       break; 
      case 7:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormAsyRMS,"P");
       break; 
      }
}

void TaPanamAna::DrawDaltonGraph(Int_t idx)
{
   switch (idx) 
     {
      case 0:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsADC,"P");
       break; 
      case 1:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsADCRMS,"P");
       break; 
      case 2:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsAsy,"P");
       break; 
      case 3:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsAsyRMS,"P");
       break; 
      case 4:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormADC,"P");
       break; 
      case 5:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormADCRMS,"P");
       break; 
      case 6:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormAsy,"P");
       break; 
      case 7:
       fDaltonGraph[idx]->DrawGraph(numdalton,Daltons,DaltonsBCMNormAsyRMS,"P");
       break; 
      }
}

void
TaPanamAna::EventAnalysis()
{
  // Event analysis.
  // For now we hard code four results, the values of bcm1 and bcm2
  // (raw and corrected).  A somewhat more intelligent analysis 
  // should be concocted eventually.

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
  FillEventPlots();       
}

void 
TaPanamAna::PairAnalysis()
{ 
  // Pair analysis
  // All we have here is a call to AutoPairAna.
  AutoPairAna();
  FillPairPlots();
}


void
TaPanamAna::InitChanLists ()
{
  InitMonitorDevices();
}
void
TaPanamAna::InitDevicesList(vector<string> arrayofname)
{
  cout<<"\n ---- Copy of the dataname wanted for monitoring ----\n"<<endl;
  fArrayOfDataName = arrayofname;
  for (vector<string>::const_iterator i= fArrayOfDataName.begin(); i!= fArrayOfDataName.end(); i++)
  cout<<" key name : "<<*i<<endl;
}

TaPanamDevice* 
TaPanamAna::GetPanamDevice(Char_t* const devname) const
{
   TaPanamDevice* thedev;
   for (vector<TaPanamDevice*>::const_iterator i= fMonDev.begin(); i != fMonDev.end(); i++)
     {
       if ((Char_t*) (*i)->GetName() == devname) { thedev=(*i);} 
     }
   return thedev;
}

TaPanamADevice* 
TaPanamAna::GetPanamADevice(Char_t* const devname) const
{
   TaPanamADevice* theAdev;
   for (vector<TaPanamADevice*>::const_iterator i= fMonADev.begin(); i != fMonADev.end(); i++)
     {
       if ((Char_t*) (*i)->GetName() == devname) { theAdev=(*i);} 
     }
   return theAdev;
}
