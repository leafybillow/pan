////////////////////////////////////////////////////////////////////////
//
// HALL A C++/ROOT parity analyzer  Pan           
//
// TaADCCalib.cc
//
// ADC calibration class
//
// Authors: KDP Feb 2002
//
////////////////////////////////////////////////////////////////////////

#include <fstream>
#include "TaADCCalib.hh"
#include "TaEvent.hh"
#include "TaRun.hh"
#include "TaLabelledQuantity.hh"

#ifdef DICT
ClassImp(TaADCCalib)
#endif


// This non-member string comparison routine probably should be somewhere else!
Int_t cmp_nocase2 (const string& s, const string& s2)
{
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end())
    {
      if (toupper(*p) != toupper(*p2))
	return (toupper(*p) < toupper(*p2)) ? -1 : 1;
      ++p;
      ++p2;
    }

  return (s2.size() == s.size()) ? 0 : (s.size() < s2.size()) ? -1 : 1;
}

// Constructors/destructors/operators

TaADCCalib::TaADCCalib():VaAnalysis()
{
  typeFlag = 0;
  hist = new TH1F* [(ADC_MaxSlot+1)*ADC_MaxChan];
}

TaADCCalib::TaADCCalib(const string& anName)
  :VaAnalysis()
{
  if (cmp_nocase2 (anName, "adcped") == 0)
    typeFlag = 1;
  else if (cmp_nocase2 (anName, "adcdac") == 0)
    typeFlag = 2;
  else
    typeFlag = 0;

  hist = new TH1F* [(ADC_MaxSlot+1)*ADC_MaxChan];
  // initialize sum, sum2 arrays
  fSumX    = vector<Double_t>((size_t) (ADC_MaxSlot+1)*ADC_MaxChan,0);
  fSumX2   = vector<Double_t>((size_t) (ADC_MaxSlot+1)*ADC_MaxChan,0);
  nEntries = vector<Int_t>((size_t) (ADC_MaxSlot+1)*ADC_MaxChan,0);
	
  cout << "TaADCCalib:: analysis type is " << anName << " and typeflag is " 
       << typeFlag << endl;
   
}



TaADCCalib::~TaADCCalib(){}


// Private member functions

// We should not need to copy or assign an analysis, so copy
// constructor and operator= are private.

TaADCCalib::TaADCCalib (const TaADCCalib& copy) 
{
}


TaADCCalib& TaADCCalib::operator=( const TaADCCalib& assign) 
{ 
  return *this; 
}


void TaADCCalib::Init()
{
  VaAnalysis::Init();
  cout << "Local init of TaADCCalib needed also..." << endl;

  char charkey[10];
  string key; 
  // loop over slots, 0-9
  //  cout << "loop over slots" << endl;
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    //    cout << "   loop over channels" << endl;
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
      //  make key ADC<slot>_<chan>
      sprintf(charkey, "adc%i_%i", isl,ich);
      key = charkey;
      //      cout << "      produced key " << key << endl;
      //  check for existence of key  
      //     needs utility in TaEvent to search fKeyDev for the key.
      //     if exists, mark existence on slot, chan bool array
      //     for now, just assume it exists
      chanExists[isl][ich] = true;
    }
  }

  // separate init calls for each type of analysis
  if (typeFlag == 1) 
    InitPed();
  else if (typeFlag == 2)
    InitDAC();
  else {
    cout << endl;
    cout << " Invalid ADC Calibration Procedure Selected: typeFlag = " 
	 << typeFlag <<endl;
    cout << endl;
  }

}

void TaADCCalib::Finish() 
{
  VaAnalysis::Finish();
  cout << " Local finish of TaADCCalib needed also..." << endl;


  // separate init calls for each type of analysis
  if (typeFlag == 1) 
    FinishPed();
  else if (typeFlag == 2)
    FinishDAC();
  else {
    cout << endl;
    cout << " Invalid ADC Calibration Procedure Selected: typeFlag = " 
	 << typeFlag <<endl;
    cout << endl;
  }


  hfile->Write();
  hfile->Close();

}

