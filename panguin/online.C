///////////////////////////////////////////////////////////////////
//  Macro to help with online analysis
//    B. Moffit  Oct. 2003

#include "online.h"
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <TMath.h>
#include <TBranch.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TGImageMap.h>
#include <TGFileDialog.h>
#include <TKey.h>
#include <TSystem.h>
#include <TLatex.h>
#include "GetRootFileName.C"
#include "GetRunNumber.C"

//#define DEBUG
//#define DEBUG2
//#define NOISY
//#define OLDTIMERUPDATE

///////////////////////////////////////////////////////////////////
//  Class: OnlineConfig
//
//     Utility class that reads in a text file (.cfg) and
//     stores it's contents.
//
//

OnlineConfig::OnlineConfig() 
{
  // Constructor.  Without an argument, will use default "standard" config
  fMonitor = kFALSE;
  OnlineConfig("standard");
}

OnlineConfig::OnlineConfig(TString anatype) 
{
  // Constructor.  Takes the config anatype as the only argument.
  //  Loads up the configuration file, and stores it's contents for access.
  
  confFileName = anatype;
  confFileName += ".cfg";
  fMonitor = kFALSE;
  fFoundCfg = kFALSE;

  //  ifstream *fConfFile = new ifstream(confFileName.Data());
  fConfFile = new ifstream(confFileName.Data());
  if ( ! (*fConfFile) ) {
    cerr << "OnlineConfig() WARNING: config file " << confFileName.Data()
         << " does not exist" << endl;
    cerr << " Checking the panguin directory" << endl;
    confFileName.Prepend("panguin/");
    fConfFile = new ifstream(confFileName.Data());
    if ( ! (*fConfFile) ) {
      confFileName = "panguin/default.cfg";
      cout << "OnlineConfig()  Trying " << confFileName.Data() 
	   << " as default configuration." << endl
	   << " (May be ok.)" << endl;
      fConfFile = new ifstream(confFileName.Data());
      if ( ! (*fConfFile) ) {
	cerr << "OnlineConfig() WARNING: no file "
	     << confFileName.Data() <<endl;
	cerr << "You need a configuration to run.  Ask an expert."<<endl;
	fFoundCfg = kFALSE;
	//      return;
      } else {
	fFoundCfg = kTRUE;
      }
    } else {
      fFoundCfg = kTRUE;
    }
  } else {
    fFoundCfg = kTRUE;
  }

  if(fFoundCfg) {
  clog << "GUI Configuration loading from " 
       << confFileName.Data() << endl;
  }

  ParseFile();

  fConfFile->close();
  delete fConfFile;

}

void OnlineConfig::ParseFile() 
{
  // Reads in the Config File, and makes the proper calls to put
  //  the information contained into memory.

  if(!fFoundCfg) {
    return;
  }

  TString comment = "#";
  vector<TString> strvect;
  TString sinput, sline;
  while (sline.ReadLine(*fConfFile)) {
    if(sline.Contains(comment)) continue;
    strvect = SplitString(sline," ");
    sConfFile.push_back(strvect);
  }

#ifdef DEBUG  
  for(UInt_t ii=0; ii<sConfFile.size(); ii++) {
    cout << "Line " << ii << endl << "  ";
    for(UInt_t jj=0; jj<sConfFile[ii].size(); jj++) 
      cout << sConfFile[ii][jj] << " ";
    cout << endl;
  }
#endif

  cout << "     " << sConfFile.size() << " lines read from " 
       << confFileName << endl;
  
}

Bool_t OnlineConfig::ParseConfig() 
{
  //  Goes through each line of the config [must have been ParseFile()'d]
  //   and interprets.

  if(!fFoundCfg) {
    return kFALSE;
  }

  UInt_t command_cnt=0;
  UInt_t j=0;
  // If statement for each high level command (cut, newpage, etc)
  for(UInt_t i=0;i<sConfFile.size();i++) {
    // "newpage" command
    if(sConfFile[i][0] == "newpage") {
      // sConfFile[i] is first of pair
      for(j=i+1;j<sConfFile.size();j++) {
	if(sConfFile[j][0] != "newpage") {
	  // Count how many commands within the page
	  command_cnt++;
	} else break;
      }
      pageInfo.push_back(make_pair(i,command_cnt));
      i += command_cnt;
      command_cnt=0;
    }
    if(sConfFile[i][0] == "watchfile") {
      fMonitor = kTRUE;
    }
    if(sConfFile[i][0] == "definecut") {
      if(sConfFile[i].size()>3) {
	cerr << "cut command has too many arguments" << endl;
	continue;
      }
      TCut tempCut(sConfFile[i][1],sConfFile[i][2]);
      //      cutList.push_back(make_pair(sConfFile[i][1],sConfFile[i][2]));
      cutList.push_back(tempCut);
    }
    if(sConfFile[i][0] == "rootfile") {
      if(sConfFile[i].size() != 2) {
	cerr << "WARNING: rootfile command does not have the "
	     << "correct number of arguments"
	     << endl;
	continue;
      }
      if(!rootfilename.IsNull()) {
	cerr << "WARNING: too many rootfile's defined. " 
	     << " Will only use the first one." 
	     << endl;
	continue;
      }
      rootfilename = sConfFile[i][1];
    }

  }

#ifdef NOISY
  for(UInt_t i=0; i<GetPageCount(); i++) {
    cout << "Page " << i << " (" << GetPageTitle(i) << ")"
	 << " will draw " << GetDrawCount(i) 
	 << " histograms." << endl;
  }
#endif

  cout << "Number of pages defined = " << GetPageCount() << endl;
  cout << "Number of cuts defined = " << cutList.size() << endl;

  if (fMonitor) 
    cout << "Will periodically update plots" << endl;

  return kTRUE;

}

