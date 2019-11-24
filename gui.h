#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/cshelp.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "parser.h"

typedef struct 
{
	name devId;
	name pinId;
	int number;
	string nme;
	int startTime;
} mons;

typedef struct
{
	name devId;
	name pinId;
} monDev;

// Specify the IDs for the various wxWidgets used

enum { 
	MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  	MY_TEXTCTRL_ID,
  	MY_RUN_BUTTON_ID,
  	MY_FILE_RUN_ID,
  	MY_FILE_CONTINUE_ID,
  	MY_FILE_SAVE_ID,
  	TOOLBAR_ID,
  	RUN_TOOLBAR_ID,
  	CONTINUE_TOOLBAR_ID,
  	START_TOOLBAR_ID,
  	END_TOOLBAR_ID,
  	MY_CONTINUE_BUTTON_ID,
  	VERT_SLIDER_ID,
  	HORZ_SLIDER_ID,
  	SWITCH_LIST_ID,
  	SWITCH_BUTTON_ID,
  	SWITCH_LISTBOX_ID,
  	MONITOR_LISTBOX_ID,
  	SHOW_GRID_ID,
  	SHOW_SETTINGS_ID,
  	SHOW_DIALOG_ID,
  	SAVE_TOOLBAR_ID,
  	START_MENU_ID,
  	END_MENU_ID,
  	SAVE_MENU_ID,
  	ZOOM_IN_MENU_ID,
  	ZOOM_OUT_MENU_ID,
  	HELP_MENU_ID,
}; // widget identifiers

class MyGLCanvas;