void TaADCCalib::FinishPed()
{

  cout << "Output from event accumulation." << endl;
  char charkey[10];
  string key; 
  Int_t id;
  vector<Double_t> avg = vector<Double_t>((size_t) (ADC_MaxSlot+1)*ADC_MaxChan,0);
  vector<Double_t> sigma = vector<Double_t>((size_t) (ADC_MaxSlot+1)*ADC_MaxChan,0);

  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
        id = isl*ADC_MaxChan + ich + 1;
	if (nEntries[id]>0 && fSumX2[id]>0) {
	  avg[id] = fSumX[id] / nEntries[id];
	  sigma[id] = sqrt(fSumX2[id]/nEntries[id] - avg[id]*avg[id]);
	}

    }
  }

  cout << "\n The following keys were not found : " << endl;
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
	//  make key ADC<slot>_<chan>
	sprintf(charkey, "adc%i_%i", isl,ich);
	key = charkey;
        id = isl*ADC_MaxChan + ich + 1;
	if (!chanExists[isl][ich])
	  cout << "  " << key << "  was not found." << endl;
    }
  }

  cout << "\n The following keys were found with no data : " << endl;
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
	//  make key ADC<slot>_<chan>
	sprintf(charkey, "adc%i_%i", isl,ich);
	key = charkey;
        id = isl*ADC_MaxChan + ich + 1;
	if (chanExists[isl][ich] && (nEntries[id]<=0 || fSumX2[id]<=0)) 
	  cout << "  " << key << "  returned no data for all events." << endl;
    }
  }

  cout << "\n Pedestals found and set" << endl;
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
	//  make key ADC<slot>_<chan>
	sprintf(charkey, "adc%i_%i", isl,ich);
	key = charkey;
        id = isl*ADC_MaxChan + ich + 1;
	if (chanExists[isl][ich] && nEntries[id]>0 && fSumX2[id]>0) {
	  cout << "  " << key << " : " << avg[id] << " +/- " << sigma[id] << endl;
	  //	  cout << "       " << fSumX[id] << "  " << fSumX2[id] << endl;
	}
    }
  }

  ofstream ofile("ADCCalib_Peds.txt",ios::out);
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
        id = isl*ADC_MaxChan + ich + 1;
	if (chanExists[isl][ich] && nEntries[id]>0 && fSumX2[id]>0) {
	  ofile << isl << "  " << ich << "  " << avg[id] << "  " 
		<< sigma[id] << endl;
	}
    }
  }
  ofile.close();

  

}

void TaADCCalib::FinishDAC()
{

  cout << "Output from event accumulation." << endl;
  char charkey[10];
  string key; 
  Int_t id;
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
	//  make key ADC<slot>_<chan>
	sprintf(charkey, "adc%i_%i", isl,ich);
	key = charkey;
        id = isl*ADC_MaxChan + ich + 1;
	cout << "        key       = " << key << endl;
	cout << "        Exists    = " << chanExists[isl][ich] << endl;
	cout << "        nEntries  = " << nEntries[id] << endl;
	cout << "        fSumX     = " << fSumX[id] << endl;
	cout << "        fSumX2    = " << fSumX2[id] << endl;
    }
  }
}



void TaADCCalib::InitPed()
{
  cout << "TaADCCalib:: Initializing ADC Pedestal analysis" << endl;

  // Set up ROOT.  Define output file.
  hfile = new TFile("ADCCalib_Peds.root","RECREATE","Pedestal calibration file");

  char *hid = new char[100];
  char *title = new char[100];

  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    //    cout << "   loop over channels" << endl;
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
      if ( chanExists[isl][ich] ) {
	//  if exists, Book 1D histo for ADC value	
	Int_t ihid = isl*ADC_MaxChan + ich + 1;
	sprintf(hid,"Ped:%d-%d",isl,ich);
	sprintf(title,"ADC slot %d, channel %d",isl, ich); 
	hist[ihid] = new TH1F(hid,title,25000,0,25000);
	// initialize sum, sum2 arrays
	fSumX[ihid]   = 0.0;
	fSumX2[ihid]  = 0.0;
	nEntries[ihid]= 0;
      }
    }
  }
}