TCut OnlineConfig::GetDefinedCut(TString ident) {
  // Returns the defined cut, according to the identifier

  for(UInt_t i=0; i<cutList.size(); i++) {
    if((TString)cutList[i].GetName() == ident.Data()) {
      TCut tempCut = cutList[i].GetTitle();
      return tempCut;
    }
  }
  return "";
}

vector <TString> OnlineConfig::GetCutIdent() {
  // Returns a vector of the cut identifiers, specified in config
  vector <TString> out;

  for(UInt_t i=0; i<cutList.size(); i++) {
    out.push_back(cutList[i].GetName());
  }

  return out;
}

Bool_t OnlineConfig::IsLogy(UInt_t page) {
// Check if last word on line is "logy"

  UInt_t page_index = pageInfo[page].first;
  Int_t word_index = sConfFile[page_index].size()-1;
  if (word_index <= 0) return kFALSE;
  TString option = sConfFile[page_index][word_index];  
  if(option == "logy") return kTRUE;
  return kFALSE;

}


pair <UInt_t, UInt_t> OnlineConfig::GetPageDim(UInt_t page) 
{
  // If defined in the config, will return those dimensions
  //  for the indicated page.  Otherwise, will return the
  //  calculated dimensions required to fit all histograms.

  pair <UInt_t, UInt_t> outDim;

  // This is the page index in sConfFile.
  UInt_t page_index = pageInfo[page].first;
  
  UInt_t size1 = 2;
  if (IsLogy(page)) size1 = 3;  // last word is "logy"
  
  // If the dimensions are defined, return them.
  if(sConfFile[page_index].size()>size1-1) {
    if(sConfFile[page_index].size() == size1) {
      outDim = make_pair(UInt_t(atoi(sConfFile[page_index][1])),
		       UInt_t(atoi(sConfFile[page_index][1])));
      return outDim;
    } else if (sConfFile[page_index].size() == size1+1) {
      outDim = make_pair(UInt_t(atoi(sConfFile[page_index][1])),
		       UInt_t(atoi(sConfFile[page_index][2])));
      return outDim;
    } else {
      cout << "Warning: newpage command has too many arguments. "
	   << "Will automatically determine dimensions of page."
	   << endl;
    }
  }
  
  // If not defined, return the "default."
  UInt_t draw_count = GetDrawCount(page);
  UInt_t dim = UInt_t(TMath::Nint(sqrt(double(draw_count+1))));
  outDim = make_pair(dim,dim);

  return outDim;
}

TString OnlineConfig::GetPageTitle(UInt_t page) 
{
  // Returns the title of the page.
  //  if it is not defined in the config, then return "Page #"

  TString title;

  UInt_t iter_command = pageInfo[page].first+1;

  for(UInt_t i=0; i<pageInfo[page].second; i++) { // go through each command
    if(sConfFile[iter_command+i][0] == "title") { 
      // Combine the strings, and return it
      for (UInt_t j=1; j<sConfFile[iter_command+i].size(); j++) {
	title += sConfFile[iter_command+i][j];
	title += " ";
      }
      title.Chop();
      return title;
    }
  }
  title = "Page "; title += page;
  return title;
}

vector <UInt_t> OnlineConfig::GetDrawIndex(UInt_t page) 
{
  // Returns an index of where to find the draw commands within a page
  //  within the sConfFile vector

  vector <UInt_t> index;
  UInt_t iter_command = pageInfo[page].first+1;

  for(UInt_t i=0; i<pageInfo[page].second; i++) {
    if(sConfFile[iter_command+i][0] != "title") {
      index.push_back(iter_command+i);
    }
  }

  return index;
}

UInt_t OnlineConfig::GetDrawCount(UInt_t page) 
{
  // Returns the number of histograms that have been request for this page
  UInt_t draw_count=0;

  for(UInt_t i=0; i<pageInfo[page].second; i++) {
    if(sConfFile[pageInfo[page].first+i+1][0] != "title") draw_count++;
  }

  return draw_count;

}