class MyFrame: public wxFrame
{
 	public:
  	MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL, parser * parser_mod = NULL, long style = wxDEFAULT_FRAME_STYLE); // constructor
  	void print_action(string message);		// Function to print to the bottom dialog box
  	//void mirror_char(unsigned char *pixels, int width, int height);
  	void zoom(int n);	// Updates slider values when zooming is caried out on the canvas. Zooming whilst on the canvas and zooming whilst on the frame are treated as two separate events
  	
 	private:
  	MyGLCanvas *canvas;                     // OpenGL drawing area widget to draw traces
  	names *nmz;                             // pointer to names class
  	devices *dmz;                           // pointer to devices class
  	monitor *mmz;                           // pointer to monitor class
  	parser *prs;
	
	// Initialise windows used to set the overall layout of the program
	
	wxWindow *left_canvas_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, _("Canvas"));
	wxWindow *dialog_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, _("Dialog"));
	wxScrolledWindow* right_button_window = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(150,-1), wxVSCROLL, _("Settings"));
	//wxWindow *right_button_window = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN, "Settings");
	
	// Initialise menu items
		
	wxMenu *fileMenu = new wxMenu;
	wxMenu *viewMenu = new wxMenu;
	wxMenu *windowMenu = new wxMenu;	
	
	// Initilise sizers used for layout
		
  	wxBoxSizer *frame_sizer = new wxBoxSizer(wxHORIZONTAL);					// Overall sizer
  	wxBoxSizer *left_canvas_sizer = new wxBoxSizer(wxVERTICAL);				// Sizer for the left side of the program including the toolbar, the canvas and the dialog box
  	wxBoxSizer *canvas_sizer = new wxBoxSizer(wxVERTICAL);					// Sizer for the canvas
  	wxBoxSizer *toolbar_sizer = new wxBoxSizer(wxHORIZONTAL);				// Sizer for the toolbar
  	wxBoxSizer *right_button_sizer = new wxBoxSizer(wxVERTICAL);			// Sizer for the right side of the program including the widgets
  	wxBoxSizer *dialog_sizer = new wxBoxSizer (wxVERTICAL);					// Sizer for the bottom dialog box
  	
  	wxSpinCtrl *spin = new wxSpinCtrl(right_button_window, MY_SPINCNTRL_ID, wxString("10"));	// Spin control used for the number of cycles that the simulation is carried out for   	
  	
  	wxSlider *vert_zoom_slider;		// Slider used for zooming vertically				
  	wxSlider *horz_zoom_slider;		// Slider used for zooming horizontally
  	
  	wxStaticText *vert_zoom_value = new wxStaticText(right_button_window, wxID_ANY, "x100%");		// Used to display the current value of the vertical zoom
  	wxStaticText *horz_zoom_value = new wxStaticText(right_button_window, wxID_ANY, "x100%");		// Used to display the current value of the horizontal zoom
  	
  	int action_num = 2;
  	
  	wxString *switch_list;				// Used to initialise a wxString array used for storing the names of the switches in the circuit
  	wxCheckListBox *toggle_list;		// Initialise the toggleable list of switches
  	wxString *monitor_list;				// Used to initialise a wxString array used for storing the names of the outputs which can be monitored
  	wxCheckListBox *monitor_list1;		// Initiliase the toggleable list of outputs that can be monitored
  	wxString *action_list;				// Used to initialise a wxString which contains a history of the actions taken by the user
	wxListBox *action_list1;			// Initialise a listbox containing a history of the actions taken by the user. This is easy to append to
	
  	wxKeyboardState *kState;  			
  	wxDialog *helpD;					// Dialog for the help menu
  	
  	bool show_grid = true;				// Flag for whether the canvas grid is shown
  	bool show_settings = true;			// Flag for whether the right widgets window is shown
  	bool show_dialog = true;			// Flag for whether the bottom dialog window containing the history of the user's actions is shown
  	bool run_flag = false;				// Flag for whether the simulation has already been run
  	
  	int cyclescompleted;                    // How many simulation cycles have been completed
  	
  	void OnExit(wxCommandEvent& event);     			// Event handler for exit menu item
  	void ShowGrid(wxCommandEvent &event);				// Event handler to show/hide the canvas grid
  	void ShowSettings(wxCommandEvent &event);			// Event handler to show/hide the right widgets window 
  	void ShowDialog(wxCommandEvent &event);				// Event handler to show the bottom dialog window
  	void OnSpin(wxSpinEvent& event);        			// Event handler for spin control
  	void SwitchList(wxCommandEvent &event);				// Event handler for toggling an item in the switch list
  	void MonitorList(wxCommandEvent &event);			// Event handler for toggling an item in the monitor list
  	void OnVertZoomRelease(wxCommandEvent &event);		// Event handler for zooming vertically using the slider
  	void OnHorzZoomRelease(wxCommandEvent &event);		// Event handler for zooming horizontally using the slider
  	void OnRunButton(wxCommandEvent& event);   			// Event handler for the push button
  	void runnetwork(int ncycles);           			// Function to run the logic network
  	void OnContinueButton(wxCommandEvent &event);		// Event handler for pressing the continue button
  	void SaveCanvas(wxCommandEvent &event);				// Event handler for using the saving toolbar icon
  	void OnStart(wxCommandEvent &event);				// Event handler for panning to the start of the simulation
  	void OnEnd(wxCommandEvent &event);					// Event handler for panning to the end of the simulation
  	void OnZoomIn(wxCommandEvent &event);				// Event handler for zooming in (vertically and horizontally by the same amount) using shorcuts
  	void OnZoomOut(wxCommandEvent &event);				// Event handler for zooming out (vertically and horizontally by the same amount) using shorcuts
  	void OnMouse(wxMouseEvent &event);					// Event handler for zooming using the mouse wheel
  	void ShowHelp(wxCommandEvent &event);				// Event handler for opening the help menu
  	void closedHelp(wxCloseEvent &event);				// Event handler for closing the help menu
  	
  	vector<dev> devList;								// Vector containing a list of devices which can be easily appended to within a for loop
  	vector<string> switchNames;							// Vector of strings containing the names of the switches in the circuit which can be easily appended to
  	vector<monDev> MList;
  	
  	vector<dev> rcList;
  	
  DECLARE_EVENT_TABLE()
};

    
class MyGLCanvas: public wxGLCanvas
{
	
