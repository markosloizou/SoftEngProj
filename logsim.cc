#include "logsim.h"
#include "userint.h"
#include "gui.h"
#include <GL/glut.h>

//#define USE_GUI

IMPLEMENT_APP(MyApp)
  
bool MyApp::OnInit()
  // This function is automatically called when the application starts
{

	bool useGui = false;
	
	
  if (argc < 2 || argc > 3) { // check we have one command line argument
    wcout << "Usage:     " << argv[0] << " \"filename\" " << "[-g or --gui]" << endl;
    exit(1);
  }
  
  if(argc == 3)
  {
	  if(argv[1] == "-g" || argv[1] =="--gui" || argv[2] == "-g" || argv[2] =="--gui")
	  {
	 	useGui = true;
	 	cout << "Using GUI" << endl;
	  }
	  else
	  {
	  	 wcout << "Usage:     " << argv[0] << " \"filename\" " << "[-g or --gui]" << endl;
    	exit(1);
	  }
  }
  

  // Construct the six classes required by the innards of the logic simulator
  nmz = new names();
  netz = new network(nmz);
  dmz = new devices(nmz, netz);
  mmz = new monitor(nmz, netz);
  
  smz = new Scanner(nmz, string{wxString(argv[1]).mb_str()} );
  pmz = new parser(netz, dmz, mmz, smz, nmz);

  if (pmz->readin ()) { // check the logic file parsed correctly
	if(useGui)
	{
		// glutInit cannot cope with Unicode command line arguments, so we pass
		// it some fake ASCII ones instead
		char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);
		// Construct the GUI
		MyFrame *frame = new MyFrame(NULL, "Logic simulator", wxDefaultPosition,  wxSize(900, 800), nmz, dmz, mmz,pmz);
		frame->Show(true);
		return(true); // enter the GUI event loop
	}
    else
    {
    // Construct the text-based interface
    userint umz(nmz, dmz, mmz);
    umz.userinterface();
	}
  }
  return(false); // exit the application
}
