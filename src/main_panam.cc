
#include "TaPanam.hh"
#include <signal.h>

extern void InitGui();
VoidFuncPtr_t initfuncs[] = { InitGui, 0 };

extern "C" void signalhandler(int s);

TROOT root("Pan monitirong", "", initfuncs);

int main(int argc, char **argv)
{
  TApplication theApp("App", &argc, argv);

  TaPanam mainWindow(gClient->GetRoot(), 400, 220);

  theApp.Run();

  return 0;
}

void signalhandler(int sig)
{  // To deal with the signal "kill -31 pid"
  cout << "Ending the online analysis"<<endl<<flush;
  //  mainWindow.EndPan();
  exit(1);
}