vector <TString> OnlineConfig::GetDrawCommand(UInt_t page, UInt_t nCommand)
{
  // Returns the vector of strings pertaining to a specific page, and 
  //   draw command from the config.
  // Return vector of TStrings:
  //  0: variable
  //  1: cut
  //  2: type
  //  3: title
  //  $: treename

  vector <TString> out_command(5);
  vector <UInt_t> command_vector = GetDrawIndex(page);
  UInt_t index = command_vector[nCommand];

#ifdef DEBUG
  cout << "OnlineConfig::GetDrawCommand(" << page << "," 
       << nCommand << ")" << endl;
#endif
  for(UInt_t i=0; i<out_command.size(); i++) {
    out_command[i] = "";
  }


  // First line is the variable
  if(sConfFile[index].size()>=1) {
    out_command[0] = sConfFile[index][0];
  }
  if(sConfFile[index].size()>=2) {
    if((sConfFile[index][1] != "-type") &&
       (sConfFile[index][1] != "-title") &&
       (sConfFile[index][1] != "-tree"))
      out_command[1] = sConfFile[index][1];
  }

  // Now go through the rest of that line..
  for (UInt_t i=1; i<sConfFile[index].size(); i++) {
    if(sConfFile[index][i]=="-type") {
      out_command[2] = sConfFile[index][i+1];
      i = i+1;
    } else if(sConfFile[index][i]=="-title") {
      // Put the entire title, surrounded by quotes, as one TString
      TString title;
      UInt_t j=0;
      for(j=i+1; j<sConfFile[index].size(); j++) {
	TString word = sConfFile[index][j];
	if( (word.BeginsWith("\"")) && (word.EndsWith("\"")) ) {
	  cout << "Urgent" << endl;
	  title = word.ReplaceAll("\"","");
	  out_command[3] = title;
	  i = j;
	  break;
	} else if(word.BeginsWith("\"")) {
	  title = word.ReplaceAll("\"","");
	} else if(word.EndsWith("\"")) {
	  title += " " + word.ReplaceAll("\"","");
	  out_command[3] = title;
	  i = j;
	  break;
	} else {
	  title += " " + word;
	}
      }
    } else if(sConfFile[index][i]=="-tree") {
      out_command[4] = sConfFile[index][i+1];
      i = i+1;
    }

#ifdef DEBUG
    cout << endl;
#endif
  }
#ifdef DEBUG
  cout << sConfFile[index].size() << ": ";
  for(UInt_t i=0; i<sConfFile[index].size(); i++) {
    cout << sConfFile[index][i] << " ";
  }
  cout << endl;
  for(UInt_t i=0; i<out_command.size(); i++) {
    cout << i << ": " << out_command[i] << endl;
  }
#endif
  return out_command;
}

vector <TString> OnlineConfig::SplitString(TString instring,TString delim) 
{
  // Utility to split up a string on the deliminator.
  //  returns a vector of strings.

  vector <TString> v;

  TString remainingString = instring;
  TString tempstring = instring;
  int i;

  while (remainingString.Index(delim) != -1) {
    i = remainingString.Index(delim);
    tempstring.Remove(i);
    v.push_back(tempstring);
    remainingString.Remove(0,i+1);
    while(remainingString.Index(delim) == 0) {
      remainingString.Remove(0,1);
    }
    tempstring = remainingString;
  }

  while(tempstring.EndsWith(delim)) {
    tempstring.Chop();
  }
     
  if(!tempstring.IsNull()) v.push_back(tempstring);

  return v;
}

void OnlineConfig::OverrideRootFile(UInt_t runnumber) 
{
  // Override the ROOT file defined in the cfg file
  //  Uses a helper macro "GetRootFileName.C(UInt_t runnumber)

  rootfilename = GetRootFileName(runnumber);

  cout << "Overridden File name: " << rootfilename << endl;
}

///////////////////////////////////////////////////////////////////
//  Class: OnlineGUI
//
//    Creates a GUI to display the commands used in OnlineConfig
//
//

OnlineGUI::OnlineGUI(OnlineConfig& config, Bool_t printonly):
  runNumber(0),
  timer(0),
  fFileAlive(kFALSE)
{
  // Constructor.  Get the config pointer, and make the GUI.

  fConfig = &config;

  if(printonly) {
    fPrintOnly=kTRUE;
    PrintPages();
  } else {
    fPrintOnly=kFALSE;
    CreateGUI(gClient->GetRoot(),200,200);
  }
}

