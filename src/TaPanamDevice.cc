//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan (Online Monitor version)           
//
//        TaPanamDevice.cc   (implementation file)
//        ^^^^^^^^^^^^^^^
//    Authors :  A. Vacheret
//
//    List of histos and stripchart for the corresponding Pan device in 
//    the datamap. 
//
//////////////////////////////////////////////////////////////////////////

#include "TaPanamDevice.hh"
#include "VaPair.hh"
#include "TaEvent.hh"

ClassImp(TaPanamDevice)

TaPanamDevice::TaPanamDevice():
 fSData(0),
 fSDataRMS(0), 
 fHData(0),
 fName(0),fDevicekey(0),
 fSNumOfChan(0),
 fSNumOfEvPerChan(0),
 fHbins(0),
 fColor(0),
 fXSmin(0),fXSmax(0),
 fXHmin(0),fXHmax(0)
{}

TaPanamDevice::TaPanamDevice(char* name, Int_t namekey,
                                 Int_t SNumChan, Int_t SNumEvperChan,
                                 Int_t histobin, 
                                 Axis_t xsmin, Axis_t xsmax, 
                                 Axis_t xhmin, Axis_t xhmax, 
                                 Int_t color):
 fSData(0),
 fSDataRMS(0), 
 fHData(0)
{
  fSCArray.clear();
  fName      = name;
  fDevicekey = namekey;
  fSNumOfChan      = SNumChan;
  fSNumOfEvPerChan = SNumEvperChan;
  fHbins   = histobin;
  fXSmin = xsmin;
  fXSmax = xsmax;
  fXHmin = xhmin;
  fXHmax = xhmax;
  fColor = color;
  Init();
}

TaPanamDevice::~TaPanamDevice()
{
   delete fSData; 
   delete fSDataRMS; 
   delete fHData;
}

TaPanamDevice::TaPanamDevice(const TaPanamDevice& md)
{
  fDataVal               = md.fDataVal;
  fSCArray               = md.fSCArray;
  fName                  = md.fName;
  fDevicekey             = md.fDevicekey;
  fSNumOfChan            = md.fSNumOfChan;
  fSNumOfEvPerChan       = md.fSNumOfEvPerChan;
  fHbins                 = md.fHbins;
  fXSmin                 = md.fXSmin;
  fXSmax                 = md.fXSmax;
  fXHmin                 = md.fXHmin;
  fXHmax                 = md.fXHmax;
  fColor                 = md.fColor;
  if(md.fSData) 
     {         
      fSData   = new TaStripChart(md.fSData->GetName(),md.fSData->GetDataName(),
                              fSNumOfChan,fSNumOfEvPerChan,
                              fXSmin,fXSmax); 
      fSData  = md.fSData;
     } 
  else fSData = NULL;
  if(md.fSDataRMS) 
     {         
      fSDataRMS = new TaStripChart(md.fSDataRMS->GetName(),md.fSDataRMS->GetDataName(),
                                   fSNumOfChan,fSNumOfEvPerChan,
                                   fXSmin,fXSmax); 
      fSDataRMS  = md.fSDataRMS;
     } 
  else fSDataRMS = NULL;
  if(md.fHData) 
     {         
      fHData   = new TH1D(md.fHData->GetName(),md.fHData->GetTitle(),
                          md.fHbins,fXHmin,fXHmax); 
      cout<<" Histogram "<<fHData->GetName()<<" created"<<endl;
      fHData  = md.fHData;
     } 
  else fHData = NULL;
}