	public:
	//comstructor
	MyGLCanvas(wxWindow *parent,MyFrame *f ,wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
		 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
		 const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); 
		 
		 
	void Render(); // function to draw canvas contents

	void ShowGrid(bool show);//functions used by frame to toggle grid on and off
	void ZoomVert(int zoom);//changes vertical zoom
	void ZoomHor(int zoom);//changes horizontal zoom
	void save_canvas();//saves the canvas
	void mirror_char(unsigned char *pixels, int width, int height);//mirrors pixels for saving
	void run(int cycles);//runs the simulation for the number of cycles specified
	void cont(int cycles);//continues the simulation for the number of cycles specified
	void montr();//determines which devices are monitored
	void goToStart();//goes to time zero
	void goToEnd();//goes to the current time in the simulation


	private:
	MyFrame *frame;					   //pointer to the parent frame, needed to call 
									   //some functions, such as changing the slider position
									   //when zooming

	wxGLContext *context;              // OpenGL rendering context
	bool init;                         // has the OpenGL context been initialised?
	int pan_x;                         // the current x pan
	int pan_y;                         // the current y pan
	double zoom;                       // the current zoom
	int cyclesdisplayed;               // how many simulation cycles have been displayed
	monitor *mmz;                      // pointer to monitor class, used to extract signal traces
	names *nmz;                        // pointer to names class, used to extract signal names
	void InitGL();                     // function to initialise OpenGL context
	void OnSize(wxSizeEvent& event);   // event handler for when canvas is resized
	void OnPaint(wxPaintEvent& event); // event handler for when canvas is exposed
	void OnMouse(wxMouseEvent& event); // event handler for mouse events inside canvas

	void printGrid();					//displays a grid for the signals 
	void printTime();					//displayes the simulation cycles at the top
	void maxNumber();					//used to find the maximum number of signals to use

	void printSignals();				//prints signals to cout, used for debugging


	// events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void dClick(wxKeyEvent& event);

	//keep track of monitored signals
	int nmonitor = 0;	//number of the,
	int currentTime = 0;//current simulation time
	vector< vector<asignal> > sigs; //vector of the signals(which are themselves vectors)
	vector<mons> monitoring;	//vector of monitoring points
	bool * monitored; //array of booleans to indicate which devices were monitored in the
						//previous run/continue cycle

	//used for testing
	int signals = 100;	//signals to generate
	int SignalLength = 100;//length of each signal
	vector< vector<int> > signls; //vector that stores the signals
	void generateSignals();//generate signals randomly(50% high, 50% low)


	//time to start dispaying the signal, used for testing and debugging
	int start_signal = 0;	//start at zero
	int end_signal = 10;	//default

	bool showGrid;		//boolean to indicate if the grid is to be displayed or not

	//variables used for printng
	int height;	//height of canvas
	int width;	//width of canvas

	//heigh and width of signal in pixels
	float signal_height;
	float signal_width;

	//the start time and end time of the signal
	//these are for use in testing
	int start_signal_time;
	int end_signal_time;
	
	//max number that can be printed vertically
	int max_number_to_print;
	float space_between_signals = 20.0;//vertical space between signals

	int cycles;
	bool isSmall;	//if the zoom is far out and the signals are small this is set to true

	//start signal at time startAt, display up to endAt
	int startAt = 0;
	int endAt = 10; 

	//for moving the start and end position of the signal
	bool mouse_left;	//true if mouse is on the leftmost part of the canvas
	bool mouse_right;	//true if mouse is on the rightmost part of the canvas

	void printRectangle();	//used to print a gray rectangle at the edges of the canvas

	//used to monitor the current state of some buttons of the keyboard(like shift ctrl and alt)
	wxKeyboardState *kState;  


	DECLARE_EVENT_TABLE()
};
    
    
#endif /* gui_h */