void OnlineGUI::CreateGUI(const TGWindow *p, UInt_t w, UInt_t h) 
{
  
  // Open the RootFile.  Die if it doesn't exist.
  //  unless we're watching a file.
  fRootFile = new TFile(fConfig->GetRootFile(),"READ");
  if(!fRootFile->IsOpen()) {
    cout << "ERROR:  rootfile: " << fConfig->GetRootFile()
	 << " does not exist"
	 << endl;
    if(fConfig->IsMonitor()) {
      cout << "Will wait... hopefully.." << endl;
    } else {
      return;
    }
  } else {
    fFileAlive = kTRUE;
    ObtainRunNumber();
    // Open the Root Trees.  Give a warning if it's not there..
    GetFileObjects();
    GetRootTree();
    GetTreeVars();
    for(UInt_t i=0; i<fRootTree.size(); i++) {
      if(fRootTree[i]==0) {
	fRootTree.erase(fRootTree.begin() + i);
      }
    }

  }


  // Create the main frame
  fMain = new TGMainFrame(p,w,h);
  fMain->Connect("CloseWindow()", "OnlineGUI", this, "MyCloseWindow()");
  ULong_t black, lightblue, red;
  gClient->GetColorByName("black",black);
  gClient->GetColorByName("lightblue",lightblue);
  gClient->GetColorByName("red",red);

  fMain->SetBackgroundColor(lightblue);

  // Top frame, to hold page buttons and canvas
  fTopframe = new TGHorizontalFrame(fMain,200,200);
  fTopframe->SetBackgroundColor(lightblue);
  fMain->AddFrame(fTopframe, new TGLayoutHints(kLHintsExpandX 
                                              | kLHintsExpandY,10,10,10,1));

  // Create a verticle frame widget with radio buttons
  //  This will hold the page buttons
  vframe = new TGVerticalFrame(fTopframe,40,200);
  vframe->SetBackgroundColor(lightblue);
  TString buff;
  for(UInt_t i=0; i<fConfig->GetPageCount(); i++) {
    buff = fConfig->GetPageTitle(i);
    fRadioPage[i] = new TGRadioButton(vframe,buff,i);
    fRadioPage[i]->SetBackgroundColor(lightblue);
  }

  fRadioPage[0]->SetState(kButtonDown);
  current_page = 0;

  for (UInt_t i=0; i<fConfig->GetPageCount(); i++) {
    vframe->AddFrame(fRadioPage[i], new TGLayoutHints(kLHintsLeft |
                                                     kLHintsCenterY,5,5,3,4));
    fRadioPage[i]->Connect("Pressed()", "OnlineGUI", this, "DoRadio()");
  }
  wile = 
    new TGPictureButton(vframe,gClient->GetPicture("panguin/genius.xpm"));
  wile->SetBackgroundColor(lightblue);
  if(!fConfig->IsMonitor()) {
    wile->Connect("Pressed()","OnlineGUI", this,"DoDraw()");
  } else {
    wile->Connect("Pressed()","OnlineGUI", this,"DoDrawClear()");
  }

  vframe->AddFrame(wile,new TGLayoutHints(kLHintsBottom|kLHintsLeft,5,10,4,2));


  fTopframe->AddFrame(vframe,new TGLayoutHints(kLHintsCenterX|
                                               kLHintsCenterY,2,2,2,2));
  
  // Create canvas widget
  fEcanvas = new TRootEmbeddedCanvas("Ecanvas", fTopframe, 800, 600);
  fEcanvas->SetBackgroundColor(lightblue);
  fTopframe->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandY,10,10,10,1));
  fCanvas = fEcanvas->GetCanvas();

  // Create the bottom frame.  Contains control buttons
  fBottomFrame = new TGHorizontalFrame(fMain,1200,200);
  fBottomFrame->SetBackgroundColor(lightblue);
  fMain->AddFrame(fBottomFrame, new TGLayoutHints(kLHintsExpandX,10,10,10,10));
  
  // Create a horizontal frame widget with buttons
  hframe = new TGHorizontalFrame(fBottomFrame,1200,40);
  hframe->SetBackgroundColor(lightblue);
  fBottomFrame->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX,200,20,2,2));

  fPrev = new TGTextButton(hframe,"Prev");
  fPrev->SetBackgroundColor(lightblue);
  fPrev->Connect("Clicked()","OnlineGUI",this,"DrawPrev()");
  hframe->AddFrame(fPrev, new TGLayoutHints(kLHintsCenterX,5,5,1,1));

  fNext = new TGTextButton(hframe,"Next");
  fNext->SetBackgroundColor(lightblue);
  fNext->Connect("Clicked()","OnlineGUI",this,"DrawNext()");
  hframe->AddFrame(fNext, new TGLayoutHints(kLHintsCenterX,5,5,1,1));

  fExit = new TGTextButton(hframe,"Exit GUI");
  fExit->SetBackgroundColor(red);
  fExit->Connect("Clicked()","OnlineGUI",this,"CloseGUI()");

  hframe->AddFrame(fExit, new TGLayoutHints(kLHintsCenterX,5,5,1,1));
  
  TString Buff;
  if(runNumber==0) {
    Buff = "";
  } else {
    Buff = "Run #";
    Buff += runNumber;
  }
  TGString labelBuff(Buff);
  
  fRunNumber = new TGLabel(hframe,Buff);
  fRunNumber->SetBackgroundColor(lightblue);
  hframe->AddFrame(fRunNumber,new TGLayoutHints(kLHintsCenterX,5,5,1,1));

  fPrint = new TGTextButton(hframe,"Print To &File");
  fPrint->SetBackgroundColor(lightblue);
  fPrint->Connect("Clicked()","OnlineGUI",this,"PrintToFile()");
  hframe->AddFrame(fPrint, new TGLayoutHints(kLHintsCenterX,5,5,1,1));


  // Set a name to the main frame
  fMain->SetWindowName("Online Analysis GUI");
  
  // Map all sub windows to main frame
  fMain->MapSubwindows();
  
  // Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());
  
  // Map main frame
  fMain->MapWindow();

#ifdef DEBUG
  fMain->Print();
#endif

  if(fFileAlive) DoDraw();

  if(fConfig->IsMonitor()) {
    timer = new TTimer();
    if(fFileAlive) {
      timer->Connect(timer,"Timeout()","OnlineGUI",this,"TimerUpdate()");
    } else {
      timer->Connect(timer,"Timeout()","OnlineGUI",this,"CheckRootFile()");
    }
    timer->Start(UPDATETIME);
  }

}

