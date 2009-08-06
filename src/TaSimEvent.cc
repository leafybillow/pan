//**********************************************************************
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           TaSimEvent.cc  (implementation)
//
// Author:  R. Holmes <http://mep1.phy.syr.edu/~rsholmes>, A. Vacheret <http://www.jlab.org/~vacheret>, R. Michaels <http://www.jlab.org/~rom>, K.Paschke
// @(#)pan/src:$Name$:$Id$
//
////////////////////////////////////////////////////////////////////////
//
//    An event of data, probably munged up by some varity of simulation!
//
////////////////////////////////////////////////////////////////////////

//#define NOISY
//#define DEBUG

#include "TaSimEvent.hh"

#include "TaCutList.hh"
#include "TaLabelledQuantity.hh"
#include "TaRun.hh"
#include "TaString.hh"
#include "TaDevice.hh"
#include "TaDataBase.hh"
#include <iostream>
#include <iomanip>
#include <utility>
#include <cmath>

#ifndef NODICT
ClassImp(TaSimEvent)
#endif

// Static members

#ifdef FAKEHEL
ifstream TaSimEvent::fgHelfile("helicity.data");
#endif

//  TRandom TaSimEvent::fgR;      // Random number object
//  Double_t TaSimEvent::fDetVsBcm[4];
//  Double_t TaSimEvent::fDetNoiseR[4];
//  // modify position by dithering slope for 12x, 4a x,y and 4b x,y given value
//  //  and ident of dithering object
//  //                            coils:  1x     1y     2x     2y     3x     3y     E
//  Double_t TaSimEvent::fBpm4AXvCoil[7];   Double_t TaSimEvent::fBpm4AYvCoil[7];
//  Double_t TaSimEvent::fBpm4BXvCoil[7];   Double_t TaSimEvent::fBpm4BYvCoil[7];
//  Double_t TaSimEvent::fBpm12XvCoil[7];
//  // modify detector by position
//  Double_t TaSimEvent::fDetVsBpm12X[4];  
//  Double_t TaSimEvent::fDetVsBpm4AX[4];  Double_t TaSimEvent::fDetVsBpm4BX[4];
//  Double_t TaSimEvent::fDetVsBpm4AY[4];  Double_t TaSimEvent::fDetVsBpm4BY[4];
//  Double_t TaSimEvent::fDet12Xoff;  
//  Double_t TaSimEvent::fDet4AXoff;  Double_t TaSimEvent::fDet4BXoff;
//  Double_t TaSimEvent::fDet4AYoff;  Double_t TaSimEvent::fDet4BYoff;



TaSimEvent::TaSimEvent(): VaEvent()
{
  this->SetSimConstants();
}

TaSimEvent::~TaSimEvent() {
}

TaSimEvent::TaSimEvent(const TaSimEvent& ev): VaEvent(ev)
{
  this->SetSimConstants();
} 

TaSimEvent &TaSimEvent::operator=(const TaSimEvent &ev)
{
  if ( &ev != this )
    {
      VaEvent::Uncreate();
      VaEvent::Create (ev);
    }
  return *this;
}

TaSimEvent& 
TaSimEvent::CopyInPlace (const TaSimEvent& rhs)
{
  // Like operator=, but copy an event into existing memory, not
  // deleting and reallocating.  This should always be safe, but just
  // to minimize possible problems use this instead of operator= only
  // when pointers must be preserved.
  VaEvent::CopyInPlace(rhs);

  // copy over sim specific data arrays

  return *this;
};


// Major functions

ErrCode_t TaSimEvent::RunInit(const TaRun& run)
{ 
 // Initialization at start of run.  Get quantities from database
  // which will be needed in event loop, and set other static variables
  // to initial values.

  clog << "!!!!!****************************!!!!!" << endl;
  clog << "!!!!!****************************!!!!!" << endl;
  clog << "TaSimEvent::RunInit WARNING: Data Faking is ON! **************" << endl;
  clog << "!!!!!****************************!!!!!" << endl;
  clog << "!!!!!****************************!!!!!" << endl;

  ErrCode_t stat = VaEvent::RunInit(run);

  return stat;
}