void TaADCCalib::InitDAC()
{
  cout << "TaADCCalib:: Initializing ADC noise DAC calibration analysis" << endl;

  // loop over slots, 0-9
  //   loop over chans 1-4
  //     check existence of key from array filled in Init()
  //     if exists, Book 2D histo for ADC value vs DAC value
  //                create vectors for adc vs DACVal graph
}


void TaADCCalib::ProcessRun()
{
  // Main analysis routine -- this is the event loop, over-ride this
  // from VaAnalysis because it is such a pain to deal with helicity

  while ( fRun->NextEvent() )
    {
      fRun->GetEvent().CheckEvent(*fRun);
      *fEvt = fRun->GetEvent();
      EventAnalysis();
      if (fRun->GetEvent().GetEvNumber() >= fMaxNumEv)
	break;
    }
  
}



void TaADCCalib::EventAnalysis()
{
  // put everything needed to analyze one event 

  // when looping over all devices, make keys (strings like adc0_1, first
  // number slot, second number channel), and check whether keys exist.
  // use GetData(key) to get data?
  // maybe init phase can be used to check for all valid keys?


  //  cout << "TaADCCalib: Processing Event Analaysis for ADC Calib" << endl;
  char charkey[10];
  string key; 
  Int_t id;
  Double_t dataX;
  // loop over slots, 0-9
  for (Int_t isl=0; isl < ADC_MaxSlot; isl++) {
    //  loop over chans 1-4
    for (Int_t ich=0; ich < ADC_MaxChan;  ich++) {
      if (chanExists[isl][ich]) {
	id = isl*ADC_MaxChan + ich + 1;
	//  fEVt->GetData("slotnum","channum"); or fEVt->GetData("key");
	// THREE methods of get data:
	//
	//  1) make (globally unique) key ADC<slot>_<chan>
	//	sprintf(charkey, "adc%i_%i", isl,ich+1);
	//	key = charkey;
	//	dataX = fEvt->GetData(key);
	//	cout << "Data for " << key << " : " <<dataX << endl;
	//
	//   2) **  GetADCData(slot, channel) doesn't work... **
	//	dataX = fEvt->GetADCData(isl,ich);
	//	cout << "Data for Slot " << isl << "  Channel  " << ich << " : " 
	//	     <<dataX << endl;
	//
	//   3) make device name, check individual channel by number:
	sprintf(charkey, "adc%i", isl);
	key = charkey;
	dataX = fEvt->GetData(key,ich+1);
	//	cout << "Data for " << key << "  Channel  " << ich << " : " 
	//	     <<dataX << endl;

	// PEDESTAL analysis
	// accumulate sums for ADC pedestal averages, widths
	fSumX[id]  += dataX;
	fSumX2[id] += dataX*dataX;
	nEntries[id]++;
	// fill each 1D pedestal histo  
	hist[id]->Fill(dataX);
	//	cout << "TaADCCalib:   Filling histo for " << key << endl;
      }
    }
  }



  // DACCalib analysis
  // accumulate sums for ADC vs DAC lin fit for slope, intercept, and uncertainties
  // fill each 2D histo?
  // fill ADC vs DACVal vector, to be later turned into a graph 

}

void TaADCCalib::PairAnalysis()
{ 
  // no pair analysis for ADC calib runs
}


void TaADCCalib::InitChanLists ()
{
  // not using any external framework for output in root files, etc. 
  // so this routine is commented out.
}