void OnlineGUI::DoDraw() 
{
  // The main Drawing Routine.

  gStyle->SetOptStat(1110);
  gStyle->SetStatFontSize(0.1);
  if (fConfig->IsLogy(current_page)) {
    gStyle->SetOptLogy(1);
  } else {
    gStyle->SetOptLogy(0);
  }
  gStyle->SetTitleH(0.10);
  gStyle->SetTitleW(0.40);
//   gStyle->SetLabelSize(0.10,"X");
//   gStyle->SetLabelSize(0.10,"Y");
  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");
  gStyle->SetNdivisions(505,"X");
  gStyle->SetNdivisions(505,"Y");
  gROOT->ForceStyle();

  // Determine the dimensions of the canvas..
  UInt_t draw_count = fConfig->GetDrawCount(current_page);
  if(draw_count>=8) {
    gStyle->SetLabelSize(0.08,"X");
    gStyle->SetLabelSize(0.08,"Y");
  }
//   Int_t dim = Int_t(round(sqrt(double(draw_count))));
  pair <UInt_t,UInt_t> dim = fConfig->GetPageDim(current_page);

#ifdef DEBUG
  cout << "Dimensions: " << dim.first << "X" 
       << dim.second << endl;
#endif  

  // Create a nice clean canvas.
  fCanvas->Clear();
  fCanvas->Divide(dim.first,dim.second);

  vector <TString> drawcommand(5);
  // Draw the histograms.
  for(UInt_t i=0; i<draw_count; i++) {    
    drawcommand = fConfig->GetDrawCommand(current_page,i);
    fCanvas->cd(i+1);
    if (drawcommand[0] == "macro") {
      MacroDraw(drawcommand);
    } else if (IsHistogram(drawcommand[0])) {
      HistDraw(drawcommand);
    } else {
      TreeDraw(drawcommand);
    }
  }
      
  fCanvas->cd();
  fCanvas->Update();

  if(!fPrintOnly) {
    CheckPageButtons();
  }

}

void OnlineGUI::DrawNext()
{
  // Handler for the "Next" button.

  fRadioPage[current_page]->SetState(kButtonUp);
  // The following line triggers DoRadio()
  fRadioPage[current_page+1]->SetState(kButtonDown);
  
}

void OnlineGUI::DrawPrev()
{
  // Handler for the "Prev" button.

  fRadioPage[current_page]->SetState(kButtonUp);
  // The following line triggers DoRadio()
  fRadioPage[current_page-1]->SetState(kButtonDown);
  
}

void OnlineGUI::DoRadio()
{
  // Handle the radio buttons
  //  Find out which button has been pressed..
  //   turn off the previous button...
  //   then draw the appropriate page.
  // This routine also handles the Draw from the Prev/Next buttons
  //   - makes a call to DoDraw()

  UInt_t pagecount = fConfig->GetPageCount();
  TGButton *btn = (TGButton *) gTQSender;
  UInt_t id = btn->WidgetId();
  
  if (id <= pagecount) {  
    fRadioPage[current_page]->SetState(kButtonUp);
  }

  current_page = id;
  if(!fConfig->IsMonitor()) DoDraw();

}

void OnlineGUI::CheckPageButtons() 
{
  // Checks the current page to see if it's the first or last page.
  //  If so... turn off the appropriate button.
  //  If not.. turn on both buttons.

  if(current_page==0) {
    fPrev->SetState(kButtonDisabled);
    if(fConfig->GetPageCount()!=1)
      fNext->SetState(kButtonUp);
  } else if(current_page==fConfig->GetPageCount()-1) {
    fNext->SetState(kButtonDisabled);
    if(fConfig->GetPageCount()!=1)
      fPrev->SetState(kButtonUp);
  } else {
    fPrev->SetState(kButtonUp);
    fNext->SetState(kButtonUp);
  }
}

Bool_t OnlineGUI::IsHistogram(TString objectname) 
{
  // Utility to determine if the objectname provided is a histogram

  for(UInt_t i=0; i<fileObjects.size(); i++) {
    if (fileObjects[i].first.Contains(objectname)) {
#ifdef DEBUG2
      cout << fileObjects[i].first << "      "
	   << fileObjects[i].second << endl;
#endif
      if(fileObjects[i].second.Contains("TH"))
	return kTRUE;
    }
  }

  return kFALSE;

}

void OnlineGUI::GetFileObjects() 
{
  // Utility to find all of the objects within a File (TTree, TH1F, etc).
  //  The pair stored in the vector is <ObjName, ObjType>
  //  If there's no good keys.. do nothing.
#ifdef DEBUG
  cout << "Keys = " << fRootFile->ReadKeys() << endl;
#endif
  if(fRootFile->ReadKeys()==0) {
    fUpdate = kFALSE;
//     delete fRootFile;
//     fRootFile = 0;
//     CheckRootFile();
    return;
  }
  fileObjects.clear();
  TIter next(fRootFile->GetListOfKeys());
  TKey *key = new TKey();

  // Do the search
  while((key=(TKey*)next())!=0) {
#ifdef DEBUG
    cout << "Key = " << key << endl;    
#endif
    TString objname = key->GetName();
    TString objtype = key->GetClassName();
#ifdef DEBUG
    cout << objname << " " << objtype << endl;
#endif
    fileObjects.push_back(make_pair(objname,objtype));
  }
  fUpdate = kTRUE;
  delete key;
}

void OnlineGUI::GetTreeVars() 
{
  // Utility to find all of the variables (leaf's/branches) within a
  // Specified TTree and put them within the treeVars vector.
  treeVars.clear();
  TObjArray *branchList;
  vector <TString> currentTree;

  for(UInt_t i=0; i<fRootTree.size(); i++) {
    currentTree.clear();
    branchList = fRootTree[i]->GetListOfBranches();
    TIter next(branchList);
    TBranch *brc;

    while((brc=(TBranch*)next())!=0) {
      TString found = brc->GetName();
      // Not sure if the line below is so smart...
      currentTree.push_back(found);
    }
    treeVars.push_back(currentTree);
  }
#ifdef DEBUG2
  for(UInt_t iTree=0; iTree<treeVars.size(); iTree++) {
  cout << "In Tree " << iTree << ": " << endl;
    for(UInt_t i=0; i<treeVars[iTree].size(); i++) {
      cout << treeVars[iTree][i] << endl;
    }
  }
#endif
}