void TaSimEvent::SetSimConstants()
{

  // one day, it would be nice to have a non-hardwire interface for these parameters.

  fDetVsBcm[0] = 1.0;  fDetVsBcm[1] = 1.0;
  fDetVsBcm[2] = 1.0;  fDetVsBcm[3] = 1.0;

  fDetNoiseR[0] = 0.005;  fDetNoiseR[1] = 0.005;
  fDetNoiseR[2] = 0.005;  fDetNoiseR[3] = 0.005;

//   fBpm4AXvCoil[0] =  1.5; //1x 
//   fBpm4AXvCoil[1] = -0.5; //1y 
//   fBpm4AXvCoil[2] = -1.5; //2x 
//   fBpm4AXvCoil[3] =  0.0; //2y 
//   fBpm4AXvCoil[4] = -1.5; //3x 
//   fBpm4AXvCoil[5] = -0.5; //3y 
//   fBpm4AXvCoil[6] =  0.0; //E  

//   fBpm4AYvCoil[0] =  0.5; //1x 
//   fBpm4AYvCoil[1] =  1.5; //1y 
//   fBpm4AYvCoil[2] =  0.5; //2x 
//   fBpm4AYvCoil[3] =  1.5; //2y 
//   fBpm4AYvCoil[4] = -0.5; //3x 
//   fBpm4AYvCoil[5] = -2.0; //3y 
//   fBpm4AYvCoil[6] =  0.0; //E  

//   fBpm4BXvCoil[0] = -1.5; //1x
//   fBpm4BXvCoil[1] =  0.5; //1y
//   fBpm4BXvCoil[2] = -1.5; //2x
//   fBpm4BXvCoil[3] =  0.5; //2y
//   fBpm4BXvCoil[4] =  2.0; //3x
//   fBpm4BXvCoil[5] = -0.5; //3y
//   fBpm4BXvCoil[6] =  0.00; //E  

//   fBpm4BYvCoil[0] =  0.00; //1x
//   fBpm4BYvCoil[1] = -1.50; //1y
//   fBpm4BYvCoil[2] =  0.00; //2x
//   fBpm4BYvCoil[3] =  1.00; //2y
//   fBpm4BYvCoil[4] =  0.50; //3x
//   fBpm4BYvCoil[5] =  1.00; //3y
//   fBpm4BYvCoil[6] =  0.00; //E 

//   fBpm12XvCoil[0] =  0.500; //1x
//   fBpm12XvCoil[1] =  0.500; //1y
//   fBpm12XvCoil[2] =  0.500; //2x
//   fBpm12XvCoil[3] =  0.500; //2y
//   fBpm12XvCoil[4] =  0.500; //3x
//   fBpm12XvCoil[5] =  0.500; //3y
//   fBpm12XvCoil[6] =  2.000; //E 

// Raw BPMs in units off mm.
// Coils readout as uA, with 1E-6 in Decode, so this should be mm/A
// realistically, I think we should expect 100 um / 100 mA, 
// which is on order 1 in these units

  fBpm4AXvCoil[0] =  10.;  //1x 
  fBpm4AXvCoil[1] =   0.0; //1y 
  fBpm4AXvCoil[2] =   0.0; //2x 
  fBpm4AXvCoil[3] =   0.0; //2y 
  fBpm4AXvCoil[4] =   0.0; //3x 
  fBpm4AXvCoil[5] =   0.0; //3y 
  fBpm4AXvCoil[6] =   0.0; //E  

  fBpm4AYvCoil[0] =   0.0; //1x 
  fBpm4AYvCoil[1] =  10.0; //1y 
  fBpm4AYvCoil[2] =   0.0; //2x 
  fBpm4AYvCoil[3] =   0.0; //2y 
  fBpm4AYvCoil[4] =   0.0; //3x 
  fBpm4AYvCoil[5] =   0.0; //3y 
  fBpm4AYvCoil[6] =   0.0; //E  

  fBpm4BXvCoil[0] =   0.0; //1x
  fBpm4BXvCoil[1] =   0.0; //1y
  fBpm4BXvCoil[2] =  10.0; //2x
  fBpm4BXvCoil[3] =   0.0; //2y
  fBpm4BXvCoil[4] =   0.0; //3x
  fBpm4BXvCoil[5] =   0.0; //3y
  fBpm4BXvCoil[6] =   0.0; //E  

  fBpm4BYvCoil[0] =   0.0; //1x
  fBpm4BYvCoil[1] =   0.0; //1y
  fBpm4BYvCoil[2] =   0.0; //2x
  fBpm4BYvCoil[3] =  10.0; //2y
  fBpm4BYvCoil[4] =   0.0; //3x
  fBpm4BYvCoil[5] =   0.0; //3y
  fBpm4BYvCoil[6] =   0.0; //E 

  fBpm12XvCoil[0] =   0.0; //1x
  fBpm12XvCoil[1] =   0.0; //1y
  fBpm12XvCoil[2] =   0.0; //2x
  fBpm12XvCoil[3] =   0.0; //2y
  fBpm12XvCoil[4] =   0.0; //3x
  fBpm12XvCoil[5] =  10.0; //3y
  fBpm12XvCoil[6] =   0.0; //E 

  fDet12Xoff = 0.407;  
  fDet4AXoff = 0.0824;
  fDet4BXoff = -0.166;
  fDet4AYoff = 0.2892;
  fDet4BYoff = -1.084;

  // this is fraction, per mm.
  // we expect on order 40 ppm/um in pair tree, but
  // this is done in raw tree (without extra factor of 2)
  // so 80000 ppm / mm  or about 0.08 in the units seen here.

  fDetVsBpm12X[0] =  0.800;  fDetVsBpm12X[1] =  0.80;
  fDetVsBpm12X[2] =  0.200;  fDetVsBpm12X[3] =  0.20;  
  
  fDetVsBpm4AX[0] =  0.800;  fDetVsBpm4AX[1] = -0.000;
  fDetVsBpm4AX[2] =  0.080;  fDetVsBpm4AX[3] = -0.080;
  
  fDetVsBpm4BX[0] =  0.000;  fDetVsBpm4BX[1] = -0.800;
  fDetVsBpm4BX[2] = -0.080;  fDetVsBpm4BX[3] =  0.080;
  
  fDetVsBpm4AY[0] =  0.000;  fDetVsBpm4AY[1] =  0.200;
  fDetVsBpm4AY[2] =  0.080;  fDetVsBpm4AY[3] =  0.080;
  
  fDetVsBpm4BY[0] =  0.400;  fDetVsBpm4BY[1] =  0.400;
  fDetVsBpm4BY[2] = -0.080;  fDetVsBpm4BY[3] =  0.040;

}
  

