// An ET Client that retreives the ET's heartbeat.
// B. Moffit


#include "THaEtClient.h"
#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char *argv[]) 
{
  
  int mymode = 1;   // prefered mode for ET
  THaEtClient *et;
  et = new THaEtClient("adaql1", mymode);  // opens connection to adaqcp computer.

  string HBenv = "HEARTBEATFILE";
  char *cHBenv = getenv(HBenv.c_str());
  TString HBfile;
  if(cHBenv == 0) {
    cHBenv = "/adaqfs/halla/apar/feedback/etHeartbeat.dat";
  }

  ofstream fileout(cHBenv);
  int heartbeat = et->getheartbeat();
  cout << "ET Heartbeat: " << heartbeat << endl;
  fileout << heartbeat << endl << flush;
  fileout.close();
  
  return 1;
}