void OnlineGUI::GetRootTree() {
  // Utility to search a ROOT File for ROOT Trees
  // Fills the fRootTree vector
  fRootTree.clear();

  list <TString> found;
  for(UInt_t i=0; i<fileObjects.size(); i++) {
#ifdef DEBUG2
    cout << "Object = " << fileObjects[i].second <<
      "     Name = " << fileObjects[i].first << endl;
#endif
    if(fileObjects[i].second.Contains("TTree"))
       found.push_back(fileObjects[i].first);
  }

  // Remove duplicates, then insert into fRootTree
  found.unique();
  UInt_t nTrees = found.size();

  for(UInt_t i=0; i<nTrees; i++) {
    fRootTree.push_back((TTree*)fRootFile->Get(found.front()));
    found.pop_front();
  }  
  // Initialize the fTreeEntries vector
  fTreeEntries.clear();
  for(UInt_t i=0;i<fRootTree.size();i++) {
    fTreeEntries.push_back(0);
  }
  
}

UInt_t OnlineGUI::GetTreeIndex(TString var) {
  // Utility to find out which Tree (in fRootTree) has the specified
  // variable "var".  If the variable is a collection of Tree
  // variables (e.g. bcm1:lumi1), will only check the first
  // (e.g. bcm1).  
  // Returns the correct index.  if not found returns an index 1
  // larger than fRootTree.size()

  //  This is for 2d draws... look for the first only
  if(var.Contains(":")) {
    TString first_var = fConfig->SplitString(var,":")[0];
    var = first_var;
  }

  //  This is for variables with multiple dimensions.
  if(var.Contains("[")) {
    TString first_var = fConfig->SplitString(var,"[")[0];
    var = first_var;
  }

#ifdef OLD_GETTREEINDEX
  TObjArray *branchList;

  for(UInt_t i=0; i<fRootTree.size(); i++) {
    branchList = fRootTree[i]->GetListOfBranches();
    TIter next(branchList);
    TBranch *brc;

    while((brc=(TBranch*)next())!=0) {
      TString found = brc->GetName();
      if (found == var) {
	return i;
      }
    }
  }
#else
  for(UInt_t iTree=0; iTree<treeVars.size(); iTree++) {
    for(UInt_t ivar=0; ivar<treeVars[iTree].size(); ivar++) {
      if(var == treeVars[iTree][ivar]) return iTree;
    }
  }

#endif
  return fRootTree.size()+1;
}

UInt_t OnlineGUI::GetTreeIndexFromName(TString name) {
  // Called by TreeDraw().  Tries to find the Tree index provided the
  //  name.  If it doesn't match up, return a number that's one larger
  //  than the number of found trees.
  for(UInt_t iTree=0; iTree<fRootTree.size(); iTree++) {
    TString treename = fRootTree[iTree]->GetName();
    if(name == treename) {
      return iTree;
    }
  }

  return fRootTree.size()+1;
}

void OnlineGUI::MacroDraw(vector <TString> command) {
  // Called by DoDraw(), this will make a call to the defined macro, and
  //  plot it in it's own pad.  One plot per macro, please.

  if(command[1].IsNull()) {
    cout << "macro command doesn't contain a macro to execute" << endl;
    return;
  }

  gROOT->Macro(command[1]);
  

}

void OnlineGUI::DoDrawClear() {
  // Utility to grab the number of entries in each tree.  This info is
  // then used, if watching a file, to "clear" the TreeDraw
  // histograms, and begin looking at new data.
  for(UInt_t i=0; i<fTreeEntries.size(); i++) {
    fTreeEntries[i] = (Int_t) fRootTree[i]->GetEntries();
  }
  

}

void OnlineGUI::TimerUpdate() {
  // Called periodically by the timer, if "watchfile" is indicated
  // in the config.  Reloads the ROOT file, and updates the current page.
#ifdef DEBUG
  cout << "Update Now" << endl;
#endif

#ifdef OLDTIMERUPDATE
  fRootFile = new TFile(fConfig->GetRootFile(),"READ");
  if(fRootFile->IsZombie()) {
    cout << "New run not yet available.  Waiting..." << endl;
    fRootFile->Close();
    delete fRootFile;
    fRootFile = 0;
    timer->Reset();
    timer->Disconnect();
    timer->Connect(timer,"Timeout()","OnlineGUI",this,"CheckRootFile()");
    return;
  }

  // Update the runnumber
  ObtainRunNumber();
  if(runNumber != 0) {
    TString rnBuff = "Run #";
    rnBuff += runNumber;
    fRunNumber->SetText(rnBuff.Data());
    hframe->Layout();
  }

  // Open the Root Trees.  Give a warning if it's not there..
  GetFileObjects();
  if (fUpdate) { // Only do this stuff if their are valid keys
    GetRootTree();
    GetTreeVars();
    for(UInt_t i=0; i<fRootTree.size(); i++) {
      if(fRootTree[i]==0) {
	fRootTree.erase(fRootTree.begin() + i);
      }
    }
    DoDraw();
  }
  fRootFile->Close();
  fRootFile->Delete();
  delete fRootFile;
  fRootFile = 0;
#else

  if(fRootFile->IsZombie() || (fRootFile->GetSize() == -1)
     || (fRootFile->ReadKeys()==0)) {
    cout << "New run not yet available.  Waiting..." << endl;
    fRootFile->Close();
    delete fRootFile;
    fRootFile = 0;
    timer->Reset();
    timer->Disconnect();
    timer->Connect(timer,"Timeout()","OnlineGUI",this,"CheckRootFile()");
    return;
  }
  for(UInt_t i=0; i<fRootTree.size(); i++) {
    fRootTree[i]->Refresh();
  }
  DoDraw();
  timer->Reset();

#endif

}