void TaSimEvent::Decode(TaDevice& devices) {
// Decodes all the raw data and applies all the calibrations and BPM
// rotations.. THEN it adds in a simulation, modifying the 
// beam monitors to simulate dithering (depending on paramters)
// and then modifying detectors
// to account for beam position, beam current, and some noise
// This will also allow for faking the helicity signals

  Int_t key;

  VaEvent::Decode(devices);  // first, handle the real data

   for (Int_t i = 0; i < 4; ++i)
     {
       // set detector = cal * bcm
       key = DETOFF + 3*i +1;
       fData[key] = fData[IBCM1]*(fDetVsBcm[i] + fgR.Gaus(0,fDetNoiseR[i]));
       devices.SetUsed(key);
       devices.SetUsed(key-1);
     }

  // modify position by dithering slope for 12x, 4a x,y and 4b x,y given value
  //  and ident of dithering object
  if (fData[IBMWOBJ]<7 && fData[IBMWOBJ]>-1 && 
      fData[IBMWCYC]>0 && fData[IBMWCLN]>0    ) {
    Int_t icoil = (int) fData[IBMWOBJ];
    if (icoil>=0) {
      fData[IBPM4AX] += fBpm4AXvCoil[icoil]*fData[IBMWVAL]/1.E6;
      fData[IBPM4BX] += fBpm4BXvCoil[icoil]*fData[IBMWVAL]/1.E6;
      fData[IBPM4AY] += fBpm4AYvCoil[icoil]*fData[IBMWVAL]/1.E6;
      fData[IBPM4BY] += fBpm4BYvCoil[icoil]*fData[IBMWVAL]/1.E6;
      fData[IBPM12X] += fBpm12XvCoil[icoil]*fData[IBMWVAL]/1.E6;
    }
  }

  // modify detector by position
  for (Int_t i = 0; i < DETNUM; i++) {
    key = DETOFF + 3*i +1;
    fData[key] *= (1.0+fDetVsBpm4AX[i]*(fData[IBPM4AX]-fDet4AXoff));
    fData[key] *= (1.0+fDetVsBpm4BX[i]*(fData[IBPM4BX]-fDet4BXoff));
    fData[key] *= (1.0+fDetVsBpm4AY[i]*(fData[IBPM4AY]-fDet4AYoff));
    fData[key] *= (1.0+fDetVsBpm4BY[i]*(fData[IBPM4BY]-fDet4BYoff));
    fData[key] *= (1.0+fDetVsBpm12X[i]*(fData[IBPM12X]-fDet12Xoff));
  }

#ifdef FAKEHEL
  // overwrite recorded control signals
  fgHelfile >> fData[IHELICITY] >> fData[IPAIRSYNCH]
	    >> fData[IMULTIPLETSYNCH] >> fData[ITIMESLOT];
//    clog << "TaSimEvent::Load hel/ps/ms/ts: " 
//         << " " << fData[IHELICITY]
//         << " " << fData[IPAIRSYNCH]
//         << " " << fData[IMULTIPLETSYNCH]
//         << " " << fData[ITIMESLOT] << endl;
#endif


};


void TaSimEvent::Create(const TaSimEvent& rhs)
{
  // Utility routine used by copy constructor and assignment.
  VaEvent::Create(rhs);

  // also copy simulation specific arrays... err, reset, but it's ok, 'cause
  //  they are hardwired in anyway...
  this->SetSimConstants();
};



