//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan (Online Monitor version)           
//
//        TaPanamADevice.cc   (implementation file)
//        ^^^^^^^^^^^^^^^
//    Authors :  A. Vacheret
//
//    List of histos and stripchart for the corresponding Pan device in 
//    the datamap with extra analyzed data (asym or diff). 
//
//////////////////////////////////////////////////////////////////////////

#include "TaPanamADevice.hh"
#include "VaPair.hh"
#include "TaEvent.hh"

ClassImp(TaPanamADevice)

 TaPanamADevice::TaPanamADevice():TaPanamDevice(),
 fSAData(0),
 fSADataRMS(0), 
 fHAData(0),
 fSANumOfChan(0),
 fSANumOfEvPerChan(0),
 fHAbins(0),
 fAColor(0),
 fXSAmin(0),fXSAmax(0),
  fXHAmin(0),fXHAmax(0),fWhichData(kFALSE)
{}
TaPanamADevice::TaPanamADevice(char* name, Int_t namekey, 
                               Int_t SNumChan, Int_t SNumEvperChan,
                               Int_t SANumChan, Int_t SANumEvperChan,
                               Int_t histobin,
                               Axis_t xsmin, Axis_t xsmax,
                               Axis_t xhmin, Axis_t xhmax, 
                               Int_t histoabin,  
                               Axis_t xsamin, Axis_t xsamax,
                               Axis_t xhamin, Axis_t xhamax, 
                               Int_t color, Int_t acolor, Bool_t datatype)
: TaPanamDevice(name,namekey,SNumChan,SNumEvperChan,histobin, 
                xsmin,xsmax,xhmin,xhmax,color),
 fSAData(0),
 fSADataRMS(0), 
 fHAData(0)
{
  fHArray.clear();
  fSANumOfChan      = SANumChan;
  fSANumOfEvPerChan = SANumEvperChan;
  fHAbins   = histoabin;
  fXSAmin = xsamin;
  fXSAmax = xsamax;
  fXHAmin = xhamin;
  fXHAmax = xhamax;
  fAColor = acolor;
  fWhichData = datatype;
  Init();
}  

TaPanamADevice::~TaPanamADevice()
{
   delete fSAData; 
   delete fSADataRMS; 
   delete fHAData;
}

TaPanamADevice::TaPanamADevice(const TaPanamADevice& md):TaPanamDevice(md)
{
  fADataVal          = md.fADataVal;          
  fHArray                = md.fHArray;
  fSANumOfChan           = md.fSANumOfChan;
  fSANumOfEvPerChan      = md.fSANumOfEvPerChan;
  fHAbins                = md.fHAbins;
  fXSAmin                = md.fXSAmin;
  fXSAmax                = md.fXSAmax;
  fXHAmin                = md.fXHAmin;
  fXHAmax                = md.fXHAmax;
  fAColor                = md.fAColor;
  fColor                 = md.fAColor;
  if(md.fSAData) 
     {         
      fSAData   = new TaStripChart(md.fSAData->GetName(),
                                   md.fSAData->GetDataName(),
                                   fSANumOfChan,fSANumOfEvPerChan,
                                   fXSAmin,fXSAmax); 
      fSAData  = md.fSAData;
     } 
  else fSAData = NULL;
  if(md.fSADataRMS) 
     {         
      fSADataRMS   = new TaStripChart(md.fSADataRMS->GetName(),
                                      md.fSADataRMS->GetDataName(),
                                      fSANumOfChan,fSANumOfEvPerChan,
                                      fXSAmin,fXSAmax); 
      fSADataRMS  = md.fSADataRMS;
     } 
  else fSADataRMS = NULL;
  if(md.fHAData) 
     {         
      fHAData   = new TH1D(md.fHAData->GetName(),md.fHAData->GetTitle(),
                           md.fHAbins,fXHAmin,fXHAmax); 
      fHAData  = md.fHAData;
     } 
  else fHAData = NULL;
}