void OnlineGUI::BadDraw(TString errMessage) {
  // Routine to display (in Pad) why a particular draw method has
  // failed.
  TLatex l;
  l.SetTextAlign(23);
  l.SetTextSize(0.1);
  l.DrawLatex(0.5,0.5,errMessage);
  

}


void OnlineGUI::CheckRootFile() {
  // Check the path to the rootfile (should follow symbolic links)
  // ... If found:
  //   Reopen new root file, 
  //   Reconnect the timer to TimerUpdate()

  if(gSystem->AccessPathName(fConfig->GetRootFile())==0) {
    cout << "Found the new run" << endl;
#ifndef OLDTIMERUPDATE
    if(OpenRootFile()==0) {
#endif
      timer->Reset();
      timer->Disconnect();
      timer->Connect(timer,"Timeout()","OnlineGUI",this,"TimerUpdate()");
#ifndef OLDTIMERUPDATE
    }
#endif
  } else {
    TString rnBuff = "Waiting for run";
    fRunNumber->SetText(rnBuff.Data());
    hframe->Layout();
  }

}

Int_t OnlineGUI::OpenRootFile() {


  fRootFile = new TFile(fConfig->GetRootFile(),"READ");
  if(fRootFile->IsZombie() || (fRootFile->GetSize() == -1)
     || (fRootFile->ReadKeys()==0)) {
    cout << "New run not yet available.  Waiting..." << endl;
    fRootFile->Close();
    delete fRootFile;
    fRootFile = 0;
    timer->Reset();
    timer->Disconnect();
    timer->Connect(timer,"Timeout()","OnlineGUI",this,"CheckRootFile()");
    return -1;
  }

  // Update the runnumber
  ObtainRunNumber();
  if(runNumber != 0) {
    TString rnBuff = "Run #";
    rnBuff += runNumber;
    fRunNumber->SetText(rnBuff.Data());
    hframe->Layout();
  }

  // Open the Root Trees.  Give a warning if it's not there..
  GetFileObjects();
  if (fUpdate) { // Only do this stuff if their are valid keys
    GetRootTree();
    GetTreeVars();
    for(UInt_t i=0; i<fRootTree.size(); i++) {
      if(fRootTree[i]==0) {
	fRootTree.erase(fRootTree.begin() + i);
      }
    }
    DoDraw();
  } else {
    return -1;
  }
  return 0;

}

void OnlineGUI::HistDraw(vector <TString> command) {
  // Called by DoDraw(), this will plot a histogram.

  

  // Determine dimensionality of histogram
  for(UInt_t i=0; i<fileObjects.size(); i++) {
    if (fileObjects[i].first.Contains(command[0])) {
      if(fileObjects[i].second.Contains("TH1")) {
	mytemp1d = (TH1D*)gDirectory->Get(command[0]);
	mytemp1d->Draw();
	break;
      }
      if(fileObjects[i].second.Contains("TH2")) {
	mytemp2d = (TH2D*)gDirectory->Get(command[0]);
	mytemp2d->Draw();
	break;
      }
      if(fileObjects[i].second.Contains("TH3")) {
	mytemp3d = (TH3D*)gDirectory->Get(command[0]);
	mytemp3d->Draw();
	break;
      }
    }
  }


}

void OnlineGUI::TreeDraw(vector <TString> command) {
  // Called by DoDraw(), this will plot a Tree Variable

  TString var = command[0];

  // Combine the cuts (definecuts and specific cuts)
  TCut cut = "";
  TString tempCut;
  if(command.size()>1) {
    tempCut = command[1];
    vector <TString> cutIdents = fConfig->GetCutIdent();
    for(UInt_t i=0; i<cutIdents.size(); i++) {
      if(tempCut.Contains(cutIdents[i])) {
	TString cut_found = (TString)fConfig->GetDefinedCut(cutIdents[i]);
	tempCut.ReplaceAll(cutIdents[i],cut_found);
      }
    }
    cut = (TCut)tempCut;
  }

  // Determine which Tree the variable comes from, then draw it.
  UInt_t iTree;
  if(command[4].IsNull()) {
    iTree = GetTreeIndex(var);
  } else {
    iTree = GetTreeIndexFromName(command[4]);
  }
  TString drawopt = command[2];
  Int_t errcode=0;
  if(drawopt.IsNull() && var.Contains(":")) drawopt = "box";
  if (iTree <= fRootTree.size() ) {
    errcode = fRootTree[iTree]->Draw(var,cut,drawopt,
				     1000000000,fTreeEntries[iTree]);
    TObject *hobj = (TObject*)gROOT->FindObject("htemp");
    if(errcode==-1) {
      BadDraw(var+" not found");
    } else if (errcode!=0) {
      if(!command[3].IsNull()) {
	TH1* thathist = (TH1*)hobj;
	thathist->SetTitle(command[3]);
      }
    } else {
      BadDraw("No Entries to Draw");
    }
  } else {
    BadDraw(var+" not found");
    if (fConfig->IsMonitor()){
      // Maybe we missed it... look again.  I dont like the code
      // below... maybe I can come up with something better
      GetFileObjects();
      GetRootTree();
      GetTreeVars();
    }
  }
}