TaPanamDevice& TaPanamDevice::operator=(const TaPanamDevice &md)
{
if ( this != &md )
    {
     if(fSData)        delete fSData; 
     if(fSDataRMS)     delete fSDataRMS; 
     if(fHData)        delete fHData;
     fDataVal          = md.fDataVal;
     fSCArray          = md.fSCArray;
     fName             = md.fName;
     fDevicekey        = md.fDevicekey;
     fSNumOfChan       = md.fSNumOfChan;
     fSNumOfEvPerChan  = md.fSNumOfEvPerChan;
     fHbins            = md.fHbins;
     fXSmin            = md.fXSmin;
     fXSmax            = md.fXSmax;
     fXHmin            = md.fXHmin;
     fXHmax            = md.fXHmax;
     fColor            = md.fColor;
     if (md.fSData) 
       {         
        fSData   = new TaStripChart(md.fSData->GetName(),md.fSData->GetDataName(),
                                    fSNumOfChan,fSNumOfEvPerChan,
                                    fXSmin,fXSmax); 
        fSData  = md.fSData;
       } 
     else fSData = NULL;
     if (md.fSDataRMS) 
       {         
        fSDataRMS   = new TaStripChart(md.fSDataRMS->GetName(),md.fSDataRMS->GetDataName(),
                                       fSNumOfChan,fSNumOfEvPerChan,
                                       fXSmin,fXSmax); 
        fSDataRMS  = md.fSDataRMS;
       } 
     else fSDataRMS = NULL;
     if(md.fHData) 
       {         
        fHData   = new TH1D(md.fHData->GetName(),md.fHData->GetTitle(),
                            md.fHbins,fXHmax,fXHmax); 
        fHData  = md.fHData;
       } 
     else fHData = NULL;
     return *this;     
    }
 else
   {
     cout<<"warning ! TaPanamDevice assignment to self !"<<endl;
     return *this;
   }
}  

void 
TaPanamDevice::FillFromEvent(TaRun& run){
  if (fDevicekey !=0)
    {
      fDataVal = run.GetEvent().GetData(fDevicekey);
     if (fSData->DataSum(fDataVal))
       { 
    	fSData->AddBinVal(fSData->GetSumNorm());
        fSData->Fill();
       }
     fHData->Fill(fDataVal); 
     if (fSDataRMS->DataSum(fHData->GetRMS()))
       { 
	fSDataRMS->AddBinVal(fSDataRMS->GetSumNorm());
        fSDataRMS->Fill();
       }         
    }
}

void 
TaPanamDevice::InitSCPad(UInt_t plotidx)
{
  if (plotidx <=  (UInt_t) fSCArray.size()) 
    {
     cout<<" sc "<<fSCArray[plotidx]->GetSCHist()->GetName()<<" drawn init \n";
     fSCArray[plotidx]->SCDrawInit();
    }
  else cout<<" can't plot, index is bigger than array size!!! \n";
}

void 
TaPanamDevice::DisplaySC(UInt_t plotidx)
{
  if (plotidx <= (UInt_t)fSCArray.size()) 
    { 
     fSCArray[plotidx]->SCDraw();
     //     cout<<" strip of "<<fName<<" drawn \n";
    }
  else cout<<" can't plot, index is bigger than array size!!! \n";
}
void 
TaPanamDevice::DrawHPad()
{
 fHData->Draw();
}

void
TaPanamDevice::Init()
{
 string dname;
 dname = string("S_")+string(fName); 
 fSData    = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                              fSNumOfChan,fSNumOfEvPerChan,
                              fXSmin,fXSmax);   
 fSCArray.push_back(fSData); 

 dname = string("S_")+string(fName) + string("_RMS");
 fSDataRMS = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                              fSNumOfChan,fSNumOfEvPerChan,
                              fXSmin,fXSmax);
 fSCArray.push_back(fSDataRMS); 

 dname = string("H_")+string(fName); 
 fHData    = new TH1D((char*) dname.c_str(), (char*) dname.c_str(),
                      fHbins,fXHmin,fXHmax); 
}

TH1D*
TaPanamDevice::GetPlot(char* const plotname, Int_t const plottype) const 
{
  TH1D* theplot=NULL;
  if (plottype ==1)
    {
      for (vector<TaStripChart*>::const_iterator sc=fSCArray.begin();sc!=fSCArray.end();sc++)
	{
	  if ((*sc)->GetSCHist()->GetName() == plotname) 
            {
             theplot = (*sc)->GetSCHist();  
	    }
          if (theplot!=NULL) break;
        }
    }
  if(plottype ==2) theplot = fHData;   
  return theplot;
}