TaPanamADevice& TaPanamADevice::operator=(const TaPanamADevice &md)
{
if ( this != &md )
    {
     TaPanamDevice::operator=(md);
     if(fSAData)        delete fSAData; 
     if(fSADataRMS)    delete fSADataRMS; 
     if(fHAData)        delete fHAData;
     fADataVal          = md.fADataVal;          
     fHArray            = md.fHArray;
     fSANumOfChan       = md.fSANumOfChan;
     fSANumOfEvPerChan  = md.fSANumOfEvPerChan;
     fHAbins            = md.fHAbins;
     fXSAmin            = md.fXSAmin;
     fXSAmax            = md.fXSAmax;
     fXHAmin            = md.fXHAmin;
     fXHAmax            = md.fXHAmax;
     fAColor            = md.fAColor;
     if (md.fSAData) 
       {         
        fSAData   = new TaStripChart(md.fSAData->GetName(),md.fSAData->GetDataName(),
                                    fSANumOfChan,fSANumOfEvPerChan,
                                    fXSAmin,fXSAmax); 
        fSAData  = md.fSAData;
       } 
     else fSAData = NULL;
     if (md.fSADataRMS) 
       {         
        fSADataRMS   = new TaStripChart(md.fSADataRMS->GetName(),md.fSADataRMS->GetDataName(),
                                       fSANumOfChan,fSANumOfEvPerChan,
                                       fXSAmin,fXSAmax); 
        fSADataRMS  = md.fSADataRMS;
       } 
     else fSADataRMS = NULL;
     if(md.fHAData) 
       {         
        fHAData   = new TH1D(md.fHAData->GetName(),md.fHAData->GetTitle(),
                            md.fHAbins,fXHAmax,fXHAmax); 
        fHAData  = md.fHAData;
       } 
     else fHAData = NULL;
     return *this;     
    }
 else
   {
     cout<<"warning ! TaPanamADevice assignment to self !"<<endl;
     return *this;
   }
}
  
void
TaPanamADevice::Init()
{
  string dname;
  fHArray.push_back(fHData);
  if (!fWhichData)
    {
     dname = string("S_")+string(fName) + string("_Asy"); 
     fSAData    = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                                   fSANumOfChan,fSANumOfEvPerChan,
                                   fXSAmin,fXSAmax);   
     fSCArray.push_back(fSAData); 
     dname = string("S_")+string(fName) + string("_Asy_RMS"); 
     fSADataRMS = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                                  fSANumOfChan,fSANumOfEvPerChan,
                                  fXSAmin,fXSAmax);
     fSCArray.push_back(fSADataRMS); 
     dname = string("H_")+string(fName) + string("_Asy"); 
     fHAData    = new TH1D((char*) dname.c_str(), (char*) dname.c_str(),
                            fHAbins,fXHAmin,fXHAmax); 
     fHArray.push_back(fHAData);
    }
  else
    {
     dname = string("S_")+string(fName) + string("_Diff"); 
     fSAData    = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                                   fSANumOfChan,fSANumOfEvPerChan,
                                   fXSAmin,fXSAmax);   
     fSCArray.push_back(fSAData); 
     dname = string("S_")+string(fName) + string("_Diff_RMS"); 
     fSADataRMS = new TaStripChart((char*)dname.c_str(),(char*)dname.c_str(),
                                  fSANumOfChan,fSANumOfEvPerChan,
                                  fXSAmin,fXSAmax);
     fSCArray.push_back(fSADataRMS); 
     dname = string("H_")+string(fName) + string("_Diff"); 
     fHAData    = new TH1D((char*) dname.c_str(), (char*) dname.c_str(),
                            fHAbins,fXHAmin,fXHAmax); 
     fHArray.push_back(fHAData);     
    }
}
void 
TaPanamADevice::FillFromPair(VaPair& pair)
 {
   if (fDevicekey !=0)
     {
       if (!fWhichData) 
         {
	   fADataVal = pair.GetAsy(fDevicekey)*1E6;
 	  if (fSAData->DataSum(fADataVal))
            { 
 	     fSAData->AddBinVal(fSAData->GetSumNorm());
             fSAData->Fill();
            }
          fHAData->Fill(fADataVal);
	  if(fSADataRMS->DataSum(fHData->GetRMS()))
            { 
	     fSADataRMS->AddBinVal(fSDataRMS->GetSumNorm());
             fSADataRMS->Fill();
            }         
	 }
       else
        {
	  fADataVal = pair.GetDiff(fDevicekey)*1E3;
	  if (fSAData->DataSum(fADataVal))
            { 
	     fSAData->AddBinVal(fSAData->GetSumNorm());
             fSAData->Fill();
            }
          fHAData->Fill(fADataVal);
	  if (fSADataRMS->DataSum(fHAData->GetRMS()))
            { 
	      fSADataRMS->AddBinVal(fSADataRMS->GetSumNorm());
              fSADataRMS->Fill();
            }         
	}
     }

}

void 
TaPanamADevice::DrawHPad(UInt_t plotidx)
{
  if (plotidx <= (UInt_t)fHArray.size()) fHArray[plotidx]->Draw();
  else cout<<" can't plot, index is bigger than array size!!! \n";
}