void OnlineGUI::ObtainRunNumber()
{
  // Utility to obtain the runnumber through a helper macro
  //  "GetRunNumber.C"
  
  runNumber = GetRunNumber();
#ifdef DEBUG
  cout << "Runnumber from file: " << runNumber << endl;
#endif
}

void OnlineGUI::PrintToFile()
{
  // Routine to print the current page to a File.
  //  A file dialog pop's up to request the file name.
  fCanvas = fEcanvas->GetCanvas();
  gStyle->SetPaperSize(20,24);
  static TString dir("printouts");
  TGFileInfo fi;
  fi.fFileTypes = filetypes;
  fi.fIniDir    = StrDup(dir.Data());

  new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
  if(fi.fFilename!=NULL) fCanvas->Print(fi.fFilename);
}

void OnlineGUI::PrintPages() {
  // Routine to go through each defined page, and print the output to 
  // a postscript file. (good for making sample histograms).
  
  // Open the RootFile
    //  unless we're watching a file.
  fRootFile = new TFile(fConfig->GetRootFile(),"READ");
  if(!fRootFile->IsOpen()) {
    cout << "ERROR:  rootfile: " << fConfig->GetRootFile()
	 << " does not exist"
	 << endl;
    return;
  } else {
    fFileAlive = kTRUE;
    ObtainRunNumber();
    // Open the Root Trees.  Give a warning if it's not there..
    GetFileObjects();
    GetRootTree();
    GetTreeVars();
    for(UInt_t i=0; i<fRootTree.size(); i++) {
      if(fRootTree[i]==0) {
	fRootTree.erase(fRootTree.begin() + i);
      }
    }
    
  }
  
  // I'm not sure exactly how this works.  But it does.
  fCanvas = new TCanvas("fCanvas","trythis",850,1050);
//   TCanvas *maincanvas = new TCanvas("maincanvas","whatever",850,1100);
//   maincanvas->SetCanvas(fCanvas);
  TLatex *lt = new TLatex();

  TString filename = "sampleplots";
  if(runNumber!=0) {
    filename += "_";
    filename += runNumber;
  }
  filename += ".ps";

  TString pagehead = "Sample Plots";
  if(runNumber!=0) {
    pagehead += "(Run #";
    pagehead += runNumber;
    pagehead += ")";
  }
  pagehead += ": ";

  fCanvas->Print(filename+"[");
  for(UInt_t i=0; i<fConfig->GetPageCount(); i++) {
    current_page=i;
    DoDraw();
    TString pagename = pagehead + fConfig->GetPageTitle(current_page);
    lt->SetTextSize(0.025);
    lt->DrawLatex(0,1.00,pagename);
    fCanvas->Print(filename);
  }
  fCanvas->Print(filename+"]");
  
}

void OnlineGUI::MyCloseWindow()
{
  fMain->SendCloseMessage();
  cout << "OnlineGUI Closed." << endl;
  if(timer!=NULL) {
    timer->Stop();
    delete timer;
  }
  delete fPrint;
  delete fExit;
  delete fRunNumber;
  delete fPrev;
  delete fNext;
  delete wile;
  for(UInt_t i=0; i<fConfig->GetPageCount(); i++) 
    delete fRadioPage[i];
  delete hframe;
  delete fEcanvas;
  delete fBottomFrame;
  delete vframe;
  delete fTopframe;
  delete fMain;
  if(fRootFile!=NULL) delete fRootFile;
  delete fConfig;
}

void OnlineGUI::CloseGUI() 
{
  // Routine to take care of the Exit GUI button
  fMain->SendCloseMessage();
}

OnlineGUI::~OnlineGUI()
{
  //  fMain->SendCloseMessage();
  if(timer!=NULL) {
    timer->Stop();
    delete timer;
  }
  delete fExit;
  delete fRunNumber;
  delete fPrev;
  delete fNext;
  delete wile;
  for(UInt_t i=0; i<fConfig->GetPageCount(); i++) 
    delete fRadioPage[i];
  delete hframe;
  delete fEcanvas;
  delete vframe;
  delete fBottomFrame;
  delete fTopframe;
  delete fMain;
  if(fRootFile!=NULL) delete fRootFile;
  delete fConfig;
}

void online(TString type="standard",UInt_t run=0,Bool_t printonly=kFALSE) 
{
  // "main" routine.  Run this at the ROOT commandline.

  OnlineConfig *fconfig = new OnlineConfig(type);
    //    OnlineConfig *fconfig = new OnlineConfig("halla");

  if(!fconfig->ParseConfig()) {
    return;
  }

  if(run!=0) fconfig->OverrideRootFile(run);

  new OnlineGUI(*fconfig,printonly);

}

